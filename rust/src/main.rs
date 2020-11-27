fn main() {
    unsafe {
        let args = fstack::tools::CArgs::from(&["/home/jack/f-stack/rust/test"]);
        let (argc, argv) = args.get();
        fstack::bindings::fstack_init(argc, argv);
        // fstack_run(None);
    }
}