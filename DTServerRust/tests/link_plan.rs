//! Tests of the link planner used by `build.rs` (feature `ffi`). The planner is pure, so this runs without the
//! feature and for every target OS on any host. The JSON fixtures follow the shape of
//! `conan install ... --format=json`: `graph.nodes.<id>` with `ref`, `name`, `context`, `package_folder`,
//! `settings` and `cpp_info` (`root` plus components with `includedirs`, `libdirs`, `bindirs`, `libs`).

#[path = "../ffi/link_plan.rs"]
mod link_plan;

use std::path::{Path, PathBuf};

use link_plan::{plan, LinkInput, LinkPlan, PROJECT_LIBS};

const BUILD: &str = "/b";
const SRC: &str = "/src";

fn cache_multi(extra: &str) -> String {
    format!(
        "# comment\n//COMMENT\nCMAKE_BUILD_TYPE:STRING=\nCMAKE_CONFIGURATION_TYPES:STRING=Debug;Release;RelWithDebInfo\n{extra}\n"
    )
}

fn cache_single(build_type: &str) -> String {
    format!("CMAKE_BUILD_TYPE:STRING={build_type}\nCMAKE_GENERATOR:INTERNAL=Ninja\n")
}

/// Graph with a root consumer, a build-context tool (with a library that must not be linked), a Boost-like host
/// package (headers and libdir, no linked libs) and sysmllib with libs in `root` and in a component.
fn graph(build_type: &str) -> String {
    format!(
        r#"{{"graph":{{"root":{{"0":"app/None"}},"nodes":{{
          "0":{{"ref":"app/None","name":null,"context":"host","package_folder":null,
                "cpp_info":{{"root":{{"includedirs":["include"],"libdirs":["lib"],"bindirs":["bin"],"libs":null}}}}}},
          "1":{{"ref":"boost/1.0#a","name":"boost","context":"host","package_folder":"/c/boost",
                "settings":{{"build_type":"{bt}"}},
                "cpp_info":{{"root":{{"includedirs":["/c/boost/include"],"libdirs":["/c/boost/lib"],"bindirs":["/c/boost/bin"],"libs":null}},
                             "headers":{{"includedirs":["/c/boost/include"],"libdirs":[],"bindirs":["/c/boost/bin"],"libs":[]}}}}}},
          "2":{{"ref":"cmake/4.0#b","name":"cmake","context":"build","package_folder":"/c/cmake",
                "cpp_info":{{"root":{{"includedirs":["/c/cmake/include"],"libdirs":["/c/cmake/lib"],"bindirs":["/c/cmake/bin"],"libs":["cmaketool"]}}}}}},
          "3":{{"ref":"sysmllib/2611.1#c","name":"sysmllib","context":"host","package_folder":"/c/sysml",
                "settings":{{"build_type":"{bt}","compiler.runtime_type":"{bt}"}},
                "cpp_info":{{"root":{{"includedirs":["/c/sysml/include"],"libdirs":["/c/sysml/lib"],"bindirs":["/c/sysml/bin"],
                                      "libs":["sysmlv2rest","sysml"],"system_libs":null}},
                             "extra":{{"includedirs":["/c/sysml/include"],"libdirs":["/c/sysml/lib"],"bindirs":[],"libs":["sysml","kerml"],"system_libs":["m"]}}}}}},
          "4":{{"ref":"gtest/1#d","name":"gtest","context":"host","package_folder":null,
                "cpp_info":{{"root":{{"includedirs":["include"],"libdirs":["lib"],"bindirs":["bin"],"libs":["gtest"]}}}}}}
        }}}}}}"#,
        bt = build_type
    )
}

fn input<'a>(
    os: &'a str,
    env: &'a str,
    cache: &'a str,
    graph: &'a str,
    config: Option<&'a str>,
    extra: &'a [PathBuf],
) -> LinkInput<'a> {
    LinkInput {
        target_os: os,
        target_env: env,
        cmake_cache: cache,
        conan_graph: graph,
        config,
        build_dir: Path::new(BUILD),
        source_dir: Path::new(SRC),
        extra_lib_dirs: extra,
    }
}

fn run(
    os: &str,
    env: &str,
    cache: &str,
    graph: &str,
    config: Option<&str>,
) -> Result<LinkPlan, String> {
    plan(&input(os, env, cache, graph, config, &[])).map_err(|e| e.to_string())
}

fn p(s: &str) -> PathBuf {
    PathBuf::from(s)
}

