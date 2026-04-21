// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//! Build script for gg-sdk.

use std::env;
use std::path::PathBuf;

fn main() {
    unsafe {
        env::set_var("SOURCE_DATE_EPOCH", "0");
    }

    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());

    // Support both layouts:
    // - Development: C sources are in the parent directory (../include, ../src, etc.)
    // - Packaged crate: C sources are copied into the crate directory
    //   (include/, csrc/, etc.)
    let (include_dir, priv_include_dir, mock_dir, c_src_dir);
    if manifest_dir.join("include").is_dir() {
        // Packaged crate layout
        include_dir = manifest_dir.join("include");
        priv_include_dir = manifest_dir.join("priv_include");
        mock_dir = manifest_dir.join("mock");
        c_src_dir = manifest_dir.join("csrc");
    } else {
        // Development layout (rust/ is a subdirectory of the project root)
        let project_root = manifest_dir.parent().unwrap().to_path_buf();
        include_dir = project_root.join("include");
        priv_include_dir = project_root.join("priv_include");
        mock_dir = project_root.join("mock");
        c_src_dir = project_root.join("src");
    }

    bindgen::Builder::default()
        .header(manifest_dir.join("wrapper.h").to_str().unwrap())
        .clang_arg(format!("-I{}", include_dir.display()))
        .clang_arg(format!("-I{}", priv_include_dir.display()))
        .clang_arg(format!("-I{}", mock_dir.display()))
        .default_enum_style(bindgen::EnumVariation::Rust {
            non_exhaustive: false,
        })
        .generate_inline_functions(true)
        .enable_function_attribute_detection()
        .disable_nested_struct_naming()
        .derive_default(true)
        .generate_comments(false)
        .use_core()
        .generate_cstr(true)
        .sort_semantically(true)
        .allowlist_function("gg_.*")
        .allowlist_function("ggipc_.*")
        .allowlist_type("Gg.*")
        .allowlist_type("GgIpc.*")
        .allowlist_var("GG_.*")
        .generate()
        .unwrap()
        .write_to_file(PathBuf::from(env::var("OUT_DIR").unwrap()).join("c.rs"))
        .unwrap();

    let mut src_files = Vec::new();
    let mut dirs = vec![c_src_dir.clone(), mock_dir.clone()];
    while let Some(dir) = dirs.pop() {
        for entry in std::fs::read_dir(dir).unwrap() {
            let entry = entry.unwrap();
            let path = entry.path();
            if path.is_dir() {
                dirs.push(path);
            } else if path.extension().and_then(|s| s.to_str()) == Some("c") {
                src_files.push(path);
            }
        }
    }

    let mut build = cc::Build::new();
    for file in &src_files {
        build.flag(format!("-frandom-seed={}", file.display()));
    }

    build
        .files(&src_files)
        .include(&include_dir)
        .include(&priv_include_dir)
        .include(&mock_dir)
        .include(&manifest_dir)
        .flag("-pthread")
        .flag("-fno-strict-aliasing")
        .flag("-std=gnu11")
        .flag("-Wno-missing-braces")
        .flag("-fno-semantic-interposition")
        .flag("-fno-unwind-tables")
        .flag("-fno-asynchronous-unwind-tables")
        .flag_if_supported("-fstrict-flex-arrays=3")
        .define("_GNU_SOURCE", None)
        .define("GG_MODULE", "\"gg-sdk\"")
        .define("GG_LOG_LEVEL", "GG_LOG_DEBUG");

    if env::var("PROFILE").unwrap() == "release" {
        build.flag("-Oz");
        build.flag("-flto");
        build.flag("-ffat-lto-objects");
    }

    build.compile("gg-sdk");

    println!("cargo:rerun-if-changed=wrapper.h");
    println!("cargo:rerun-if-changed={}", c_src_dir.display());
    println!("cargo:rerun-if-changed={}", include_dir.display());
    println!("cargo:rerun-if-changed={}", priv_include_dir.display());
    println!("cargo:rerun-if-changed={}", mock_dir.display());
}
