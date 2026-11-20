//! Pure link planning for the `ffi` feature: which include directories, library search directories, libraries,
//! rpath entries and DLL directories the C++ facade needs, derived from an existing CMake build tree
//! (`CMakeCache.txt`) and the Conan graph (`conan install ... --format=json`).
//!
//! No file access, no environment access: everything comes in as text or values, so `build.rs` (feature `ffi`)
//! and `tests/link_plan.rs` (always) share the same code and the Linux/macOS logic is testable on Windows.
//! This file is included with `#[path]`, it is not part of the library crate. Only std and serde_json are used.
#![allow(dead_code)]

use std::collections::BTreeMap;
use std::fmt;
use std::path::{Path, PathBuf};

use serde_json::Value;

/// CMake targets of the project libraries, in link order (dependents first). The target name is also the
/// library file name apart from the platform prefix and suffix.
pub const PROJECT_LIBS: [&str; 5] = [
    "DigitalTwinLib",
    "BackendCommunication",
    "PhysicalTwinCommunication",
    "CPSBaseLib",
    "DigitalTwinStore",
];

/// Header directories of the project libraries, relative to the source directory.
pub const PROJECT_INCLUDE_DIRS: [&str; 5] = [
    "CpsBaseLib/src",
    "BackendCommunicationService/src",
    "PhysicalTwinCommunicationService/src",
    "DigitalTwinLib/src",
    "DigitalTwinStore/src",
];

/// The Conan package whose libraries the facade links.
pub const SYSML_PACKAGE: &str = "sysmllib";

/// Error with a message that names the missing file, key or variable.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct LinkPlanError(pub String);

impl fmt::Display for LinkPlanError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&self.0)
    }
}

impl std::error::Error for LinkPlanError {}

fn err<T>(msg: impl Into<String>) -> Result<T, LinkPlanError> {
    Err(LinkPlanError(msg.into()))
}

/// Everything the planner needs. All paths are plain values; nothing is read from disk.
#[derive(Debug, Clone)]
pub struct LinkInput<'a> {
    /// `CARGO_CFG_TARGET_OS`: `linux`, `windows` or `macos`.
    pub target_os: &'a str,
    /// `CARGO_CFG_TARGET_ENV`, e.g. `msvc`, `gnu`, empty on macOS.
    pub target_env: &'a str,
    /// Text of `CMakeCache.txt` (a UTF-8 BOM is tolerated).
    pub cmake_cache: &'a str,
    /// Text of the Conan graph JSON (a UTF-8 BOM is tolerated).
    pub conan_graph: &'a str,
    /// `DT_CPP_CONFIG`, if set.
    pub config: Option<&'a str>,
    /// CMake build directory (`DT_CPP_BUILD_DIR`).
    pub build_dir: &'a Path,
    /// Source directory with the four library directories.
    pub source_dir: &'a Path,
    /// `DT_CPP_EXTRA_LIB_DIRS`, searched first.
    pub extra_lib_dirs: &'a [PathBuf],
}

#[derive(Debug, Clone, Default, PartialEq, Eq)]
pub struct LinkPlan {
    /// The configuration that was selected (also used for the project library directory).
    pub config: String,
    /// `CMAKE_CONFIGURATION_TYPES` was set: libraries are in `out/<config>`.
    pub multi_config: bool,
    /// Directory with the project libraries.
    pub project_lib_dir: PathBuf,
    /// Header directories: project libraries first, then Conan packages.
    pub include_dirs: Vec<PathBuf>,
    /// Library search order: extra directories, Conan packages, then the project `out` directory.
    pub lib_search_dirs: Vec<PathBuf>,
    /// Library names without prefix and suffix (`rustc-link-lib=dylib=...`).
    pub libs: Vec<String>,
    /// Run-time search directories for Linux and macOS (empty on Windows).
    pub rpath_dirs: Vec<PathBuf>,
    /// Windows only: directories whose `*.dll` files are copied next to the binaries. Conan binary
    /// directories first, then the project directory; the first DLL of a name wins.
    pub dll_dirs: Vec<PathBuf>,
    /// Include directories of the sysmllib package (for `cargo:rerun-if-changed`).
    pub sysml_include_dirs: Vec<PathBuf>,
    /// Preprocessor definitions from `cpp_info.defines` of the host packages (`NAME` or `NAME=VALUE`), without
    /// duplicates, as `(name, value)`.
    pub defines: Vec<(String, Option<String>)>,
    /// `CMAKE_CXX_COMPILER` of the C++ tree (not on MSVC); `build.rs` uses it only if `CXX` is not set.
    pub cxx_compiler: Option<String>,
    /// Extra compiler flags for the facade (`-stdlib=libc++` when the Conan packages use libc++ on Linux).
    pub cxx_flags: Vec<String>,
    /// Value for `cc::Build::cpp_set_stdlib` (`c++`), so the facade links libc++.
    pub cpp_set_stdlib: Option<String>,
}