#[test]
fn linux_single_config_uses_out_and_build_type() {
    let r = run(
        "linux",
        "gnu",
        &cache_single("Debug"),
        &graph("Debug"),
        None,
    )
    .unwrap();
    assert!(!r.multi_config);
    assert_eq!(r.config, "Debug");
    assert_eq!(r.project_lib_dir, Path::new(BUILD).join("out"));
    // search order: Conan packages, then out
    assert_eq!(
        r.lib_search_dirs,
        vec![
            p("/c/boost/lib"),
            p("/c/sysml/lib"),
            Path::new(BUILD).join("out")
        ]
    );
    assert_eq!(r.rpath_dirs, r.lib_search_dirs);
    assert!(r.dll_dirs.is_empty());
}

#[test]
fn linux_debug_is_fine_only_msvc_rejects_it() {
    assert!(run(
        "linux",
        "gnu",
        &cache_single("Debug"),
        &graph("Debug"),
        Some("Debug")
    )
    .is_ok());
    assert!(run("macos", "", &cache_single("Debug"), &graph("Debug"), None).is_ok());
}

#[test]
fn macos_plan_has_rpath_and_no_dlls() {
    let r = run(
        "macos",
        "",
        &cache_single("Release"),
        &graph("Release"),
        None,
    )
    .unwrap();
    assert_eq!(r.rpath_dirs.last().unwrap(), &Path::new(BUILD).join("out"));
    assert!(r.dll_dirs.is_empty());
}

#[test]
fn windows_multi_config_selects_config_directory_and_dlls() {
    let c = cache_multi("");
    let r = run(
        "windows",
        "msvc",
        &c,
        &graph("Release"),
        Some("RelWithDebInfo"),
    )
    .unwrap();
    assert!(r.multi_config);
    assert_eq!(r.config, "RelWithDebInfo");
    assert_eq!(
        r.project_lib_dir,
        Path::new(BUILD).join("out").join("RelWithDebInfo")
    );
    assert!(r.rpath_dirs.is_empty());
    // Conan bin dirs first, project dir last; the build-context tool is absent
    assert_eq!(
        r.dll_dirs,
        vec![
            p("/c/boost/bin"),
            p("/c/sysml/bin"),
            r.project_lib_dir.clone()
        ]
    );
}

#[test]
fn default_config_is_release_without_build_type() {
    let r = run("windows", "msvc", &cache_multi(""), &graph("Release"), None).unwrap();
    assert_eq!(r.config, "Release");
    assert!(r.project_lib_dir.ends_with("Release"));
}

#[test]
fn default_config_is_cmake_build_type_when_set() {
    let c = "CMAKE_BUILD_TYPE:STRING=RelWithDebInfo\nCMAKE_CONFIGURATION_TYPES:STRING=Debug;Release;RelWithDebInfo\n";
    let r = run("windows", "msvc", c, &graph("Release"), None).unwrap();
    assert_eq!(r.config, "RelWithDebInfo");
}

#[test]
fn msvc_debug_is_an_error_with_advice() {
    let e = run(
        "windows",
        "msvc",
        &cache_multi(""),
        &graph("Release"),
        Some("Debug"),
    )
    .unwrap_err();
    assert!(e.contains("Debug"), "{e}");
    assert!(e.contains("Release or RelWithDebInfo"), "{e}");
    // also for a single-configuration tree whose build type is Debug
    let e = run(
        "windows",
        "msvc",
        &cache_single("Debug"),
        &graph("Release"),
        None,
    )
    .unwrap_err();
    assert!(e.contains("Release or RelWithDebInfo"), "{e}");
}

#[test]
fn msvc_debug_conan_package_is_an_error() {
    let e = run(
        "windows",
        "msvc",
        &cache_multi(""),
        &graph("Debug"),
        Some("Release"),
    )
    .unwrap_err();
    assert!(e.contains("sysmllib") && e.contains("Debug"), "{e}");
}

#[test]
fn unknown_config_in_multi_config_tree_is_an_error() {
    let e = run(
        "windows",
        "msvc",
        &cache_multi(""),
        &graph("Release"),
        Some("Fancy"),
    )
    .unwrap_err();
    assert!(
        e.contains("Fancy") && e.contains("CMAKE_CONFIGURATION_TYPES"),
        "{e}"
    );
}

#[test]
fn config_that_contradicts_single_config_tree_is_an_error() {
    let e = run(
        "linux",
        "gnu",
        &cache_single("Release"),
        &graph("Release"),
        Some("Debug"),
    )
    .unwrap_err();
    assert!(e.contains("CMAKE_BUILD_TYPE=Release"), "{e}");
}

