//! Build script. Does nothing without the cargo feature `ffi`.
//!
//! With `ffi` it compiles the cxx bridge (`src/twin/ffi.rs`) and the C++ facade (`ffi/dt_facade.cpp`) and links
//! them against an existing CMake/Conan build tree of the C++ libraries (it never builds or changes that tree).
//! The decisions (paths, libraries, rpath, DLLs) are made by `ffi/link_plan.rs`; this file only reads the
//! environment and the two input files, and prints the cargo directives. See README.md, "With the C++ model
//! libraries".
//!
//! Note: the root `.gitignore` ignores `**/build**`; the exception `!DTServerRust/build.rs` keeps this file tracked.

#[cfg(feature = "ffi")]
#[path = "ffi/link_plan.rs"]
mod link_plan;

#[cfg(not(feature = "ffi"))]
fn main() {
    // Without the feature there is nothing to do; tell cargo so the script is not rerun on every source change.
    println!("cargo:rerun-if-changed=build.rs");
}

#[cfg(feature = "ffi")]
fn main() {
    if let Err(message) = ffi::run() {
        // Cargo shows stderr of a failed build script; keep it readable (no Debug dump of an error type).
        eprintln!("\nerror: {message}\n");
        std::process::exit(1);
    }
}

#[cfg(feature = "ffi")]
mod ffi {
    use std::collections::BTreeSet;
    use std::env;
    use std::ffi::OsString;
    use std::fs;
    use std::path::{Path, PathBuf};

    use crate::link_plan::{self, LinkInput, LinkPlan, PROJECT_INCLUDE_DIRS};

    const ENV_VARS: [&str; 7] = [
        "CXX",
        "DT_CPP_BUILD_DIR",
        "DT_CPP_CONFIG",
        "DT_CPP_CONAN_GRAPH",
        "DT_CPP_SOURCE_DIR",
        "DT_CPP_EXTRA_LIB_DIRS",
        "DT_CPP_NO_DLL_COPY",
    ];

    fn var(name: &str) -> Option<OsString> {
        env::var_os(name).filter(|v| !v.is_empty())
    }

    fn cargo_var(name: &str) -> Result<String, String> {
        env::var(name).map_err(|_| {
            format!("cargo did not set {name}; this build script must run under cargo")
        })
    }

    fn read_text(path: &Path, what: &str, hint: &str) -> Result<String, String> {
        fs::read(path)
            .map_err(|e| format!("cannot read {what} '{}': {e}. {hint}", path.display()))
            .and_then(|bytes| {
                String::from_utf8(bytes)
                    .map_err(|_| format!("{what} '{}' is not valid UTF-8", path.display()))
            })
    }