/// Candidate files of the Conan graph when `DT_CPP_CONAN_GRAPH` is not set: in the build directory, then one level
/// up (single-config trees of `cmake_layout` are `build/<build_type>` while the graph is written to `build/`).
pub fn graph_candidates(build_dir: &Path) -> Vec<PathBuf> {
    vec![
        build_dir.join("conan-graph.json"),
        build_dir.join("..").join("conan-graph.json"),
    ]
}

/// If `build_dir` has no `CMakeCache.txt`, a configuration subdirectory that has one is probably the directory
/// meant (`exists` tells whether a file exists; kept as a parameter so this stays pure).
pub fn cache_dir_hint(build_dir: &Path, exists: &dyn Fn(&Path) -> bool) -> Option<PathBuf> {
    ["Release", "RelWithDebInfo", "MinSizeRel", "Debug"]
        .iter()
        .map(|c| build_dir.join(c))
        .find(|d| exists(&d.join("CMakeCache.txt")))
}

/// Error text for a missing `CMakeCache.txt`; names the likely correct `DT_CPP_BUILD_DIR` if there is one.
pub fn missing_cache_message(build_dir: &Path, io_error: &str, hint: Option<&Path>) -> String {
    let mut m = format!(
        "cannot read CMakeCache.txt in '{}': {io_error}. DT_CPP_BUILD_DIR must be the CMake build directory (configured with cmake or `conan build`)",
        build_dir.display()
    );
    if let Some(h) = hint {
        m.push_str(&format!(
            ". '{}' contains a CMakeCache.txt: probably DT_CPP_BUILD_DIR should be that directory (single-config trees of cmake_layout are build/<build_type>)",
            h.display()
        ));
    }
    m
}

/// Directories of a Windows build for the DLLs, derived from `OUT_DIR`.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct RuntimeDirs {
    /// `<OUT_DIR>/dll`: the DLLs are staged here and the directory is emitted as `rustc-link-search`, which makes
    /// cargo put it on `PATH` for `cargo run` and `cargo test`.
    pub stage_dir: PathBuf,
    /// Directory of the final artifacts (`target/<profile>` or `target/<triple>/<profile>`): the DLLs are copied
    /// here as well so `dt-server.exe` can be started directly.
    pub artifact_dir: PathBuf,
}

/// Derives the directories from `OUT_DIR`. Works for both layouts of cargo:
/// old `target/<profile>/build/<pkg>-<hash>/out` and new `target/<profile>/build/<pkg>/<hash>/out`; the artifact
/// directory is the parent of the nearest ancestor named `build`. `None` if there is no such ancestor.
pub fn runtime_dirs(out_dir: &Path) -> Option<RuntimeDirs> {
    let build = out_dir
        .ancestors()
        .skip(1)
        .find(|a| a.file_name().is_some_and(|n| n == "build"))?;
    Some(RuntimeDirs {
        stage_dir: out_dir.join("dll"),
        artifact_dir: build.parent()?.to_path_buf(),
    })
}

