// build.rs - Build script to compile C++ code for Rust FFI

use std::path::PathBuf;

fn main() {
    // Get the absolute path to the build directory
    let build_dir = PathBuf::from("build");
    let abs_build_dir = std::fs::canonicalize(&build_dir)
        .unwrap_or_else(|_| build_dir.clone());
    
    // Tell cargo to look for shared libraries in the build directory
    println!("cargo:rustc-link-search=native={}", abs_build_dir.display());
    
    // Add rpath so the library can be found at runtime
    println!("cargo:rustc-link-arg=-Wl,-rpath,{}", abs_build_dir.display());
    
    // Tell cargo to tell rustc to link the byps_core shared library
    println!("cargo:rustc-link-lib=dylib=byps_core");
    
    // Explicitly add the library with full path
    let lib_path = abs_build_dir.join("libbyps_core.so");
    println!("cargo:rustc-link-arg={}", lib_path.display());
    
    // Also link OpenSSL and other dependencies the C++ library needs
    println!("cargo:rustc-link-lib=dylib=ssl");
    println!("cargo:rustc-link-lib=dylib=crypto");
    println!("cargo:rustc-link-lib=dylib=stdc++");
    
    // Tell cargo to invalidate the built crate whenever the C++ sources change
    println!("cargo:rerun-if-changed=cpp/");
    println!("cargo:rerun-if-changed=build/");
    
    // Check if library exists
    if !lib_path.exists() {
        println!("cargo:warning=C++ library not found at: {}", lib_path.display());
        println!("cargo:warning=Make sure to build the C++ library first using:");
        println!("cargo:warning=  mkdir -p build && cd build && cmake .. && make");
    } else {
        println!("cargo:warning=Found C++ library at: {}", lib_path.display());
    }
}