#[test]
fn libs_are_project_libs_then_sysmllib_libs_only() {
    let r = run(
        "linux",
        "gnu",
        &cache_single("Release"),
        &graph("Release"),
        None,
    )
    .unwrap();
    let mut expected: Vec<String> = PROJECT_LIBS.iter().map(|s| s.to_string()).collect();
    expected.extend(["sysmlv2rest", "sysml", "kerml", "m"].map(String::from));
    assert_eq!(r.libs, expected);
    assert!(!r.libs.contains(&"cmaketool".to_string()));
    assert!(!r.libs.contains(&"gtest".to_string()));
}

#[test]
fn build_context_and_root_nodes_are_skipped() {
    let r = run(
        "linux",
        "gnu",
        &cache_single("Release"),
        &graph("Release"),
        None,
    )
    .unwrap();
    let all: Vec<&PathBuf> = r.include_dirs.iter().chain(&r.lib_search_dirs).collect();
    assert!(all.iter().all(|d| !d.starts_with("/c/cmake")), "{all:?}");
    // the root node and the not installed gtest node only have relative directories: nothing of them is used
    assert!(
        all.iter()
            .all(|d| d.starts_with("/c") || d.starts_with("/src") || d.starts_with("/b")),
        "{all:?}"
    );
}

#[test]
fn include_dirs_start_with_the_project_headers() {
    let r = run(
        "linux",
        "gnu",
        &cache_single("Release"),
        &graph("Release"),
        None,
    )
    .unwrap();
    assert_eq!(r.include_dirs[0], Path::new(SRC).join("CpsBaseLib/src"));
    assert_eq!(r.include_dirs[3], Path::new(SRC).join("DigitalTwinLib/src"));
    assert_eq!(
        &r.include_dirs[4..],
        &[p("/c/boost/include"), p("/c/sysml/include")]
    );
}

#[test]
fn search_order_is_extra_then_conan_then_out() {
    let extra = vec![p("/x/one"), p("/x/two")];
    let g = graph("Release");
    let c = cache_single("Release");
    let r = plan(&input("linux", "gnu", &c, &g, None, &extra)).unwrap();
    assert_eq!(
        r.lib_search_dirs,
        vec![
            p("/x/one"),
            p("/x/two"),
            p("/c/boost/lib"),
            p("/c/sysml/lib"),
            Path::new(BUILD).join("out")
        ]
    );
    let c = cache_multi("");
    let r = plan(&input("windows", "msvc", &c, &g, None, &extra)).unwrap();
    assert_eq!(&r.dll_dirs[..2], &[p("/x/one"), p("/x/two")]);
}

#[test]
fn missing_sysmllib_names_the_package() {
    let g = r#"{"graph":{"nodes":{"0":{"ref":"app/None","context":"host","cpp_info":{"root":{}}},
        "1":{"ref":"boost/1#a","name":"boost","context":"host","package_folder":"/c/boost",
        "cpp_info":{"root":{"includedirs":["/c/boost/include"]}}}}}}"#;
    let e = run("linux", "gnu", &cache_single("Release"), g, None).unwrap_err();
    assert!(e.contains("sysmllib"), "{e}");
}

#[test]
fn sysmllib_only_in_build_context_counts_as_missing() {
    let g = r#"{"graph":{"nodes":{"1":{"ref":"sysmllib/1#a","name":"sysmllib","context":"build","package_folder":"/c/s",
        "cpp_info":{"root":{"libs":["sysml"],"libdirs":["/c/s/lib"]}}}}}}"#;
    let e = run("linux", "gnu", &cache_single("Release"), g, None).unwrap_err();
    assert!(e.contains("sysmllib"), "{e}");
}

#[test]
fn invalid_input_gives_readable_errors() {
    let e = run("linux", "gnu", &cache_single("Release"), "not json", None).unwrap_err();
    assert!(e.contains("Conan graph JSON"), "{e}");
    let e = run("linux", "gnu", &cache_single("Release"), "{}", None).unwrap_err();
    assert!(e.contains("'graph'"), "{e}");
    let e = run(
        "linux",
        "gnu",
        &cache_single("Release"),
        r#"{"graph":{}}"#,
        None,
    )
    .unwrap_err();
    assert!(e.contains("graph.nodes"), "{e}");
    let e = run(
        "freebsd",
        "",
        &cache_single("Release"),
        &graph("Release"),
        None,
    )
    .unwrap_err();
    assert!(e.contains("freebsd"), "{e}");
}

#[test]
fn utf8_bom_is_tolerated_in_cache_and_graph() {
    let c = format!("\u{feff}{}", cache_single("Release"));
    let g = format!("\u{feff}{}", graph("Release"));
    let r = run("linux", "gnu", &c, &g, None).unwrap();
    assert_eq!(r.config, "Release");
    // the BOM must not hide the first cache variable
    assert_eq!(
        link_plan::parse_cmake_cache(&c)
            .get("CMAKE_BUILD_TYPE")
            .map(String::as_str),
        Some("Release")
    );
}