/// Parses `KEY:TYPE=VALUE` lines of a `CMakeCache.txt`. Comment lines (`//`, `#`) and malformed lines are skipped.
pub fn parse_cmake_cache(text: &str) -> BTreeMap<String, String> {
    let text = text.strip_prefix('\u{feff}').unwrap_or(text);
    let mut map = BTreeMap::new();
    for line in text.lines() {
        let line = line.trim();
        if line.is_empty() || line.starts_with("//") || line.starts_with('#') {
            continue;
        }
        let Some((left, value)) = line.split_once('=') else {
            continue;
        };
        let key = left.split_once(':').map_or(left, |(k, _)| k);
        if !key.is_empty() {
            map.insert(key.to_string(), value.to_string());
        }
    }
    map
}

fn non_empty<'a>(cache: &'a BTreeMap<String, String>, key: &str) -> Option<&'a str> {
    cache
        .get(key)
        .map(|v| v.trim())
        .filter(|v| !v.is_empty() && !v.ends_with("-NOTFOUND"))
}

/// Absolute in either path style, independent of the host (the plan is tested for other targets on any host).
fn looks_absolute(p: &str) -> bool {
    let b = p.as_bytes();
    p.starts_with('/')
        || p.starts_with('\\')
        || (b.len() >= 2 && b[0].is_ascii_alphabetic() && b[1] == b':')
}

fn push_unique<T: PartialEq>(v: &mut Vec<T>, item: T) {
    if !v.contains(&item) {
        v.push(item);
    }
}

/// Library directory of the project libraries and the selected configuration. The error messages name where the
/// configuration came from.
fn project_dirs(
    cache: &BTreeMap<String, String>,
    build_dir: &Path,
    config: Option<&str>,
) -> Result<(String, bool, PathBuf), LinkPlanError> {
    let out = build_dir.join("out");
    let requested = config.map(str::trim).filter(|c| !c.is_empty());
    let build_type = non_empty(cache, "CMAKE_BUILD_TYPE");
    let (chosen, source) = match (requested, build_type) {
        (Some(c), _) => (c, "DT_CPP_CONFIG"),
        (None, Some(b)) => (b, "CMAKE_BUILD_TYPE in CMakeCache.txt"),
        (None, None) => ("Release", "the default"),
    };
    if let Some(types) = non_empty(cache, "CMAKE_CONFIGURATION_TYPES") {
        let known: Vec<&str> = types
            .split(';')
            .map(str::trim)
            .filter(|t| !t.is_empty())
            .collect();
        if !known.contains(&chosen) {
            return err(format!(
                "configuration '{chosen}' (from {source}) is not one of CMAKE_CONFIGURATION_TYPES in CMakeCache.txt: {}; set DT_CPP_CONFIG to one of them",
                known.join(";")
            ));
        }
        Ok((chosen.to_string(), true, out.join(chosen)))
    } else {
        if let Some(bt) = build_type {
            if bt != chosen {
                return err(format!(
                    "configuration '{chosen}' (from {source}) contradicts CMAKE_BUILD_TYPE={bt} in CMakeCache.txt (single-configuration build tree): unset DT_CPP_CONFIG or build the tree with that configuration"
                ));
            }
        }
        Ok((chosen.to_string(), false, out))
    }
}

/// One host package of the Conan graph with resolved directories.
#[derive(Debug, Default)]
struct Package {
    name: String,
    includedirs: Vec<PathBuf>,
    libdirs: Vec<PathBuf>,
    bindirs: Vec<PathBuf>,
    libs: Vec<String>,
    system_libs: Vec<String>,
    defines: Vec<String>,
    /// Node id and the ids of the nodes it requires (graph `dependencies`, build and test edges left out).
    id: String,
    deps: Vec<String>,
    libcxx: Option<String>,
    build_type: Option<String>,
    runtime_type: Option<String>,
}

fn str_list(v: Option<&Value>) -> Vec<String> {
    v.and_then(Value::as_array)
        .map(|a| {
            a.iter()
                .filter_map(Value::as_str)
                .map(str::to_string)
                .collect()
        })
        .unwrap_or_default()
}

