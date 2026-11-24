//! State shared by all sessions of one broker.

use std::collections::HashMap;
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::sync::{Arc, Mutex};
use std::time::Duration;

use tokio::sync::watch;
use tokio::task::JoinHandle;
use tracing::{debug, info};

use super::client::{AttachParams, ClientSession, Detached, SessionLimits};
use super::limits::BrokerLimits;
use super::persist::{SessionSnapshot, SessionStore};
use super::protocol::{ProtocolMode, ProtocolVersion, QoS};
use super::session::SessionOut;
use super::subscriptions::{SessionId, SubscriptionStore};
use super::tracker::ConnectionTracker;
use crate::auth::{Authenticator, LoginThrottle, ThrottleConfig};
use crate::hub::{now_unix_ms, InternalBus, Message};

/// How often the sweeper looks for expired sessions.
const SWEEP_INTERVAL: Duration = Duration::from_secs(1);

/// The sessions of all clients, by session id and by client identifier.
#[derive(Default)]
pub(crate) struct Clients {
    by_id: HashMap<SessionId, Arc<ClientSession>>,
    by_client_id: HashMap<String, SessionId>,
}

impl Clients {
    #[cfg(test)]
    pub(crate) fn len(&self) -> usize {
        self.by_id.len()
    }
}

/// What a CONNECT asks of the session layer.
pub(crate) struct AttachRequest<'a> {
    pub(crate) client_id: &'a str,
    pub(crate) username: Option<&'a str>,
    /// Clean Session (3.1.1) / Clean Start (MQTT 5).
    pub(crate) clean: bool,
    pub(crate) version: ProtocolVersion,
    /// Effective Session Expiry Interval in seconds.
    pub(crate) expiry_s: u32,
    pub(crate) receive_maximum: u16,
    pub(crate) maximum_packet_size: Option<u32>,
    pub(crate) conn_id: u64,
    pub(crate) out: &'a Arc<SessionOut>,
}

pub(crate) enum AttachError {
    /// The client identifier belongs to another user.
    IdentifierRejected,
    /// Too many sessions that outlive their connection.
    QuotaExceeded,
}

pub(crate) struct Attached {
    pub(crate) session: Arc<ClientSession>,
    /// An earlier session of the client was resumed.
    pub(crate) session_present: bool,
    /// Ops were submitted to the store: the CONNACK waits for a barrier.
    pub(crate) submitted: bool,
}

pub(crate) struct Core {
    pub(crate) limits: BrokerLimits,
    /// Which MQTT versions clients may connect with.
    pub(crate) protocol: ProtocolMode,
    /// Checks the credentials of every CONNECT; `None` accepts all clients.
    pub(crate) authenticator: Option<Arc<dyn Authenticator>>,
    /// Throttling of failed logins and pending checks.
    pub(crate) throttle: Arc<LoginThrottle>,
    /// Registry of the client sessions. Lock order: `clients`, `store`, then one session's lock.
    pub(crate) clients: Mutex<Clients>,
    pub(crate) store: Mutex<SubscriptionStore>,
    /// Durable copy of the sessions that outlive their connection.
    pub(crate) persist: Option<Arc<dyn SessionStore>>,
    pub(crate) tracker: Arc<ConnectionTracker>,
    pub(crate) internal: InternalBus,
    session_limits: SessionLimits,
    next_session_id: AtomicU64,
    next_conn_id: AtomicU64,
    closed: AtomicBool,
}

impl Core {
    #[cfg(test)]
    pub(crate) fn new(limits: BrokerLimits, protocol: ProtocolMode) -> Arc<Self> {
        Self::with_authenticator(
            limits,
            protocol,
            None,
            ThrottleConfig::default(),
            None,
            Vec::new(),
        )
    }