#[test]
fn relative_conan_dirs_are_joined_with_the_package_folder() {
    let g = r#"{"graph":{"nodes":{"1":{"ref":"sysmllib/1#a","name":"sysmllib","context":"host","package_folder":"/c/s",
        "cpp_info":{"root":{"includedirs":["include"],"libdirs":["lib"],"bindirs":["bin"],"libs":["sysml"]}}}}}}"#;
    let r = run("windows", "msvc", &cache_multi(""), g, Some("Release")).unwrap();
    assert!(r.lib_search_dirs.contains(&Path::new("/c/s").join("lib")));
    assert!(r.dll_dirs.contains(&Path::new("/c/s").join("bin")));
}

#[test]
fn windows_drive_paths_are_kept_as_they_are() {
    let g = r#"{"graph":{"nodes":{"1":{"ref":"sysmllib/1#a","name":"sysmllib","context":"host","package_folder":"C:\\p",
        "cpp_info":{"root":{"includedirs":["C:\\p\\include"],"libdirs":["C:\\p\\lib"],"bindirs":[],"libs":["sysml"]}}}}}}"#;
    let r = run("windows", "msvc", &cache_multi(""), g, Some("Release")).unwrap();
    assert!(r.lib_search_dirs.contains(&p("C:\\p\\lib")));
}

// ---------------------------------------------------------------------------------------------------------
// Fixture trimmed from the shape of a real `conan install --format=json` on Windows (paths are synthetic).

fn real_shape_graph(bt: &str) -> String {
    format!(
        r#"{{"graph":{{"nodes":{{
          "0":{{"ref":"digitaltwin/None","name":"digitaltwin","context":"host","test":false,"binary":null,"package_folder":null,
                "cpp_info":{{"root":{{"includedirs":["include"],"libdirs":["lib"],"bindirs":["bin"],"libs":null,"system_libs":null}}}}}},
          "1":{{"ref":"openssl/3.6.5#aa","name":"openssl","context":"host","test":false,"binary":"Download",
                "package_folder":"C:\\conan\\p\\ossl\\p","settings":{{"build_type":"{bt}"}},
                "cpp_info":{{"root":{{"includedirs":["C:\\conan\\p\\ossl\\p\\include"],"libdirs":["C:\\conan\\p\\ossl\\p\\lib"],"bindirs":["C:\\conan\\p\\ossl\\p\\bin"],"libs":null,"system_libs":null}},
                             "ssl":{{"includedirs":["C:\\conan\\p\\ossl\\p\\include"],"libdirs":["C:\\conan\\p\\ossl\\p\\lib"],"bindirs":["C:\\conan\\p\\ossl\\p\\bin"],"libs":["libssl"],"system_libs":[]}},
                             "crypto":{{"includedirs":["C:\\conan\\p\\ossl\\p\\include"],"libdirs":["C:\\conan\\p\\ossl\\p\\lib"],"bindirs":["C:\\conan\\p\\ossl\\p\\bin"],"libs":["libcrypto"],"system_libs":["crypt32"]}}}}}},
          "2":{{"ref":"cmake/4.4.4#bb","name":"cmake","context":"build","test":false,"binary":"Download","package_folder":"C:\\conan\\p\\cmake\\p",
                "cpp_info":{{"root":{{"includedirs":["C:\\conan\\p\\cmake\\p\\include"],"libdirs":["C:\\conan\\p\\cmake\\p\\lib"],"bindirs":["C:\\conan\\p\\cmake\\p\\bin"],"libs":null}}}}}},
          "3":{{"ref":"sysmllib/2611.1#cc","name":"sysmllib","context":"host","test":false,"binary":"Download",
                "package_folder":"C:\\conan\\p\\sysml\\p","settings":{{"build_type":"{bt}","compiler.runtime_type":"{bt}"}},
                "cpp_info":{{"root":{{"includedirs":["C:\\conan\\p\\sysml\\p\\include"],"libdirs":["C:\\conan\\p\\sysml\\p\\lib"],"bindirs":["C:\\conan\\p\\sysml\\p\\bin"],
                                      "libs":["sysmlv2service","sysmlv2rest","sysml","kerml","sysmlv2resources"],"system_libs":null,"defines":["SYSML_USE_X","SYSML_LEVEL=2"]}}}}}},
          "4":{{"ref":"bzip2/1.0.8#dd","name":"bzip2","context":"host","test":false,"binary":"Skip","package_folder":null,
                "cpp_info":{{"root":{{"includedirs":["include"],"libdirs":["lib"],"bindirs":["bin"],"libs":null}}}}}},
          "5":{{"ref":"gtest/1.18.0#ee","name":"gtest","context":"host","test":true,"binary":"Download","package_folder":"C:\\conan\\p\\gtest\\p",
                "cpp_info":{{"root":{{"includedirs":["C:\\conan\\p\\gtest\\p\\include"],"libdirs":["C:\\conan\\p\\gtest\\p\\lib"],"bindirs":["C:\\conan\\p\\gtest\\p\\bin"],"libs":null,"defines":["GTEST_ONLY"]}},
                             "libgtest":{{"includedirs":["C:\\conan\\p\\gtest\\p\\include"],"libdirs":["C:\\conan\\p\\gtest\\p\\lib"],"bindirs":[],"libs":["gtest"]}}}}}}
        }}}}}}"#,
        bt = bt
    )
}

