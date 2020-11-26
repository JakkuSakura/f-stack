#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

mod bindings;

#[cfg(test)]
mod tests {
    use crate::bindings::fstack_run;

    #[test]
    fn test_fstack() {
        unsafe {
            fstack_run(None);
        }
    }
}
