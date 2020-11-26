extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    println!("cargo:rustc-link-search=../example");

    let dep = vec!["fstack_daemon", "fstack", "dpdk", "rt", "dl", "crypto", "numa"];
    for d in dep {
        println!("cargo:rustc-link-lib={}", d);
    }


    // Tell cargo to invalidate the built crate whenever the wrapper changes
    println!("cargo:rerun-if-changed=wrapper.h");

    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header("wrapper.h")
        .whitelist_function("get_address")
        .whitelist_function("fstack_init")
        .whitelist_function("fstack_run")
        .whitelist_type("dispatch_t")
        .whitelist_type("control_t")
        .whitelist_type("sockaddr_in")
        .whitelist_type("commands")
        .whitelist_type("mutex_stack_t")
        .whitelist_function("mutex_stack_init")
        .whitelist_function("mutex_stack_push")
        .whitelist_function("mutex_stack_pop")
        .whitelist_function("mutex_stack_size")

        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from("src");
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}
