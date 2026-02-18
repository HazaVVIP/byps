// build.rs - Build script to compile C++ code for Rust FFI

fn main() {
    // Tell cargo to look for shared libraries in the build directory
    println!("cargo:rustc-link-search=native=build");
    
    // Tell cargo to tell rustc to link the byps_core shared library
    println!("cargo:rustc-link-lib=dylib=byps_core");
    
    // Tell cargo to invalidate the built crate whenever the C++ sources change
    println!("cargo:rerun-if-changed=cpp/");
    
    // Note: The C++ library should be built separately using CMake
    // This build script just sets up the linking
    println!("cargo:warning=Make sure to build the C++ library first using:");
    println!("cargo:warning=  mkdir -p build && cd build && cmake .. && make");
}
