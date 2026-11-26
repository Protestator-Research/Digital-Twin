//! TLS for the MQTT listener: loads the certificate chain and the private key from PEM files (as Let's Encrypt /
//! certbot write them) and does the server side of the handshake.

use std::fmt;
use std::path::{Path, PathBuf};
use std::sync::Arc;
use std::time::Duration;

use rustls::pki_types::pem::{Error as PemError, PemObject};
use rustls::pki_types::{CertificateDer, PrivateKeyDer};
use rustls::ServerConfig;
use tokio::net::TcpStream;
use tokio_rustls::server::TlsStream;
use tokio_rustls::TlsAcceptor;

/// Name of the option of the certificate chain, as it appears in error messages.
const OPTION_CHAIN: &str = "instance.mqtt.cert_chain";
/// Name of the option of the private key, as it appears in error messages.
const OPTION_KEY: &str = "instance.mqtt.cert_private_key";

/// Why the TLS setup of the broker failed. Messages name the option and the path and never contain key material.
#[derive(Debug, thiserror::Error)]
pub enum TlsError {
    #[error("MQTT TLS needs both {OPTION_CHAIN} and {OPTION_KEY}, but only {set} ({}) is set", path.display())]
    Incomplete { set: &'static str, path: PathBuf },
    #[error("cannot read {option} ({}): {source}", path.display())]
    Read {
        option: &'static str,
        path: PathBuf,
        source: std::io::Error,
    },
    #[error("{option} ({}) contains no certificate in PEM format", path.display())]
    NoCertificate { option: &'static str, path: PathBuf },
    #[error("{option} ({}) contains no private key in PEM format (PKCS#8, PKCS#1 or SEC1)", path.display())]
    NoPrivateKey { option: &'static str, path: PathBuf },
    #[error("{option} ({}) is an encrypted private key, which is not supported: use an unencrypted key (PKCS#8, PKCS#1 or SEC1)", path.display())]
    EncryptedKey { option: &'static str, path: PathBuf },
    #[error("{option} ({}) is not valid PEM: {source}", path.display())]
    Pem {
        option: &'static str,
        path: PathBuf,
        source: PemError,
    },
    #[error("{OPTION_CHAIN} ({}) and {OPTION_KEY} ({}) cannot be used together: {source}", chain.display(), key.display())]
    Config {
        chain: PathBuf,
        key: PathBuf,
        source: Box<rustls::Error>,
    },
}

/// TLS server settings of the broker: what [`BrokerOptions::tls`](super::BrokerOptions::tls) takes.
/// TLS 1.2 and 1.3, no client certificates, the whole chain is sent.
#[derive(Clone)]
pub struct BrokerTls {
    acceptor: TlsAcceptor,
}

impl fmt::Debug for BrokerTls {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("BrokerTls").finish_non_exhaustive()
    }
}

impl BrokerTls {
    /// Loads the PEM files. The key may be PKCS#8, RSA PKCS#1 or SEC1 EC. The files are read once, so a renewed
    /// certificate needs a new broker.
    pub fn load(cert_chain: &Path, private_key: &Path) -> Result<BrokerTls, TlsError> {
        let chain_pem = read(OPTION_CHAIN, cert_chain)?;
        let key_pem = read(OPTION_KEY, private_key)?;
        let certs = CertificateDer::pem_slice_iter(&chain_pem)
            .collect::<Result<Vec<_>, _>>()
            .map_err(|source| TlsError::Pem {
                option: OPTION_CHAIN,
                path: cert_chain.to_path_buf(),
                source,
            })?;
        if certs.is_empty() {
            return Err(TlsError::NoCertificate {
                option: OPTION_CHAIN,
                path: cert_chain.to_path_buf(),
            });
        }
        let key = match PrivateKeyDer::from_pem_slice(&key_pem) {
            Ok(key) => key,
            Err(_) if is_encrypted_key(&key_pem) => {
                return Err(TlsError::EncryptedKey {
                    option: OPTION_KEY,
                    path: private_key.to_path_buf(),
                })
            }
            Err(PemError::NoItemsFound) => {
                return Err(TlsError::NoPrivateKey {
                    option: OPTION_KEY,
                    path: private_key.to_path_buf(),
                })
            }
            Err(source) => {
                return Err(TlsError::Pem {
                    option: OPTION_KEY,
                    path: private_key.to_path_buf(),
                    source,
                })
            }
        };
        // An explicit provider: the process-wide default is only set if exactly one provider is compiled in,
        // and several rustls users live in this binary (reqwest, sqlx).
        let config =
            ServerConfig::builder_with_provider(Arc::new(rustls::crypto::ring::default_provider()))
                .with_protocol_versions(&[&rustls::version::TLS13, &rustls::version::TLS12])
                .map_err(|source| config_error(cert_chain, private_key, source))?
                .with_no_client_auth()
                .with_single_cert(certs, key)
                .map_err(|source| config_error(cert_chain, private_key, source))?;
        Ok(BrokerTls {
            acceptor: TlsAcceptor::from(Arc::new(config)),
        })
    }

    /// Reads the two options: both empty gives `None` (plain TCP), both set loads the files, only one is an
    /// error. Whitespace only counts as empty.
    pub fn from_options(
        cert_chain: &str,
        private_key: &str,
    ) -> Result<Option<BrokerTls>, TlsError> {
        match (cert_chain.trim(), private_key.trim()) {
            ("", "") => Ok(None),
            ("", key) => Err(TlsError::Incomplete {
                set: OPTION_KEY,
                path: PathBuf::from(key),
            }),
            (chain, "") => Err(TlsError::Incomplete {
                set: OPTION_CHAIN,
                path: PathBuf::from(chain),
            }),
            (chain, key) => BrokerTls::load(Path::new(chain), Path::new(key)).map(Some),
        }
    }

    /// The server side of the handshake, at most `limit` long.
    pub(crate) async fn accept(
        &self,
        stream: TcpStream,
        limit: Duration,
    ) -> Result<TlsStream<TcpStream>, HandshakeError> {
        match tokio::time::timeout(limit, self.acceptor.accept(stream)).await {
            Ok(Ok(stream)) => Ok(stream),
            Ok(Err(e)) => Err(HandshakeError::Failed(e)),
            Err(_) => Err(HandshakeError::TimedOut),
        }
    }
}

/// Why a TLS handshake did not complete.
#[derive(Debug, thiserror::Error)]
pub(crate) enum HandshakeError {
    #[error("{0}")]
    Failed(std::io::Error),
    #[error("handshake timeout")]
    TimedOut,
}

/// Whether the file holds a password protected key (PKCS#8 `ENCRYPTED PRIVATE KEY` or the old `Proc-Type: 4,ENCRYPTED`
/// header of a traditional key), which cannot be loaded.
fn is_encrypted_key(pem: &[u8]) -> bool {
    let text = String::from_utf8_lossy(pem);
    text.contains("BEGIN ENCRYPTED PRIVATE KEY") || text.contains("Proc-Type: 4,ENCRYPTED")
}

fn read(option: &'static str, path: &Path) -> Result<Vec<u8>, TlsError> {
    std::fs::read(path).map_err(|source| TlsError::Read {
        option,
        path: path.to_path_buf(),
        source,
    })
}

fn config_error(chain: &Path, key: &Path, source: rustls::Error) -> TlsError {
    TlsError::Config {
        chain: chain.to_path_buf(),
        key: key.to_path_buf(),
        source: Box::new(source),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // Fixtures made with `openssl` (RSA with `genrsa -traditional`, EC with `ecparam -genkey -noout`), self-signed
    // for CN=localhost; test keys that protect nothing.
    const RSA_CERT: &str = include_str!("testdata/rsa_cert.pem");
    const RSA_PKCS1_KEY: &str = include_str!("testdata/rsa_pkcs1_key.pem");
    const EC_CERT: &str = include_str!("testdata/ec_cert.pem");
    const EC_SEC1_KEY: &str = include_str!("testdata/ec_sec1_key.pem");

    /// A temporary directory that is removed at the end of the test.
    struct TempDir(PathBuf);

    impl std::ops::Deref for TempDir {
        type Target = Path;
        fn deref(&self) -> &Path {
            &self.0
        }
    }

    impl Drop for TempDir {
        fn drop(&mut self) {
            let _ = std::fs::remove_dir_all(&self.0);
        }
    }

    fn dir(name: &str) -> TempDir {
        let dir = std::env::temp_dir().join(format!("dt-tls-{name}-{}", std::process::id()));
        std::fs::create_dir_all(&dir).unwrap();
        TempDir(dir)
    }

    fn write(dir: &Path, name: &str, content: &str) -> PathBuf {
        let path = dir.join(name);
        std::fs::write(&path, content).unwrap();
        path
    }

    /// Certificate and PKCS#8 key (what current certbot writes), generated for the test.
    fn pkcs8_pair() -> (String, String) {
        let key = rcgen::KeyPair::generate().unwrap();
        let cert = rcgen::CertificateParams::new(vec!["localhost".into()])
            .unwrap()
            .self_signed(&key)
            .unwrap();
        (cert.pem(), key.serialize_pem())
    }

    #[test]
    fn loads_pkcs8_key() {
        let d = dir("pkcs8");
        let (cert, key) = pkcs8_pair();
        assert!(key.contains("BEGIN PRIVATE KEY"));
        let c = write(&d, "fullchain.pem", &cert);
        let k = write(&d, "privkey.pem", &key);
        BrokerTls::load(&c, &k).unwrap();
    }

    #[test]
    fn loads_rsa_pkcs1_key() {
        let d = dir("pkcs1");
        assert!(RSA_PKCS1_KEY.contains("BEGIN RSA PRIVATE KEY"));
        let c = write(&d, "fullchain.pem", RSA_CERT);
        let k = write(&d, "privkey.pem", RSA_PKCS1_KEY);
        BrokerTls::load(&c, &k).unwrap();
    }

    #[test]
    fn loads_sec1_ec_key() {
        let d = dir("sec1");
        assert!(EC_SEC1_KEY.contains("BEGIN EC PRIVATE KEY"));
        let c = write(&d, "fullchain.pem", EC_CERT);
        let k = write(&d, "privkey.pem", EC_SEC1_KEY);
        BrokerTls::load(&c, &k).unwrap();
    }

    #[test]
    fn loads_chain_with_several_certificates() {
        let d = dir("chain");
        // the leaf first, then an unrelated certificate as "intermediate", as in a fullchain.pem
        let c = write(&d, "fullchain.pem", &format!("{EC_CERT}\n{RSA_CERT}"));
        let k = write(&d, "privkey.pem", EC_SEC1_KEY);
        BrokerTls::load(&c, &k).unwrap();
    }

    #[test]
    fn no_option_is_plain_tcp() {
        assert!(BrokerTls::from_options("", "").unwrap().is_none());
        assert!(BrokerTls::from_options("  ", "").unwrap().is_none());
    }

    #[test]
    fn only_one_option_is_an_error() {
        let e = BrokerTls::from_options("chain.pem", "").unwrap_err();
        assert!(matches!(e, TlsError::Incomplete { .. }));
        let text = e.to_string();
        assert!(text.contains("instance.mqtt.cert_private_key"), "{text}");
        assert!(
            text.contains("only instance.mqtt.cert_chain (chain.pem) is set"),
            "{text}"
        );
        let e = BrokerTls::from_options("", "key.pem").unwrap_err();
        assert!(e
            .to_string()
            .contains("only instance.mqtt.cert_private_key (key.pem) is set"));
    }

    #[test]
    fn missing_file_names_option_and_path() {
        let d = dir("missing");
        let c = write(&d, "fullchain.pem", EC_CERT);
        let k = write(&d, "privkey.pem", EC_SEC1_KEY);
        let missing = d.join("nope.pem");
        let e = BrokerTls::load(&missing, &k).unwrap_err();
        assert!(matches!(e, TlsError::Read { .. }));
        let e = e.to_string();
        assert!(
            e.contains("instance.mqtt.cert_chain") && e.contains("nope.pem"),
            "{e}"
        );
        let e = BrokerTls::load(&c, &missing).unwrap_err().to_string();
        assert!(
            e.contains("instance.mqtt.cert_private_key") && e.contains("nope.pem"),
            "{e}"
        );
        // through the options as well
        let e = BrokerTls::from_options(missing.to_str().unwrap(), k.to_str().unwrap())
            .unwrap_err()
            .to_string();
        assert!(e.contains("nope.pem"), "{e}");
    }

    #[test]
    fn directory_instead_of_file_is_a_read_error() {
        let d = dir("isdir");
        let e = BrokerTls::load(&d, &d).unwrap_err();
        assert!(matches!(e, TlsError::Read { .. }), "{e}");
    }

    #[test]
    fn file_without_certificate_or_key_is_an_error() {
        let d = dir("nopem");
        let c = write(&d, "fullchain.pem", EC_CERT);
        let k = write(&d, "privkey.pem", EC_SEC1_KEY);
        let empty = write(&d, "empty.pem", "not a pem file\n");
        let e = BrokerTls::load(&empty, &k).unwrap_err();
        assert!(matches!(e, TlsError::NoCertificate { .. }));
        let text = e.to_string();
        assert!(
            text.contains("instance.mqtt.cert_chain") && text.contains("empty.pem"),
            "{text}"
        );
        let e = BrokerTls::load(&c, &empty).unwrap_err();
        assert!(matches!(e, TlsError::NoPrivateKey { .. }));
        let text = e.to_string();
        assert!(
            text.contains("instance.mqtt.cert_private_key") && text.contains("empty.pem"),
            "{text}"
        );
        // a certificate where the key should be, and a key where the chain should be
        assert!(matches!(
            BrokerTls::load(&c, &c).unwrap_err(),
            TlsError::NoPrivateKey { .. }
        ));
        assert!(matches!(
            BrokerTls::load(&k, &k).unwrap_err(),
            TlsError::NoCertificate { .. }
        ));
    }

    #[test]
    fn encrypted_key_is_reported_as_such() {
        let d = dir("encrypted");
        let c = write(&d, "fullchain.pem", EC_CERT);
        let pkcs8 = "-----BEGIN ENCRYPTED PRIVATE KEY-----
MIHsMFcGCSqGSIb3DQEFDTBKMCkGCSqGSIb3DQEFDDAcBAhA
-----END ENCRYPTED PRIVATE KEY-----
";
        let k = write(&d, "privkey.pem", pkcs8);
        let e = BrokerTls::load(&c, &k).unwrap_err();
        assert!(matches!(e, TlsError::EncryptedKey { .. }), "{e}");
        let text = e.to_string();
        assert!(
            text.contains("encrypted")
                && text.contains("instance.mqtt.cert_private_key")
                && text.contains("privkey.pem"),
            "{text}"
        );
        // the traditional format with a Proc-Type header
        let old = "-----BEGIN RSA PRIVATE KEY-----
Proc-Type: 4,ENCRYPTED
DEK-Info: AES-128-CBC,0123

AAAA
-----END RSA PRIVATE KEY-----
";
        let k = write(&d, "old.pem", old);
        let e = BrokerTls::load(&c, &k).unwrap_err();
        assert!(matches!(e, TlsError::EncryptedKey { .. }), "{e}");
    }

    #[test]
    fn key_that_does_not_belong_to_the_certificate_is_an_error() {
        let d = dir("mismatch");
        let c = write(&d, "fullchain.pem", EC_CERT);
        let k = write(&d, "privkey.pem", &pkcs8_pair().1);
        let e = BrokerTls::load(&c, &k).unwrap_err();
        assert!(matches!(e, TlsError::Config { .. }), "{e}");
    }

    #[test]
    fn errors_never_contain_key_material() {
        let d = dir("secret");
        let (cert, key) = pkcs8_pair();
        let c = write(&d, "fullchain.pem", &cert);
        let broken = format!("{}\n", key.replace("END", "ENX"));
        let k = write(&d, "privkey.pem", &broken);
        let wrong = write(&d, "wrong.pem", EC_SEC1_KEY);
        let errors = [
            BrokerTls::load(&c, &k).unwrap_err(),
            BrokerTls::load(&c, &wrong).unwrap_err(),
        ];
        let body = key.lines().nth(1).unwrap();
        let ec_body = EC_SEC1_KEY.lines().nth(1).unwrap();
        for e in errors {
            let text = format!("{e} {e:?}");
            assert!(!text.contains(body) && !text.contains(ec_body), "{text}");
        }
    }
}