#[test]
fn real_shape_graph_without_root_key() {
    let r = run(
        "windows",
        "msvc",
        &cache_multi(""),
        &real_shape_graph("Release"),
        Some("Release"),
    )
    .unwrap();
    // no `graph.root`: node "0" is the consumer
    let all: Vec<String> = r
        .include_dirs
        .iter()
        .chain(&r.lib_search_dirs)
        .chain(&r.dll_dirs)
        .map(|d| d.to_string_lossy().into_owned())
        .collect();
    assert!(
        all.iter().any(|d| d == "C:\\conan\\p\\ossl\\p\\lib"),
        "{all:?}"
    );
    assert!(
        all.iter().any(|d| d == "C:\\conan\\p\\sysml\\p\\bin"),
        "{all:?}"
    );
    // build context, test and skipped nodes contribute nothing
    assert!(
        !all.iter()
            .any(|d| d.contains("cmake") || d.contains("gtest")),
        "{all:?}"
    );
    assert!(
        !all.iter()
            .any(|d| d == "include" || d == "lib" || d == "bin"),
        "{all:?}"
    );
    // only sysmllib's libs are linked; the multi-component package (openssl) is not
    assert_eq!(
        &r.libs[4..],
        [
            "sysmlv2service",
            "sysmlv2rest",
            "sysml",
            "kerml",
            "sysmlv2resources"
        ]
    );
    assert!(!r.libs.iter().any(|l| l == "libssl" || l == "gtest"));
    assert_eq!(
        r.sysml_include_dirs,
        vec![p("C:\\conan\\p\\sysml\\p\\include")]
    );
}

#[test]
fn multi_component_package_directories_are_collected_once() {
    let r = run(
        "windows",
        "msvc",
        &cache_multi(""),
        &real_shape_graph("Release"),
        Some("Release"),
    )
    .unwrap();
    let n = r
        .lib_search_dirs
        .iter()
        .filter(|d| d.to_string_lossy() == "C:\\conan\\p\\ossl\\p\\lib")
        .count();
    assert_eq!(n, 1);
}

#[test]
fn test_nodes_are_skipped_including_their_defines() {
    let r = run(
        "windows",
        "msvc",
        &cache_multi(""),
        &real_shape_graph("Release"),
        Some("Release"),
    )
    .unwrap();
    assert!(
        !r.defines.iter().any(|(n, _)| n == "GTEST_ONLY"),
        "{:?}",
        r.defines
    );
}

#[test]
fn defines_are_split_into_name_and_value() {
    let r = run(
        "windows",
        "msvc",
        &cache_multi(""),
        &real_shape_graph("Release"),
        Some("Release"),
    )
    .unwrap();
    assert_eq!(
        r.defines,
        vec![
            ("SYSML_USE_X".to_string(), None),
            ("SYSML_LEVEL".to_string(), Some("2".to_string()))
        ]
    );
}

#[test]
fn multi_config_on_linux_and_macos_puts_the_config_dir_in_rpath() {
    for os in ["linux", "macos"] {
        let env = if os == "linux" { "gnu" } else { "" };
        let r = run(
            os,
            env,
            &cache_multi(""),
            &graph("Release"),
            Some("RelWithDebInfo"),
        )
        .unwrap();
        let cfg = Path::new(BUILD).join("out").join("RelWithDebInfo");
        assert_eq!(r.rpath_dirs.last(), Some(&cfg), "{os}");
        assert_eq!(r.lib_search_dirs.last(), Some(&cfg), "{os}");
    }
}

#[test]
fn release_like_configs_are_accepted_on_msvc() {
    let c = "CMAKE_CONFIGURATION_TYPES:STRING=Debug;Release;MinSizeRel;RelWithDebInfo\n";
    for cfg in ["Release", "MinSizeRel", "RelWithDebInfo"] {
        let r = run("windows", "msvc", c, &graph("Release"), Some(cfg)).unwrap();
        assert_eq!(r.config, cfg);
    }
}