    /// `persist` mirrors the sessions that outlive their connection; `initial` are the sessions it held
    /// when the broker started. They are offline until their client connects.
    pub(crate) fn with_authenticator(
        limits: BrokerLimits,
        protocol: ProtocolMode,
        authenticator: Option<Arc<dyn Authenticator>>,
        throttle: ThrottleConfig,
        persist: Option<Arc<dyn SessionStore>>,
        initial: Vec<SessionSnapshot>,
    ) -> Arc<Self> {
        let tracker = ConnectionTracker::new(limits.max_connections, limits.max_connections_per_ip);
        let session_limits = SessionLimits::new(&limits);
        let mut clients = Clients::default();
        let mut store = SubscriptionStore::new();
        let mut next_id = 1;
        let now = now_unix_ms();
        for mut snapshot in initial {
            // the limit may have been lowered since the session was stored
            snapshot.expiry_s = snapshot.expiry_s.min(limits.session_expiry_max_s);
            next_id = next_id.max(snapshot.id.saturating_add(1));
            for (filter, qos, no_local) in &snapshot.subscriptions {
                store.add_subscription(snapshot.id, filter, usize::MAX, *qos, *no_local);
            }
            let id = snapshot.id;
            let session = Arc::new(ClientSession::restore(
                snapshot,
                session_limits,
                persist.clone(),
                now,
            ));
            clients.by_client_id.insert(session.client_id.clone(), id);
            clients.by_id.insert(id, session);
        }
        Arc::new(Self {
            limits,
            protocol,
            authenticator,
            throttle: LoginThrottle::new(throttle),
            clients: Mutex::new(clients),
            store: Mutex::new(store),
            persist,
            tracker,
            internal: InternalBus::default(),
            session_limits,
            next_session_id: AtomicU64::new(next_id),
            next_conn_id: AtomicU64::new(1),
            closed: AtomicBool::new(false),
        })
    }

    pub(crate) fn next_session_id(&self) -> SessionId {
        self.next_session_id.fetch_add(1, Ordering::Relaxed)
    }

    /// Identifies a connection in the log and tells a stale detach from a current one.
    pub(crate) fn next_conn_id(&self) -> u64 {
        self.next_conn_id.fetch_add(1, Ordering::Relaxed)
    }

    /// Marks the broker as shut down and ends all internal subscriptions.
    pub(crate) fn close(&self) {
        self.closed.store(true, Ordering::SeqCst);
        self.internal.close();
    }

    pub(crate) fn is_closed(&self) -> bool {
        self.closed.load(Ordering::SeqCst)
    }

    /// Delivers a message to the internal subscribers and to every session with a matching subscription
    /// (once per session, with the lower of `qos` and the highest QoS the session subscribed with; the
    /// publishing session included unless its subscriptions use the MQTT 5 No Local option). `publisher` is
    /// the session the message came from, `None` for the server itself. Never blocks.
    ///
    /// Returns true if the message was queued in a session that is mirrored in the durable store, so the
    /// acknowledgement of the publisher has to wait for a barrier.
    pub(crate) fn dispatch(
        &self,
        message: Message,
        publisher: Option<SessionId>,
        qos: QoS,
    ) -> bool {
        self.internal.offer(&message);
        // the store lock is only held to find the targets, the delivery happens under the registry lock
        let targets = self
            .store
            .lock()
            .unwrap()
            .targets_from(&message.topic, publisher);
        if targets.is_empty() {
            return false;
        }
        let clients = self.clients.lock().unwrap();
        let mut queued_durably = false;
        for (id, granted) in targets {
            if let Some(session) = clients.by_id.get(&id) {
                queued_durably |= session.deliver(&message, qos.min(granted));
            }
        }
        queued_durably
    }

