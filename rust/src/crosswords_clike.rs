use std::env;
use std::fs::File;
use std::io;
use std::os::unix::io::AsRawFd;
use std::path::Path;
use std::process;
use std::slice;
use std::str;

use libc::{mmap, munmap, PROT_READ, MAP_PRIVATE, MAP_FAILED};

fn usage(prog: &str) {
    eprintln!("Usage: {} /path/to/dictionary.txt LENGTH LETTERS", prog);
    process::exit(1);
}

fn mmap_file(path: &Path) -> io::Result<(&'static [u8], usize)> {
    let file = File::open(path)?;
    let len = file.metadata()?.len() as usize;

    unsafe {
        let addr = mmap(
            std::ptr::null_mut(),
            len,
            PROT_READ,
            MAP_PRIVATE,
            file.as_raw_fd(),
            0,
        );
        if addr == MAP_FAILED {
            return Err(io::Error::last_os_error());
        }
        let slice = slice::from_raw_parts(addr as *const u8, len);
        Ok((slice, len))
    }
}

fn munmap_file(data: &[u8]) {
    unsafe {
        let ptr = data.as_ptr() as *mut libc::c_void;
        let len = data.len();
        munmap(ptr, len);
    }
}

fn is_word_valid(word: &str, target_len: usize, letters: &str) -> bool {
    let mut count = 0;
    for ch in word.chars() {
        if !letters.contains(ch) {
            return false;
        }
        count += 1;
    }
    count == target_len
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() != 4 {
        usage(&args[0]);
    }

    let path = Path::new(&args[1]);
    let length: usize = match args[2].parse() {
        Ok(n) => n,
        Err(_) => {
            eprintln!("Invalid word length '{}'", args[2]);
            process::exit(1);
        }
    };
    let letters = &args[3];

    let (data, size) = match mmap_file(path) {
        Ok(pair) => pair,
        Err(e) => {
            eprintln!("Error mapping file: {}", e);
            process::exit(1);
        }
    };

    let mut start = 0;
    for i in 0..size {
        if data[i] == b'\n' {
            if i > start {
                let line = &data[start..i];
                if let Ok(word) = str::from_utf8(line) {
                    if is_word_valid(word, length, letters) {
                        println!("{}", word);
                    }
                }
            }
            start = i + 1;
        }
    }

    munmap_file(data);
}