#[test]
fn windows_gnu_target_is_rejected() {
    let e = run(
        "windows",
        "gnu",
        &cache_multi(""),
        &graph("Release"),
        Some("Release"),
    )
    .unwrap_err();
    assert!(e.contains("MSVC") && e.contains("gnu"), "{e}");
}

#[test]
fn static_cpp_build_is_rejected() {
    let c = format!("BUILD_SHARED_LIBS:BOOL=OFF\n{}", cache_single("Release"));
    let e = run("linux", "gnu", &c, &graph("Release"), None).unwrap_err();
    assert!(
        e.contains("BUILD_SHARED_LIBS") && e.contains("shared"),
        "{e}"
    );
    let c = format!("BUILD_SHARED_LIBS:BOOL=ON\n{}", cache_single("Release"));
    assert!(run("linux", "gnu", &c, &graph("Release"), None).is_ok());
}

#[test]
fn config_errors_name_the_real_source() {
    // from CMAKE_BUILD_TYPE
    let c = "CMAKE_BUILD_TYPE:STRING=Fancy\nCMAKE_CONFIGURATION_TYPES:STRING=Debug;Release\n";
    let e = run("windows", "msvc", c, &graph("Release"), None).unwrap_err();
    assert!(
        e.contains("CMAKE_BUILD_TYPE") && !e.contains("(from DT_CPP_CONFIG)"),
        "{e}"
    );
    // from the default
    let c = "CMAKE_CONFIGURATION_TYPES:STRING=Debug;MinSizeRel\n";
    let e = run("windows", "msvc", c, &graph("Release"), None).unwrap_err();
    assert!(
        e.contains("the default") && !e.contains("(from DT_CPP_CONFIG)"),
        "{e}"
    );
    // from DT_CPP_CONFIG
    let e = run("windows", "msvc", c, &graph("Release"), Some("Fancy")).unwrap_err();
    assert!(e.contains("(from DT_CPP_CONFIG)"), "{e}");
}

#[test]
fn runtime_dirs_old_cargo_layout() {
    let out = Path::new("/w/target/debug/build/dt-server-0123abcd/out");
    let d = link_plan::runtime_dirs(out).unwrap();
    assert_eq!(d.artifact_dir, p("/w/target/debug"));
    assert_eq!(d.stage_dir, out.join("dll"));
}

#[test]
fn runtime_dirs_new_cargo_layout_and_cross_target() {
    let d =
        link_plan::runtime_dirs(Path::new("/w/target/debug/build/dt-server/0123abcd/out")).unwrap();
    assert_eq!(d.artifact_dir, p("/w/target/debug"));
    let d = link_plan::runtime_dirs(Path::new(
        "/w/ct/x86_64-pc-windows-msvc/release/build/dt-server/0123abcd/out",
    ))
    .unwrap();
    assert_eq!(d.artifact_dir, p("/w/ct/x86_64-pc-windows-msvc/release"));
    // a directory named build further up does not confuse it
    let d = link_plan::runtime_dirs(Path::new(
        "/home/u/build/proj/target/debug/build/dt-server/ab/out",
    ))
    .unwrap();
    assert_eq!(d.artifact_dir, p("/home/u/build/proj/target/debug"));
}

#[test]
fn runtime_dirs_without_build_ancestor_is_none() {
    assert!(link_plan::runtime_dirs(Path::new("/tmp/somewhere/out")).is_none());
}

// ---------------------------------------------------------------------------------------------------------
// Compiler / C++ runtime decisions, define scoping, candidate paths.