    pub fn run() -> Result<(), String> {
        for v in ENV_VARS {
            println!("cargo:rerun-if-env-changed={v}");
        }
        for f in [
            "build.rs",
            "ffi/dt_facade.h",
            "ffi/dt_facade.cpp",
            "ffi/link_plan.rs",
            "src/twin/ffi.rs",
        ] {
            println!("cargo:rerun-if-changed={f}");
        }

        let manifest_dir = PathBuf::from(cargo_var("CARGO_MANIFEST_DIR")?);
        let target_os = cargo_var("CARGO_CFG_TARGET_OS")?;
        let target_env = env::var("CARGO_CFG_TARGET_ENV").unwrap_or_default();

        let build_dir = var("DT_CPP_BUILD_DIR").map(PathBuf::from).ok_or_else(|| {
            "the feature `ffi` needs the C++ build tree: set DT_CPP_BUILD_DIR to the CMake build directory of the \
             C++ libraries (it must contain CMakeCache.txt and conan-graph.json; see README.md, \"With the C++ \
             model libraries\")"
                .to_string()
        })?;
        let config = var("DT_CPP_CONFIG").map(|v| v.to_string_lossy().into_owned());
        let graph_path = var("DT_CPP_CONAN_GRAPH")
            .map(PathBuf::from)
            .unwrap_or_else(|| {
                // default: in the build directory, else one level up (single-config `cmake_layout` trees)
                let candidates = link_plan::graph_candidates(&build_dir);
                candidates
                    .iter()
                    .find(|c| c.is_file())
                    .unwrap_or(&candidates[0])
                    .clone()
            });
        let source_dir = var("DT_CPP_SOURCE_DIR")
            .map(PathBuf::from)
            .or_else(|| manifest_dir.parent().map(Path::to_path_buf))
            .ok_or("cannot derive the source directory; set DT_CPP_SOURCE_DIR")?;
        let extra_lib_dirs: Vec<PathBuf> = var("DT_CPP_EXTRA_LIB_DIRS")
            .map(|v| {
                env::split_paths(&v)
                    .filter(|p| !p.as_os_str().is_empty())
                    .collect()
            })
            .unwrap_or_default();
        // Empty, "0", "false" and "off" (any case) mean "not set".
        let no_dll_copy = var("DT_CPP_NO_DLL_COPY").is_some_and(|v| {
            !matches!(
                v.to_string_lossy().trim().to_ascii_lowercase().as_str(),
                "" | "0" | "false" | "off"
            )
        });

        let cache_path = build_dir.join("CMakeCache.txt");
        println!("cargo:rerun-if-changed={}", cache_path.display());
        println!("cargo:rerun-if-changed={}", graph_path.display());

        let cache = fs::read(&cache_path)
            .map_err(|e| e.to_string())
            .and_then(|b| String::from_utf8(b).map_err(|_| "not valid UTF-8".to_string()))
            .map_err(|e| {
                let hint = link_plan::cache_dir_hint(&build_dir, &|p| p.is_file());
                link_plan::missing_cache_message(&build_dir, &e, hint.as_deref())
            })?;
        let graph = read_text(
            &graph_path,
            "the Conan graph JSON",
            "create it once with `conan install . -s build_type=<Release|RelWithDebInfo> --format=json \
             --out-file <build dir>/conan-graph.json`, or point DT_CPP_CONAN_GRAPH at it",
        )?;

        let plan = link_plan::plan(&LinkInput {
            target_os: &target_os,
            target_env: &target_env,
            cmake_cache: &cache,
            conan_graph: &graph,
            config: config.as_deref(),
            build_dir: &build_dir,
            source_dir: &source_dir,
            extra_lib_dirs: &extra_lib_dirs,
        })
        .map_err(|e| e.to_string())?;

        emit_rerun(&plan, &source_dir);
        warn_missing(&plan, &target_os);
        compile(&plan, &manifest_dir, &target_env)?;
        emit_link(&plan, &target_os);
        if target_os == "windows" && !no_dll_copy {
            let out_dir = PathBuf::from(cargo_var("OUT_DIR")?);
            let (stage, copies) = stage_dlls(&plan, &out_dir)?;
            // Cargo adds link-search directories to PATH for `cargo run` and `cargo test`.
            println!("cargo:rustc-link-search=native={}", stage.display());
            // A deleted copy next to the artifact reruns the script, which restores it. The copies keep the
            // modification time of their source, so this does not make the script rerun by itself.
            for c in copies {
                println!("cargo:rerun-if-changed={}", c.display());
            }
        }
        Ok(())
    }

    /// Reruns the script when headers or the built libraries change (cargo scans directories recursively).
    fn emit_rerun(plan: &LinkPlan, source_dir: &Path) {
        for d in PROJECT_INCLUDE_DIRS {
            println!("cargo:rerun-if-changed={}", source_dir.join(d).display());
        }
        for d in &plan.sysml_include_dirs {
            println!("cargo:rerun-if-changed={}", d.display());
        }
        if let Ok(entries) = fs::read_dir(&plan.project_lib_dir) {
            let mut files: Vec<PathBuf> = entries
                .filter_map(Result::ok)
                .map(|e| e.path())
                .filter(|p| {
                    p.extension().is_some_and(|x| {
                        ["lib", "dll", "so", "dylib", "a"]
                            .iter()
                            .any(|e| x.eq_ignore_ascii_case(e))
                    })
                })
                .collect();
            files.sort();
            for f in files {
                println!("cargo:rerun-if-changed={}", f.display());
            }
        }
    }

    fn warn_missing(plan: &LinkPlan, target_os: &str) {
        let mut seen = BTreeSet::new();
        // Not the Conan bindirs: static packages declare a bin directory they never create.
        let dirs = plan.include_dirs.iter().chain(&plan.lib_search_dirs);
        for d in dirs {
            if !d.is_dir() && seen.insert(d.clone()) {
                println!(
                    "cargo:warning=directory from the C++ build tree does not exist (stale build or Conan graph?): {}",
                    d.display()
                );
            }
        }
        if target_os == "windows" && !plan.project_lib_dir.is_dir() {
            println!(
                "cargo:warning=project library directory is missing; build the C++ tree in configuration {} first",
                plan.config
            );
        }
    }

