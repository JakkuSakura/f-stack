mod bindings;
mod tools;


#[cfg(test)]
mod tests {
    use crate::bindings::{fstack_run, fstack_init};
    use crate::tools::CArgs;
    #[test]
    fn test_fstack() {
        unsafe {
            let args = CArgs::from(&["/home/jack/f-stack/rust/test"]);
            let (argc, argv) = args.get();
            fstack_init(argc, argv);
            // fstack_run(None);
        }
    }
}