/// Graph with libcxx settings and dependency edges: sysmllib -> boost (both with defines), yaml-cpp is unrelated
/// (a define that must not reach the facade).
fn graph_cxx(root_libcxx: Option<&str>, sysml_libcxx: Option<&str>) -> String {
    let settings = |l: Option<&str>| match l {
        Some(l) => format!(r#"{{"build_type":"Release","compiler.libcxx":"{l}"}}"#),
        None => r#"{"build_type":"Release"}"#.to_string(),
    };
    format!(
        r#"{{"graph":{{"root":{{"0":"app/None"}},"nodes":{{
          "0":{{"ref":"app/None","name":"app","context":"host","test":false,"package_folder":null,"settings":{root},
                "dependencies":{{"1":{{"build":false,"test":false,"skip":false}}}},
                "cpp_info":{{"root":{{"includedirs":["include"]}}}}}},
          "1":{{"ref":"sysmllib/1#a","name":"sysmllib","context":"host","test":false,"package_folder":"/c/sysml","settings":{sysml},
                "dependencies":{{"2":{{"build":false,"test":false,"skip":false}},"5":{{"build":true,"test":false,"skip":false}}}},
                "cpp_info":{{"root":{{"includedirs":["/c/sysml/include"],"libdirs":["/c/sysml/lib"],"bindirs":[],"libs":["sysml"],"defines":["SYSML_X=1"]}}}}}},
          "2":{{"ref":"boost/1#b","name":"boost","context":"host","test":false,"package_folder":"/c/boost","settings":{sysml},
                "dependencies":{{}},
                "cpp_info":{{"root":{{"includedirs":["/c/boost/include"],"libdirs":[],"bindirs":[],"libs":null,"defines":["BOOST_ALL_NO_LIB"]}}}}}},
          "3":{{"ref":"yaml-cpp/1#c","name":"yaml-cpp","context":"host","test":false,"package_folder":"/c/yaml","settings":{sysml},
                "dependencies":{{}},
                "cpp_info":{{"root":{{"includedirs":["/c/yaml/include"],"libdirs":[],"bindirs":[],"libs":["yaml-cpp"],"defines":["_NOEXCEPT=noexcept"]}}}}}},
          "5":{{"ref":"cmake/4#d","name":"cmake","context":"build","test":false,"package_folder":"/c/cmake",
                "cpp_info":{{"root":{{"defines":["TOOL_ONLY"]}}}}}}
        }}}}}}"#,
        root = settings(root_libcxx),
        sysml = settings(sysml_libcxx)
    )
}

fn cache_with_compiler() -> String {
    format!(
        "CMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++-13\n{}",
        cache_single("Release")
    )
}

fn has_define(r: &LinkPlan, name: &str, value: Option<&str>) -> bool {
    r.defines
        .iter()
        .any(|(n, v)| n == name && v.as_deref() == value)
}

#[test]
fn defines_come_only_from_sysmllib_and_its_dependencies() {
    let r = run(
        "linux",
        "gnu",
        &cache_single("Release"),
        &graph_cxx(None, None),
        None,
    )
    .unwrap();
    assert!(has_define(&r, "SYSML_X", Some("1")));
    assert!(has_define(&r, "BOOST_ALL_NO_LIB", None));
    assert!(
        !r.defines.iter().any(|(n, _)| n == "_NOEXCEPT"),
        "{:?}",
        r.defines
    );
    assert!(
        !r.defines.iter().any(|(n, _)| n == "TOOL_ONLY"),
        "{:?}",
        r.defines
    );
    // all host packages still contribute directories
    assert!(r.include_dirs.contains(&p("/c/yaml/include")));
}

#[test]
fn old_libstdcxx_abi_defines_the_macro_on_linux() {
    let r = run(
        "linux",
        "gnu",
        &cache_single("Release"),
        &graph_cxx(Some("libstdc++"), None),
        None,
    )
    .unwrap();
    assert!(has_define(&r, "_GLIBCXX_USE_CXX11_ABI", Some("0")));
    assert!(r.cxx_flags.is_empty() && r.cpp_set_stdlib.is_none());
}

#[test]
fn new_libstdcxx_abi_changes_nothing() {
    let r = run(
        "linux",
        "gnu",
        &cache_single("Release"),
        &graph_cxx(Some("libstdc++11"), None),
        None,
    )
    .unwrap();
    assert!(!r.defines.iter().any(|(n, _)| n == "_GLIBCXX_USE_CXX11_ABI"));
    assert!(r.cxx_flags.is_empty() && r.cpp_set_stdlib.is_none());
}

#[test]
fn libcxx_on_linux_uses_stdlib_flag_and_links_libcxx() {
    let r = run(
        "linux",
        "gnu",
        &cache_single("Release"),
        &graph_cxx(Some("libc++"), None),
        None,
    )
    .unwrap();
    assert_eq!(r.cxx_flags, vec!["-stdlib=libc++".to_string()]);
    assert_eq!(r.cpp_set_stdlib.as_deref(), Some("c++"));
    assert!(!r.defines.iter().any(|(n, _)| n == "_GLIBCXX_USE_CXX11_ABI"));
}

#[test]
fn libcxx_on_macos_keeps_the_default_runtime() {
    let r = run(
        "macos",
        "",
        &cache_single("Release"),
        &graph_cxx(Some("libc++"), None),
        None,
    )
    .unwrap();
    assert!(r.cxx_flags.is_empty() && r.cpp_set_stdlib.is_none());
    let r = run(
        "macos",
        "",
        &cache_single("Release"),
        &graph_cxx(Some("libstdc++"), None),
        None,
    )
    .unwrap();
    assert!(!r.defines.iter().any(|(n, _)| n == "_GLIBCXX_USE_CXX11_ABI"));
}

