use std::env;
use std::fs::File;
use std::io;
use std::path::Path;
use std::process;
use std::str;

use std::os::unix::prelude::AsRawFd;
use std::slice;

use libc::{mmap, munmap, PROT_READ, MAP_PRIVATE, MAP_FAILED};

fn usage(program: &str) {
    eprintln!("Usage: {} /path/to/dictionary.txt LENGTH LETTERS", program);
    process::exit(1);
}

fn mmap_file(path: &Path) -> io::Result<&'static [u8]> {
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
        Ok(slice)
    }
}

fn munmap_file(data: &[u8]) {
    unsafe {
        let ptr = data.as_ptr() as *mut libc::c_void;
        let len = data.len();
        munmap(ptr, len);
    }
}

fn is_word_match(word: &str, length: usize, letters: &str) -> bool {
    if word.chars().count() != length {
        return false;
    }
    word.chars().all(|c| letters.contains(c))
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() != 4 {
        usage(&args[0]);
    }

    let dict_path = Path::new(&args[1]);
    let word_len: usize = match args[2].parse() {
        Ok(n) => n,
        Err(_) => {
            eprintln!("Invalid length: {}", args[2]);
            process::exit(1);
        }
    };
    let letters = &args[3];

    let mmap_data = match mmap_file(dict_path) {
        Ok(data) => data,
        Err(e) => {
            eprintln!("Failed to mmap file: {}", e);
            process::exit(1);
        }
    };

    for line in mmap_data.split(|&b| b == b'\n') {
        if let Ok(word) = std::str::from_utf8(line) {
            if is_word_match(word, word_len, letters) {
                println!("{}", word);
            }
        }
    }

    munmap_file(mmap_data);
}

