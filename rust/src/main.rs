mod bindings;

fn main() {
    unsafe {
        bindings::fstack_run(None);
    }
}
// does not compile https://github.com/rust-lang/rust/issues/79446