    /// Makes a connection the owner of the session of its client identifier: resumes the session, or
    /// creates a new one if there is none, the client asked for a clean start, or the old one has expired.
    /// A connection that owned the session before is told to close.
    pub(crate) fn attach(&self, request: AttachRequest<'_>) -> Result<Attached, AttachError> {
        let now = now_unix_ms();
        let mut clients = self.clients.lock().unwrap();
        let mut store = self.store.lock().unwrap();
        let mut existing = clients
            .by_client_id
            .get(request.client_id)
            .and_then(|id| clients.by_id.get(id))
            .cloned();
        // An expired session is as good as gone, whoever asks. One that ends with its connection (expiry 0)
        // is never resumed: it is only still here because its connection has not been cleaned up yet.
        let expired = existing.as_ref().is_some_and(|s| s.is_expired(now));
        if let (Some(session), false) = (&existing, expired) {
            // A session belongs to the user that created it. Whether the client asks for a clean start
            // does not matter: it must not be able to destroy the session of somebody else either.
            if self.authenticator.is_some() && session.username.as_deref() != request.username {
                return Err(AttachError::IdentifierRejected);
            }
        }
        let replace = existing
            .as_ref()
            .is_some_and(|s| request.clean || expired || !s.is_persistent());
        if (existing.is_none() || replace) && request.expiry_s > 0 {
            let mut persistent = clients.by_id.values().filter(|s| s.is_persistent()).count();
            if replace && existing.as_ref().is_some_and(|s| s.is_persistent()) {
                // the new session takes the place of the old one
                persistent -= 1;
            }
            if persistent >= self.limits.max_persistent_sessions {
                return Err(AttachError::QuotaExceeded);
            }
        }
        if replace {
            if let Some(old) = existing.take() {
                Self::remove_locked(&mut clients, &mut store, &old);
            }
        }
        let session_present = existing.is_some();
        let session = match existing {
            Some(session) => session,
            None => {
                let id = self.next_session_id();
                let session = Arc::new(ClientSession::new(
                    id,
                    request.client_id.to_owned(),
                    request.username.map(str::to_owned),
                    self.session_limits,
                    self.persist.clone(),
                ));
                clients
                    .by_client_id
                    .insert(request.client_id.to_owned(), id);
                clients.by_id.insert(id, Arc::clone(&session));
                session
            }
        };
        let submitted = session.attach(
            AttachParams {
                conn_id: request.conn_id,
                out: Arc::clone(request.out),
                version: request.version,
                expiry_s: request.expiry_s,
                receive_maximum: request.receive_maximum,
                maximum_packet_size: request.maximum_packet_size,
            },
            &store,
        );
        Ok(Attached {
            session,
            session_present,
            submitted,
        })
    }

    /// The connection `conn_id` of the session has ended.
    pub(crate) fn detach(&self, session: &Arc<ClientSession>, conn_id: u64) {
        let mut clients = self.clients.lock().unwrap();
        let mut store = self.store.lock().unwrap();
        if session.detach(conn_id, now_unix_ms()) == Detached::Destroyed {
            Self::remove_locked(&mut clients, &mut store, session);
        }
    }

    /// Destroys the sessions whose expiry has passed. Returns how many.
    pub(crate) fn sweep_expired(&self, now_ms: i64) -> usize {
        let mut clients = self.clients.lock().unwrap();
        let expired: Vec<Arc<ClientSession>> = clients
            .by_id
            .values()
            .filter(|s| s.is_expired(now_ms))
            .cloned()
            .collect();
        if expired.is_empty() {
            return 0;
        }
        let mut store = self.store.lock().unwrap();
        for session in &expired {
            debug!(session = session.id, client_id = %session.client_id, "session expired");
            Self::remove_locked(&mut clients, &mut store, session);
        }
        expired.len()
    }

    /// Destroys a session and forgets it. The caller holds the registry and the store lock, so nobody can
    /// add subscriptions or messages to the session in between.
    fn remove_locked(
        clients: &mut Clients,
        store: &mut SubscriptionStore,
        session: &Arc<ClientSession>,
    ) {
        session.destroy();
        store.remove_all(session.id);
        clients.by_id.remove(&session.id);
        if clients.by_client_id.get(&session.client_id) == Some(&session.id) {
            clients.by_client_id.remove(&session.client_id);
        }
    }