/// Resolves the directories of a `cpp_info` entry. Relative ones are joined with the package folder; without a
/// package folder they cannot be resolved and are dropped.
fn resolve_dirs(v: Option<&Value>, folder: Option<&str>) -> Vec<PathBuf> {
    str_list(v)
        .into_iter()
        .filter_map(|d| {
            if looks_absolute(&d) {
                Some(PathBuf::from(d))
            } else {
                folder.map(|f| Path::new(f).join(d))
            }
        })
        .collect()
}

fn node_name(node: &Value) -> String {
    if let Some(n) = node.get("name").and_then(Value::as_str) {
        return n.to_string();
    }
    node.get("ref")
        .and_then(Value::as_str)
        .and_then(|r| r.split('/').next())
        .unwrap_or("")
        .to_string()
}

/// Host packages of the graph (root consumer and `tool_requires` nodes skipped), in graph order.
struct Graph {
    packages: Vec<Package>,
    /// `compiler.libcxx` of the root (consumer) node.
    root_libcxx: Option<String>,
}

fn host_packages(graph_text: &str) -> Result<Graph, LinkPlanError> {
    let text = graph_text.strip_prefix('\u{feff}').unwrap_or(graph_text);
    let root: Value = serde_json::from_str(text)
        .map_err(|e| LinkPlanError(format!("the Conan graph JSON cannot be parsed: {e}")))?;
    let Some(graph) = root.get("graph") else {
        return err("the Conan graph JSON has no key 'graph' (create it with `conan install ... --format=json`)");
    };
    let Some(nodes) = graph.get("nodes").and_then(Value::as_object) else {
        return err("the Conan graph JSON has no object 'graph.nodes'");
    };
    let root_ids: Vec<String> = graph
        .get("root")
        .and_then(Value::as_object)
        .map(|o| o.keys().cloned().collect())
        .unwrap_or_else(|| vec!["0".to_string()]);

    // Node ids are numeric strings; the object keeps them sorted as text, so sort by number.
    let mut ids: Vec<&String> = nodes.keys().collect();
    ids.sort_by_key(|k| (k.parse::<u64>().unwrap_or(u64::MAX), (*k).clone()));

    let libcxx_of = |node: &Value| {
        node.pointer("/settings/compiler.libcxx")
            .and_then(Value::as_str)
            .map(str::to_string)
    };
    let root_libcxx = root_ids
        .iter()
        .filter_map(|id| nodes.get(id))
        .find_map(libcxx_of);

    let mut out = Vec::new();
    for id in ids {
        let node = &nodes[id];
        if root_ids.contains(id) {
            continue;
        }
        if node.get("context").and_then(Value::as_str) != Some("host") {
            continue;
        }
        // test_requires (gtest) are host context nodes marked "test": they are not part of the facade.
        if node.get("test").and_then(Value::as_bool) == Some(true) {
            continue;
        }
        let Some(info) = node.get("cpp_info").and_then(Value::as_object) else {
            continue;
        };
        let folder = node
            .get("package_folder")
            .and_then(Value::as_str)
            .filter(|f| !f.is_empty());
        let mut p = Package {
            name: node_name(node),
            id: id.clone(),
            libcxx: libcxx_of(node),
            ..Package::default()
        };
        if let Some(deps) = node.get("dependencies").and_then(Value::as_object) {
            for (dep_id, dep) in deps {
                let flag = |k: &str| dep.get(k).and_then(Value::as_bool) == Some(true);
                if !flag("build") && !flag("test") && !flag("skip") {
                    p.deps.push(dep_id.clone());
                }
            }
        }
        p.build_type = node
            .pointer("/settings/build_type")
            .and_then(Value::as_str)
            .map(str::to_string);
        p.runtime_type = node
            .pointer("/settings/compiler.runtime_type")
            .and_then(Value::as_str)
            .map(str::to_string);
        // "root" first, then the components in alphabetical order of their names (serde_json without
        // `preserve_order` keeps object keys sorted), so the result is deterministic.
        let mut entries: Vec<&Value> = Vec::new();
        if let Some(r) = info.get("root") {
            entries.push(r);
        }
        entries.extend(info.iter().filter(|(k, _)| *k != "root").map(|(_, v)| v));
        for e in entries {
            for d in resolve_dirs(e.get("includedirs"), folder) {
                push_unique(&mut p.includedirs, d);
            }
            for d in resolve_dirs(e.get("libdirs"), folder) {
                push_unique(&mut p.libdirs, d);
            }
            for d in resolve_dirs(e.get("bindirs"), folder) {
                push_unique(&mut p.bindirs, d);
            }
            for l in str_list(e.get("libs")) {
                push_unique(&mut p.libs, l);
            }
            for l in str_list(e.get("system_libs")) {
                push_unique(&mut p.system_libs, l);
            }
            for d in str_list(e.get("defines")) {
                push_unique(&mut p.defines, d);
            }
        }
        out.push(p);
    }
    Ok(Graph {
        packages: out,
        root_libcxx,
    })
}

