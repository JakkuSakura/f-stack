use std::ffi::CString;
use std::os::raw::{c_char, c_int};
use std::ptr;

pub struct CArgs {
    args: Vec<CString>,
    c_args: Vec<*const c_char>,
}


impl CArgs {
    pub fn new() -> CArgs {
        // create a vector of zero terminated strings
        let args = std::env::args().map(|arg| CString::new(arg).unwrap()).collect::<Vec<CString>>();
        // convert the strings to raw pointers
        let mut c_args = args.iter().map(|arg| arg.as_ptr()).collect::<Vec<*const c_char>>();
        c_args.push(ptr::null());
        CArgs {
            args,
            c_args,
        }
    }
    pub fn from(args: &[&str]) -> Self {
        let args = args.iter().map(|arg| CString::new(arg.as_bytes()).unwrap()).collect::<Vec<CString>>();
        // convert the strings to raw pointers
        let mut c_args = args.iter().map(|arg| arg.as_ptr()).collect::<Vec<*const c_char>>();
        c_args.push(ptr::null());
        CArgs {
            args,
            c_args,
        }
    }
    pub fn get(&self) -> (c_int, *mut *mut c_char) {
        (self.c_args.len() as c_int - 1, self.c_args.as_ptr() as _)
    }
}