    /// Starts the task that destroys expired sessions every second until `shutdown` is signalled.
    pub(crate) fn spawn_sweeper(
        self: &Arc<Self>,
        mut shutdown: watch::Receiver<bool>,
    ) -> JoinHandle<()> {
        let core = Arc::clone(self);
        tokio::spawn(async move {
            let mut ticker = tokio::time::interval(SWEEP_INTERVAL);
            ticker.set_missed_tick_behavior(tokio::time::MissedTickBehavior::Delay);
            loop {
                tokio::select! {
                    biased;
                    _ = shutdown.wait_for(|stop| *stop) => break,
                    _ = ticker.tick() => {
                        let n = core.sweep_expired(now_unix_ms());
                        if n > 0 {
                            info!(sessions = n, "expired sessions removed");
                        }
                    }
                }
            }
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::auth::AuthOutcome;
    use crate::broker::persist::testing::RecordingStore;
    use crate::broker::persist::Op;
    use crate::broker::session::Outgoing;
    use crate::hub::Origin;
    use bytes::Bytes;
    use std::future::Future;
    use std::pin::Pin;
    use tokio::sync::mpsc::UnboundedReceiver;

    const V311: ProtocolVersion = ProtocolVersion::V311;
    const Q0: QoS = QoS::AtMostOnce;
    const Q1: QoS = QoS::AtLeastOnce;
    const Q2: QoS = QoS::ExactlyOnce;

    fn core_with(f: impl FnOnce(&mut BrokerLimits)) -> Arc<Core> {
        let mut limits = BrokerLimits::default();
        f(&mut limits);
        Core::new(limits, ProtocolMode::Both)
    }

    struct Conn {
        out: Arc<SessionOut>,
        rx: UnboundedReceiver<Outgoing>,
        conn_id: u64,
        session: Arc<ClientSession>,
        present: bool,
    }

    fn request<'a>(
        core: &Core,
        client_id: &'a str,
        clean: bool,
        expiry_s: u32,
        out: &'a Arc<SessionOut>,
    ) -> AttachRequest<'a> {
        AttachRequest {
            client_id,
            username: None,
            clean,
            version: V311,
            expiry_s,
            receive_maximum: 100,
            maximum_packet_size: None,
            conn_id: core.next_conn_id(),
            out,
        }
    }

    fn connect(
        core: &Core,
        client_id: &str,
        clean: bool,
        expiry_s: u32,
    ) -> Result<Conn, AttachError> {
        let (out, rx) = SessionOut::new(1 << 20, 3);
        let req = request(core, client_id, clean, expiry_s, &out);
        let conn_id = req.conn_id;
        let attached = core.attach(req)?;
        assert!(attached.session.start(conn_id));
        Ok(Conn {
            out,
            rx,
            conn_id,
            session: attached.session,
            present: attached.session_present,
        })
    }

    fn subscribe(core: &Core, c: &Conn, filter: &str, qos: QoS) {
        core.store
            .lock()
            .unwrap()
            .add_subscription(c.session.id, filter, 100, qos, false);
    }

    fn msg(topic: &str) -> Message {
        Message {
            origin: Origin::Client,
            topic: Arc::from(topic),
            payload: Bytes::from_static(b"x"),
            received_at_ms: 1,
        }
    }

    /// (qos bits, packet id) of every PUBLISH waiting for the connection; QoS 0 shows as (0, 0).
    fn publishes(c: &mut Conn) -> Vec<(u8, u16)> {
        let mut v = Vec::new();
        while let Ok(o) = c.rx.try_recv() {
            match o {
                Outgoing::PublishQos { qos, packet_id, .. } => v.push((qos.bits(), packet_id)),
                Outgoing::Publish(_) => v.push((0, 0)),
                _ => {}
            }
        }
        v
    }

    #[test]
    fn delivery_uses_the_lower_of_message_and_subscription_qos() {
        let core = core_with(|_| {});
        let mut c = connect(&core, "c", true, 0).ok().unwrap();
        subscribe(&core, &c, "t", Q1);
        for qos in [Q0, Q1, Q2] {
            core.dispatch(msg("t"), None, qos);
        }
        assert_eq!(publishes(&mut c), vec![(0, 0), (1, 1), (1, 2)]);
    }

    #[test]
    fn several_matching_filters_give_one_delivery_with_the_highest_granted_qos() {
        let core = core_with(|_| {});
        let mut c = connect(&core, "c", true, 0).ok().unwrap();
        subscribe(&core, &c, "a/b", Q0);
        subscribe(&core, &c, "a/+", Q2);
        subscribe(&core, &c, "#", Q1);
        core.dispatch(msg("a/b"), None, Q2);
        assert_eq!(publishes(&mut c), vec![(2, 1)]);
    }

    #[test]
    fn a_clean_start_destroys_the_old_session_and_a_resume_keeps_it() {
        let core = core_with(|_| {});
        let a = connect(&core, "c", false, 60).ok().unwrap();
        assert!(!a.present);
        subscribe(&core, &a, "t", Q1);
        core.detach(&a.session, a.conn_id); // the session stays in the registry
                                            // resume: same session, subscription still there
        let mut b = connect(&core, "c", false, 60).ok().unwrap();
        assert!(b.present);
        assert_eq!(b.session.id, a.session.id);
        core.dispatch(msg("t"), None, Q1);
        assert_eq!(publishes(&mut b), vec![(1, 1)]);
        // clean start: new session, no subscriptions, the old connection is told
        let c = connect(&core, "c", true, 60).ok().unwrap();
        assert!(!c.present);
        assert_ne!(c.session.id, a.session.id);
        assert_eq!(
            b.out.kicked(),
            Some(crate::broker::session::KickReason::TakenOver)
        );
        assert!(a.session.is_closed());
        assert_eq!(core.store.lock().unwrap().subscription_count(), 0);
        assert_eq!(core.clients.lock().unwrap().len(), 1);
    }

    #[test]
    fn a_second_connection_takes_the_session_over_and_the_old_detach_is_harmless() {
        let core = core_with(|_| {});
        let old = connect(&core, "c", false, 60).ok().unwrap();
        subscribe(&core, &old, "t", Q1);
        let mut new = connect(&core, "c", false, 60).ok().unwrap();
        assert!(new.present);
        assert_eq!(
            old.out.kicked(),
            Some(crate::broker::session::KickReason::TakenOver)
        );
        // the old connection's task cleans up after the new one has attached
        core.detach(&old.session, old.conn_id);
        assert!(!new.session.is_closed());
        core.dispatch(msg("t"), None, Q1);
        assert_eq!(publishes(&mut new), vec![(1, 1)]);
        assert_eq!(core.clients.lock().unwrap().len(), 1);
    }

    #[test]
    fn a_session_with_expiry_zero_disappears_with_its_connection() {
        let core = core_with(|_| {});
        let c = connect(&core, "c", true, 0).ok().unwrap();
        subscribe(&core, &c, "t", Q1);
        core.detach(&c.session, c.conn_id);
        assert!(c.session.is_closed());
        assert_eq!(core.clients.lock().unwrap().len(), 0);
        assert_eq!(core.store.lock().unwrap().subscription_count(), 0);
        // a stale detach finds nothing to do
        core.detach(&c.session, c.conn_id);
    }

    #[test]
    fn offline_sessions_queue_and_the_sweeper_removes_expired_ones() {
        let core = core_with(|_| {});
        let c = connect(&core, "c", false, 10).ok().unwrap();
        subscribe(&core, &c, "t", Q1);
        core.detach(&c.session, c.conn_id);
        core.dispatch(msg("t"), None, Q1);
        core.dispatch(msg("t"), None, Q0);
        assert_eq!(c.session.queue_len(), 1);
        let now = now_unix_ms();
        assert_eq!(core.sweep_expired(now + 5_000), 0);
        assert_eq!(core.sweep_expired(now + 11_000), 1);
        assert!(c.session.is_closed());
        assert_eq!(core.clients.lock().unwrap().len(), 0);
        assert_eq!(core.store.lock().unwrap().subscription_count(), 0);
        // connected sessions never expire
        let _d = connect(&core, "d", false, 1).ok().unwrap();
        assert_eq!(core.sweep_expired(now + 1_000_000), 0);
    }

    #[test]
    fn an_expired_session_is_not_resumed_even_before_the_sweeper_ran() {
        let core = core_with(|_| {});
        let c = connect(&core, "c", false, 1).ok().unwrap();
        core.detach(&c.session, c.conn_id);
        std::thread::sleep(Duration::from_millis(1_100));
        let d = connect(&core, "c", false, 1).ok().unwrap();
        assert!(!d.present);
        assert!(c.session.is_closed());
    }

    #[test]
    fn persistent_sessions_are_limited() {
        let core = core_with(|l| l.max_persistent_sessions = 2);
        let a = connect(&core, "a", false, 60).ok().unwrap();
        let _b = connect(&core, "b", false, 60).ok().unwrap();
        // sessions that end with their connection do not count
        let _z = connect(&core, "z", true, 0).ok().unwrap();
        assert!(matches!(
            connect(&core, "c", false, 60),
            Err(AttachError::QuotaExceeded)
        ));
        assert!(matches!(
            connect(&core, "c", true, 60),
            Err(AttachError::QuotaExceeded)
        ));
        // an existing one can be resumed or replaced, and the refusal destroyed nothing
        assert!(connect(&core, "a", false, 60).ok().unwrap().present);
        assert!(!connect(&core, "a", true, 60).ok().unwrap().present);
        assert!(a.session.is_closed());
        // ending one makes room
        core.detach(&_b.session, _b.conn_id);
        assert_eq!(core.sweep_expired(now_unix_ms() + 61_000), 1);
        assert!(connect(&core, "c", false, 60).is_ok());
    }

    struct AcceptAll;

    impl Authenticator for AcceptAll {
        fn verify<'a>(
            &'a self,
            _username: &'a str,
            _password: &'a [u8],
        ) -> Pin<Box<dyn Future<Output = AuthOutcome> + Send + 'a>> {
            Box::pin(async { AuthOutcome::Accepted })
        }
    }

    #[test]
    fn with_authentication_a_session_belongs_to_its_user() {
        let core = Core::with_authenticator(
            BrokerLimits::default(),
            ProtocolMode::Both,
            Some(Arc::new(AcceptAll)),
            ThrottleConfig::default(),
            None,
            Vec::new(),
        );
        let attach = |user: &str, clean: bool| {
            let (out, _rx) = SessionOut::new(1 << 20, 3);
            let mut req = request(&core, "c", clean, 60, &out);
            req.username = Some(user);
            core.attach(req).map(|a| (a.session, a.session_present))
        };
        let (first, present) = attach("alice", false).ok().unwrap();
        assert!(!present);
        // the same user resumes, another user is refused whatever the clean flag says
        assert!(attach("alice", false).ok().unwrap().1);
        assert!(matches!(
            attach("bob", false),
            Err(AttachError::IdentifierRejected)
        ));
        assert!(matches!(
            attach("bob", true),
            Err(AttachError::IdentifierRejected)
        ));
        // and the refused clean start did not destroy anything
        assert!(!first.is_closed());
        // alice can still start clean
        assert!(!attach("alice", true).ok().unwrap().1);
        assert!(first.is_closed());
    }

    #[test]
    fn without_authentication_the_user_name_does_not_matter() {
        let core = core_with(|_| {});
        let (out, _rx) = SessionOut::new(1 << 20, 3);
        let mut req = request(&core, "c", false, 60, &out);
        req.username = Some("alice");
        assert!(core.attach(req).is_ok());
        let (out, _rx) = SessionOut::new(1 << 20, 3);
        let mut req = request(&core, "c", false, 60, &out);
        req.username = Some("bob");
        assert!(core.attach(req).ok().unwrap().session_present);
    }

    #[test]
    fn dispatch_tells_whether_a_message_was_queued_durably() {
        let store = RecordingStore::new();
        let core = Core::with_authenticator(
            BrokerLimits::default(),
            ProtocolMode::Both,
            None,
            ThrottleConfig::default(),
            Some(store.clone()),
            Vec::new(),
        );
        let durable = connect(&core, "d", false, 60).ok().unwrap();
        let plain = connect(&core, "p", true, 0).ok().unwrap();
        subscribe(&core, &durable, "t", Q2);
        subscribe(&core, &plain, "u", Q2);
        store.take_ops();
        // only a session that is mirrored counts, and QoS 0 never does
        assert!(!core.dispatch(msg("u"), None, Q1));
        assert!(!core.dispatch(msg("t"), None, Q0));
        assert!(store.ops().is_empty());
        assert!(core.dispatch(msg("t"), None, Q1));
        assert!(matches!(&store.ops()[..], [Op::Enqueue { id, .. }] if *id == durable.session.id));
        let _ = plain;
    }

    #[test]
    fn hub_publish_enqueues_for_a_durable_session_without_waiting_for_the_store() {
        let store = RecordingStore::new();
        store.hold();
        let core = Core::with_authenticator(
            BrokerLimits::default(),
            ProtocolMode::Both,
            None,
            ThrottleConfig::default(),
            Some(store.clone()),
            Vec::new(),
        );
        let durable = connect(&core, "d", false, 60).ok().unwrap();
        subscribe(&core, &durable, "t", Q2);
        store.take_ops();
        let hub = crate::hub::Hub::new(Arc::clone(&core));
        hub.publish("t", &b"x"[..]).unwrap();
        // queued write-behind: an Enqueue at QoS 1, no barrier, and the (held) store never committed
        assert!(matches!(
            &store.ops()[..],
            [Op::Enqueue { id, entry }] if *id == durable.session.id && entry.qos == Q1
        ));
        assert_eq!(store.held(), 0);
    }

    #[test]
    fn server_messages_are_not_queued_for_an_offline_session_but_stay_once_queued() {
        let store = RecordingStore::new();
        let core = Core::with_authenticator(
            BrokerLimits::default(),
            ProtocolMode::Both,
            None,
            ThrottleConfig::default(),
            Some(store.clone()),
            Vec::new(),
        );
        let mut durable = connect(&core, "d", false, 60).ok().unwrap();
        subscribe(&core, &durable, "t", Q2);
        let hub = crate::hub::Hub::new(Arc::clone(&core));
        hub.publish("t", &b"online"[..]).unwrap();
        assert_eq!(publishes(&mut durable), vec![(1, 1)]);
        // the client goes away with the message unacknowledged: it stays for the resume
        core.detach(&durable.session, durable.conn_id);
        store.take_ops();
        hub.publish("t", &b"offline"[..]).unwrap();
        assert_eq!(durable.session.queue_len(), 1);
        assert_eq!(durable.session.dropped(), 0, "no drop, no warning");
        assert!(store.ops().is_empty(), "no Enqueue for an offline session");
        // a client publish is queued as before
        assert!(core.dispatch(msg("t"), None, Q1));
        assert_eq!(durable.session.queue_len(), 2);
        let mut again = connect(&core, "d", false, 60).ok().unwrap();
        assert_eq!(publishes(&mut again), vec![(1, 1), (1, 2)]);
        // attached but not started (waiting for its CONNACK): queued, and sent at start
        let (out, mut rx) = SessionOut::new(1 << 20, 3);
        let req = request(&core, "d", false, 60, &out);
        let conn_id = req.conn_id;
        let attached = core.attach(req).ok().unwrap();
        let before = attached.session.queue_len();
        hub.publish("t", &b"early"[..]).unwrap();
        assert_eq!(attached.session.queue_len(), before + 1);
        assert!(rx.try_recv().is_err(), "nothing is sent before the start");
        assert!(attached.session.start(conn_id));
        let mut sent = 0;
        while rx.try_recv().is_ok() {
            sent += 1;
        }
        assert_eq!(sent, before + 1);
    }

    #[test]
    fn loaded_sessions_are_offline_with_their_subscriptions_and_ids_continue_above() {
        use crate::broker::persist::{SessionSnapshot, StoredEntry};
        let snapshot = |id, name: &str| SessionSnapshot {
            id,
            client_id: name.to_owned(),
            username: None,
            version: V311,
            expiry_s: 1000,
            disconnected_at_ms: Some(now_unix_ms()),
            subscriptions: vec![("t/#".to_owned(), Q1, false)],
            outbound: vec![StoredEntry {
                seq: 1,
                packet_id: 1,
                qos: Q1,
                pubrec_received: false,
                origin: Origin::Client,
                topic: Arc::from("t/a"),
                payload: Bytes::from_static(b"x"),
                received_at_ms: 1,
            }],
            inbound_qos2: vec![],
        };
        let store = RecordingStore::new();
        let core = Core::with_authenticator(
            BrokerLimits::default(),
            ProtocolMode::Both,
            None,
            ThrottleConfig::default(),
            Some(store.clone()),
            vec![snapshot(7, "seven"), snapshot(3, "three")],
        );
        assert_eq!(core.next_session_id(), 8);
        assert_eq!(core.store.lock().unwrap().subscription_count(), 2);
        // a message for a loaded offline session is queued behind the loaded one
        assert!(core.dispatch(msg("t/b"), None, Q1));
        let mut c = connect(&core, "seven", false, 1000).ok().unwrap();
        assert!(c.present);
        assert_eq!(c.session.id, 7);
        assert_eq!(publishes(&mut c), vec![(1, 1), (1, 2)]);
        // restoring submitted nothing by itself
        assert!(store
            .ops()
            .iter()
            .all(|op| matches!(op, Op::Enqueue { .. } | Op::UpdateSession { .. })));
    }

    #[test]
    fn clean_connects_with_generated_ids_leave_nothing_behind() {
        let core = core_with(|_| {});
        let a = connect(&core, "dt-1", true, 0).ok().unwrap();
        let b = connect(&core, "dt-2", true, 0).ok().unwrap();
        assert_ne!(a.session.id, b.session.id);
        assert_ne!(a.conn_id, b.conn_id);
        assert_eq!(core.clients.lock().unwrap().len(), 2);
        core.detach(&a.session, a.conn_id);
        core.detach(&b.session, b.conn_id);
        assert_eq!(core.clients.lock().unwrap().len(), 0);
    }

    #[test]
    fn a_session_that_ends_with_its_connection_is_never_resumed() {
        let core = core_with(|l| l.max_persistent_sessions = 1);
        let _a = connect(&core, "a", false, 60).ok().unwrap();
        let z = connect(&core, "z", false, 0).ok().unwrap();
        subscribe(&core, &z, "t", Q1);
        // "z" comes back with a persistent session while the old connection is not cleaned up yet: the
        // quota applies and nothing is resumed
        assert!(matches!(
            connect(&core, "z", false, 60),
            Err(AttachError::QuotaExceeded)
        ));
        assert!(!z.session.is_closed(), "a refused CONNECT destroys nothing");
        // with room for it, the old session is destroyed, its connection told, and the new one is empty
        let core = core_with(|_| {});
        let z = connect(&core, "z", false, 0).ok().unwrap();
        subscribe(&core, &z, "t", Q1);
        let new = connect(&core, "z", false, 60).ok().unwrap();
        assert!(!new.present);
        assert!(z.session.is_closed());
        assert_eq!(
            z.out.kicked(),
            Some(crate::broker::session::KickReason::TakenOver)
        );
        assert_ne!(new.session.id, z.session.id);
        assert_eq!(core.store.lock().unwrap().subscription_count(), 0);
        // the old connection's detach is harmless
        core.detach(&z.session, z.conn_id);
        assert!(!new.session.is_closed());
    }

    #[test]
    fn an_expired_session_of_another_user_is_replaced_not_rejected() {
        let core = Core::with_authenticator(
            BrokerLimits::default(),
            ProtocolMode::Both,
            Some(Arc::new(AcceptAll)),
            ThrottleConfig::default(),
            None,
            Vec::new(),
        );
        let (out, _rx) = SessionOut::new(1 << 20, 3);
        let mut req = request(&core, "c", false, 1, &out);
        req.username = Some("alice");
        let conn_id = req.conn_id;
        let alice = core.attach(req).ok().unwrap().session;
        core.detach(&alice, conn_id);
        // not expired yet: bob is refused
        let (out, _rx) = SessionOut::new(1 << 20, 3);
        let mut req = request(&core, "c", false, 1, &out);
        req.username = Some("bob");
        assert!(matches!(
            core.attach(req),
            Err(AttachError::IdentifierRejected)
        ));
        std::thread::sleep(Duration::from_millis(1_100));
        let (out, _rx) = SessionOut::new(1 << 20, 3);
        let mut req = request(&core, "c", false, 1, &out);
        req.username = Some("bob");
        let bob = core.attach(req).ok().unwrap();
        assert!(!bob.session_present);
        assert_eq!(bob.session.username.as_deref(), Some("bob"));
        assert!(alice.is_closed());
    }

    #[test]
    fn restored_sessions_expire_no_later_than_the_current_maximum() {
        use crate::broker::persist::SessionSnapshot;
        let snapshot = SessionSnapshot {
            id: 4,
            client_id: "old".to_owned(),
            username: None,
            version: V311,
            expiry_s: 1_000_000,
            disconnected_at_ms: Some(now_unix_ms()),
            subscriptions: vec![],
            outbound: vec![],
            inbound_qos2: vec![],
        };
        let limits = BrokerLimits {
            session_expiry_max_s: 100,
            ..BrokerLimits::default()
        };
        let core = Core::with_authenticator(
            limits,
            ProtocolMode::Both,
            None,
            ThrottleConfig::default(),
            None,
            vec![snapshot],
        );
        let session = Arc::clone(core.clients.lock().unwrap().by_id.get(&4).unwrap());
        assert_eq!(session.expiry_s(), 100);
        assert_eq!(core.sweep_expired(now_unix_ms() + 101_000), 1);
    }
}