/// Builds the link plan. See the module documentation for the rules.
pub fn plan(input: &LinkInput<'_>) -> Result<LinkPlan, LinkPlanError> {
    match input.target_os {
        "linux" | "windows" | "macos" => {}
        other => {
            return err(format!(
                "target OS '{other}' is not supported by the ffi build (linux, windows, macos)"
            ))
        }
    }
    if input.target_os == "windows" && input.target_env != "msvc" {
        return err(format!(
            "target '{}-windows' is not supported by the ffi build: the C++ libraries are built with MSVC and their DLLs cannot be linked from a GNU target. Use the *-pc-windows-msvc target (stable-x86_64-pc-windows-msvc)",
            if input.target_env.is_empty() { "unknown" } else { input.target_env }
        ));
    }
    let msvc = input.target_os == "windows";

    let cache = parse_cmake_cache(input.cmake_cache);
    if let Some(v) = non_empty(&cache, "BUILD_SHARED_LIBS") {
        if matches!(
            v.to_ascii_uppercase().as_str(),
            "OFF" | "FALSE" | "0" | "NO" | "N"
        ) {
            return err(
                "CMakeCache.txt has BUILD_SHARED_LIBS=OFF: the ffi feature needs the shared build of the C++ libraries (conan option shared=True, the default); rebuild the C++ tree with shared libraries",
            );
        }
    }
    let (config, multi_config, project_lib_dir) =
        project_dirs(&cache, input.build_dir, input.config)?;

    if msvc && config.eq_ignore_ascii_case("debug") {
        return err(
            "the C++ build tree is configured as Debug, which cannot be linked from Rust on MSVC: Rust always links the \
             release C runtime (/MD), the Debug DLLs use /MDd, and std::string is not ABI compatible across that \
             boundary. Build the C++ tree as Release or RelWithDebInfo (conan install/build with -s build_type=Release) \
             and set DT_CPP_CONFIG accordingly",
        );
    }

    let Graph {
        packages,
        root_libcxx,
    } = host_packages(input.conan_graph)?;
    let Some(sysml) = packages.iter().find(|p| p.name == SYSML_PACKAGE) else {
        return err(format!(
            "the Conan graph JSON has no host package '{SYSML_PACKAGE}' in graph.nodes (was it created with \
             `conan install` for this project, with --format=json?)"
        ));
    };
    if sysml.libs.is_empty() {
        return err(format!(
            "package '{SYSML_PACKAGE}' in the Conan graph JSON lists no libraries in cpp_info.libs \
             (was the graph created for a package that is not installed?)"
        ));
    }
    if msvc {
        let debug = |s: &Option<String>| {
            s.as_deref()
                .is_some_and(|v| v.eq_ignore_ascii_case("debug"))
        };
        if debug(&sysml.build_type) || debug(&sysml.runtime_type) {
            return err(format!(
                "package '{SYSML_PACKAGE}' in the Conan graph is a Debug build (settings build_type / \
                 compiler.runtime_type), which cannot be linked from Rust on MSVC (release C runtime /MD). \
                 Run `conan install` with -s build_type=Release (or RelWithDebInfo) and recreate the graph JSON"
            ));
        }
    }

    // Include directories: project headers, then the Conan packages.
    let mut include_dirs = Vec::new();
    for d in PROJECT_INCLUDE_DIRS {
        push_unique(&mut include_dirs, input.source_dir.join(d));
    }
    for p in &packages {
        for d in &p.includedirs {
            push_unique(&mut include_dirs, d.clone());
        }
    }

    // Search order: extra directories, Conan packages, project out directory. The project directory comes last
    // because it may hold older copies of the sysml libraries that must not win over the Conan package.
    let mut lib_search_dirs = Vec::new();
    for d in input.extra_lib_dirs {
        push_unique(&mut lib_search_dirs, d.clone());
    }
    for p in &packages {
        for d in &p.libdirs {
            push_unique(&mut lib_search_dirs, d.clone());
        }
    }
    push_unique(&mut lib_search_dirs, project_lib_dir.clone());

    let mut libs: Vec<String> = PROJECT_LIBS.iter().map(|s| s.to_string()).collect();
    for l in sysml.libs.iter().chain(sysml.system_libs.iter()) {
        push_unique(&mut libs, l.clone());
    }

    let sysml_include_dirs = sysml.includedirs.clone();
    // Defines only from sysmllib and the packages it depends on (not from unrelated packages such as yaml-cpp).
    let mut closure: Vec<&str> = vec![sysml.id.as_str()];
    let mut i = 0;
    while i < closure.len() {
        if let Some(p) = packages.iter().find(|p| p.id == closure[i]) {
            for d in &p.deps {
                if !closure.contains(&d.as_str()) {
                    closure.push(d.as_str());
                }
            }
        }
        i += 1;
    }
    let mut defines: Vec<(String, Option<String>)> = Vec::new();
    for p in packages.iter().filter(|p| closure.contains(&p.id.as_str())) {
        for d in &p.defines {
            let item = match d.split_once('=') {
                Some((n, v)) => (n.to_string(), Some(v.to_string())),
                None => (d.clone(), None),
            };
            push_unique(&mut defines, item);
        }
    }

    // C++ compiler and standard library of the C++ tree (not on MSVC, where cc finds cl itself).
    let mut cxx_compiler = None;
    let mut cxx_flags = Vec::new();
    let mut cpp_set_stdlib = None;
    if !msvc {
        cxx_compiler = non_empty(&cache, "CMAKE_CXX_COMPILER").map(str::to_string);
        let libcxx = root_libcxx.as_deref().or(sysml.libcxx.as_deref());
        match (input.target_os, libcxx) {
            ("linux", Some("libstdc++")) => {
                // the old, pre-C++11 std::string ABI: the facade must use it too
                push_unique(
                    &mut defines,
                    ("_GLIBCXX_USE_CXX11_ABI".to_string(), Some("0".to_string())),
                );
            }
            ("linux", Some("libc++")) => {
                cxx_flags.push("-stdlib=libc++".to_string());
                cpp_set_stdlib = Some("c++".to_string());
            }
            // libstdc++11 (new ABI) is the default of the compiler; on macOS libc++ is the default
            _ => {}
        }
    }

    let rpath_dirs = if input.target_os == "windows" {
        Vec::new()
    } else {
        lib_search_dirs.clone()
    };

    let mut dll_dirs = Vec::new();
    if input.target_os == "windows" {
        for d in input.extra_lib_dirs {
            push_unique(&mut dll_dirs, d.clone());
        }
        for p in &packages {
            for d in &p.bindirs {
                push_unique(&mut dll_dirs, d.clone());
            }
        }
        push_unique(&mut dll_dirs, project_lib_dir.clone());
    }

    Ok(LinkPlan {
        config,
        multi_config,
        project_lib_dir,
        include_dirs,
        lib_search_dirs,
        libs,
        rpath_dirs,
        dll_dirs,
        sysml_include_dirs,
        defines,
        cxx_compiler,
        cxx_flags,
        cpp_set_stdlib,
    })
}