    fn compile(plan: &LinkPlan, manifest_dir: &Path, target_env: &str) -> Result<(), String> {
        let mut build = cxx_build::bridge("src/twin/ffi.rs");
        build
            .file("ffi/dt_facade.cpp")
            .include(manifest_dir.join("ffi"));
        for d in &plan.include_dirs {
            build.include(d);
        }
        for (name, value) in &plan.defines {
            build.define(name, value.as_deref());
        }
        if target_env == "msvc" {
            build.std("c++20").flag("/bigobj").flag("/EHsc");
        } else {
            build.std("c++20");
            // The compiler of the C++ tree, unless the user chose one with CXX.
            if let (Some(cxx), None) = (&plan.cxx_compiler, var("CXX")) {
                build.compiler(cxx);
            }
            for f in &plan.cxx_flags {
                build.flag(f);
            }
            if plan.cpp_set_stdlib.as_deref() == Some("c++") {
                build.cpp_set_stdlib("c++");
            }
        }
        build
            .try_compile("dt_facade")
            .map_err(|e| format!("compiling the cxx bridge and ffi/dt_facade.cpp failed: {e}"))
    }

    fn emit_link(plan: &LinkPlan, target_os: &str) {
        for d in &plan.lib_search_dirs {
            println!("cargo:rustc-link-search=native={}", d.display());
        }
        for l in &plan.libs {
            println!("cargo:rustc-link-lib=dylib={l}");
        }
        match target_os {
            "linux" => {
                // DT_RPATH (not RUNPATH) so the rpath also covers dependencies of the libraries.
                println!("cargo:rustc-link-arg=-Wl,--disable-new-dtags");
                for d in &plan.rpath_dirs {
                    println!("cargo:rustc-link-arg=-Wl,-rpath,{}", d.display());
                }
            }
            "macos" => {
                for d in &plan.rpath_dirs {
                    println!("cargo:rustc-link-arg=-Wl,-rpath,{}", d.display());
                }
            }
            _ => {}
        }
    }

    /// Copies the DLLs into `<OUT_DIR>/dll` (returned; emitted as link-search so cargo puts it on PATH for
    /// `cargo run`/`cargo test`) and next to the final artifacts, so `dt-server.exe` starts directly. Earlier
    /// directories of the plan win for equal file names.
    fn stage_dlls(plan: &LinkPlan, out_dir: &Path) -> Result<(PathBuf, Vec<PathBuf>), String> {
        let dirs = link_plan::runtime_dirs(out_dir).ok_or_else(|| {
            format!(
                "cannot find the 'build' directory above OUT_DIR '{}' to locate the target directory (set DT_CPP_NO_DLL_COPY=1 to skip the DLL copy)",
                out_dir.display()
            )
        })?;
        let targets = [dirs.stage_dir.clone(), dirs.artifact_dir.clone()];
        for t in &targets {
            fs::create_dir_all(t)
                .map_err(|e| format!("cannot create '{}' for the DLL copy: {e}", t.display()))?;
        }
        let mut done = BTreeSet::new();
        let mut artifact_copies = Vec::new();
        for dir in &plan.dll_dirs {
            let Ok(entries) = fs::read_dir(dir) else {
                continue; // a missing project directory is reported by warn_missing
            };
            let mut files: Vec<PathBuf> = entries
                .filter_map(Result::ok)
                .map(|e| e.path())
                .filter(|p| p.extension().is_some_and(|x| x.eq_ignore_ascii_case("dll")))
                .collect();
            files.sort();
            for f in files {
                let Some(file_name) = f.file_name() else {
                    continue;
                };
                if !done.insert(file_name.to_string_lossy().to_ascii_lowercase()) {
                    continue;
                }
                artifact_copies.push(dirs.artifact_dir.join(file_name));
                for t in &targets {
                    let dest = t.join(file_name);
                    // Skip identical copies so a running dt-server.exe does not block an unchanged rebuild.
                    if same_file(&f, &dest) {
                        continue;
                    }
                    fs::copy(&f, &dest).map_err(|e| {
                        format!(
                            "cannot copy '{}' to '{}': {e} (set DT_CPP_NO_DLL_COPY=1 to skip the copy)",
                            f.display(),
                            dest.display()
                        )
                    })?;
                }
            }
        }
        Ok((dirs.stage_dir, artifact_copies))
    }

    fn same_file(a: &Path, b: &Path) -> bool {
        match (fs::metadata(a), fs::metadata(b)) {
            (Ok(x), Ok(y)) => x.len() == y.len() && x.modified().ok() == y.modified().ok(),
            _ => false,
        }
    }
}