#[test]
fn libcxx_falls_back_to_the_sysmllib_node() {
    let r = run(
        "linux",
        "gnu",
        &cache_single("Release"),
        &graph_cxx(None, Some("libstdc++")),
        None,
    )
    .unwrap();
    assert!(has_define(&r, "_GLIBCXX_USE_CXX11_ABI", Some("0")));
    // the root node wins over sysmllib
    let r = run(
        "linux",
        "gnu",
        &cache_single("Release"),
        &graph_cxx(Some("libstdc++11"), Some("libstdc++")),
        None,
    )
    .unwrap();
    assert!(!r.defines.iter().any(|(n, _)| n == "_GLIBCXX_USE_CXX11_ABI"));
}

#[test]
fn compiler_comes_from_the_cache_except_on_msvc() {
    let r = run(
        "linux",
        "gnu",
        &cache_with_compiler(),
        &graph_cxx(None, None),
        None,
    )
    .unwrap();
    assert_eq!(r.cxx_compiler.as_deref(), Some("/usr/bin/g++-13"));
    let r = run(
        "linux",
        "gnu",
        &cache_single("Release"),
        &graph_cxx(None, None),
        None,
    )
    .unwrap();
    assert_eq!(r.cxx_compiler, None);
    let c = format!(
        "CMAKE_CXX_COMPILER:FILEPATH=C:/VS/cl.exe\n{}",
        cache_multi("")
    );
    let r = run(
        "windows",
        "msvc",
        &c,
        &graph_cxx(None, None),
        Some("Release"),
    )
    .unwrap();
    assert_eq!(r.cxx_compiler, None);
    assert!(r.cxx_flags.is_empty() && r.cpp_set_stdlib.is_none());
}

#[test]
fn graph_candidates_are_build_dir_then_parent() {
    let c = link_plan::graph_candidates(Path::new("/r/build/Release"));
    assert_eq!(c.len(), 2);
    assert_eq!(c[0], Path::new("/r/build/Release").join("conan-graph.json"));
    assert_eq!(
        c[1],
        Path::new("/r/build/Release")
            .join("..")
            .join("conan-graph.json")
    );
}

#[test]
fn cache_dir_hint_finds_the_config_subdirectory() {
    let exists = |f: &Path| f == Path::new("/r/build").join("Release").join("CMakeCache.txt");
    assert_eq!(
        link_plan::cache_dir_hint(Path::new("/r/build"), &exists),
        Some(Path::new("/r/build").join("Release"))
    );
    let none = |_: &Path| false;
    assert_eq!(
        link_plan::cache_dir_hint(Path::new("/r/build"), &none),
        None
    );
    // Release is preferred over Debug when both exist
    let both = |f: &Path| f.ends_with("CMakeCache.txt");
    assert_eq!(
        link_plan::cache_dir_hint(Path::new("/r/build"), &both),
        Some(Path::new("/r/build").join("Release"))
    );
}

#[test]
fn missing_cache_message_names_the_probable_directory() {
    let hint = Path::new("/r/build").join("Release");
    let m = link_plan::missing_cache_message(Path::new("/r/build"), "not found", Some(&hint));
    assert!(
        m.contains("CMakeCache.txt") && m.contains("DT_CPP_BUILD_DIR"),
        "{m}"
    );
    assert!(
        m.contains(&hint.display().to_string()) && m.contains("probably"),
        "{m}"
    );
    let m = link_plan::missing_cache_message(Path::new("/r/build"), "not found", None);
    assert!(!m.contains("probably"), "{m}");
    assert!(!m.contains("  "), "no runs of spaces: {m}");
}

#[test]
fn error_messages_have_no_runs_of_spaces() {
    let errs = [
        run(
            "windows",
            "gnu",
            &cache_multi(""),
            &graph("Release"),
            Some("Release"),
        )
        .unwrap_err(),
        run(
            "linux",
            "gnu",
            "BUILD_SHARED_LIBS:BOOL=OFF\n",
            &graph("Release"),
            None,
        )
        .unwrap_err(),
        run(
            "windows",
            "msvc",
            &cache_multi(""),
            &graph("Release"),
            Some("Fancy"),
        )
        .unwrap_err(),
        run(
            "linux",
            "gnu",
            &cache_single("Release"),
            &graph("Release"),
            Some("Debug"),
        )
        .unwrap_err(),
        run(
            "windows",
            "msvc",
            &cache_multi(""),
            &graph("Release"),
            Some("Debug"),
        )
        .unwrap_err(),
    ];
    for e in errs {
        assert!(!e.contains("  "), "{e}");
    }
}
