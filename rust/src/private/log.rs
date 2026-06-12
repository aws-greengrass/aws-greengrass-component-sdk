// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

use crate::c;
use core::fmt;

include!(concat!(env!("OUT_DIR"), "/config.rs"));

/// GG log levels
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
#[repr(u32)]
pub enum LogLevel {
    /// Only useful for disabling logging
    None = c::GG_LOG_NONE,

    /// ERROR level logs
    Error = c::GG_LOG_ERROR,

    /// WARN level logs
    Warn = c::GG_LOG_WARN,

    /// INFO level logs
    Info = c::GG_LOG_INFO,

    /// DEBUG level logs
    Debug = c::GG_LOG_DEBUG,

    /// TRACE level logs
    Trace = c::GG_LOG_TRACE,
}

impl LogLevel {
    /// Attempts to convert a string to a log level.
    #[must_use]
    pub const fn from_str(s: &str) -> Option<Self> {
        match s.as_bytes() {
            b"GG_LOG_NONE" => Some(LogLevel::None),
            b"GG_LOG_ERROR" => Some(LogLevel::Error),
            b"GG_LOG_WARN" => Some(LogLevel::Warn),
            b"GG_LOG_INFO" => Some(LogLevel::Info),
            b"GG_LOG_DEBUG" => Some(LogLevel::Debug),
            b"GG_LOG_TRACE" => Some(LogLevel::Trace),
            _ => None,
        }
    }

    /// Returns true if the log level is enabled.
    #[must_use]
    pub const fn enabled(&self) -> bool {
        let Some(level) = LogLevel::from_str(LOG_LEVEL) else {
            return true;
        };

        *self as u32 <= level as u32
    }
}

/// Logs a structured message on the SDK's log stream.
pub fn log(
    level: LogLevel,
    file: &core::ffi::CStr,
    line: core::ffi::c_int,
    tag: &core::ffi::CStr,
    args: fmt::Arguments,
) {
    const BUF_LEN: core::ffi::c_int = 1024;
    let mut buf = [0u8; BUF_LEN as usize];

    let mut w = FmtBuf {
        buf: &mut buf,
        pos: 0,
    };
    let _ = fmt::write(&mut w, args);

    let len = core::ffi::c_int::try_from(w.pos).unwrap_or(BUF_LEN);

    // SAFETY: pointers passed to this function must be non-null.
    // Strings passed to `file`, `tag`, and `format` must be null-terminated.
    //
    // The format string must be satisfied by the variadic arguments according
    // to the specification of C `printf` family functions:
    //
    // If there are insufficient arguments for the format, the behavior is undefined.
    // If any of the arguments is not the correct type for its
    // corresponding conversion specification, the behavior is undefined.
    //
    // Justification:
    // Non-null pointer and null-termination preconditions for string parameters
    // are satisfied by CStr's invariants.
    //
    // A format string is chosen (c"%.*s") to always be satisfied
    // by passing a known-length buffer (c_int, *const c_char)
    unsafe {
        c::gg_log(
            level as u32,
            file.as_ptr(),
            line,
            tag.as_ptr(),
            c"%.*s".as_ptr(),
            len,
            w.buf.as_ptr().cast::<core::ffi::c_char>(),
        );
    }
}

struct FmtBuf<'a> {
    buf: &'a mut [u8],
    pos: usize,
}

impl fmt::Write for FmtBuf<'_> {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        let remaining = self.buf.len() - self.pos;
        let len = s.len().min(remaining);
        self.buf[self.pos..self.pos + len]
            .copy_from_slice(&s.as_bytes()[..len]);
        self.pos += len;
        Ok(())
    }
}

/// Logs at the given level. Use the level-specific macros.
#[macro_export]
macro_rules! gg_log {
    ($level:expr, $($arg:tt)+) => {
        if $level.enabled() {
            $crate::private::log(
                $level,
                core::ffi::CStr::from_bytes_with_nul(concat!(file!(), "\0").as_bytes()).unwrap_or(c""),
                core::ffi::c_int::try_from(line!()).unwrap_or(0),
                core::ffi::CStr::from_bytes_with_nul(concat!(module_path!(), "\0").as_bytes()).unwrap_or(c""),
                format_args!($($arg)+),
            )
        }
    };
}

/// Log at ERROR level.
#[macro_export]
macro_rules! log_error {
    ($($arg:tt)+) => { $crate::gg_log!($crate::private::LogLevel::Error, $($arg)+) };
}

/// Log at WARN level.
#[macro_export]
macro_rules! log_warn {
    ($($arg:tt)+) => { $crate::gg_log!($crate::private::LogLevel::Warn, $($arg)+) };
}

/// Log at INFO level.
#[macro_export]
macro_rules! log_info {
    ($($arg:tt)+) => { $crate::gg_log!($crate::private::LogLevel::Info, $($arg)+) };
}

/// Log at DEBUG level.
#[macro_export]
macro_rules! log_debug {
    ($($arg:tt)+) => { $crate::gg_log!($crate::private::LogLevel::Debug, $($arg)+) };
}

/// Log at TRACE level.
#[macro_export]
macro_rules! log_trace {
    ($($arg:tt)+) => { $crate::gg_log!($crate::private::LogLevel::Trace, $($arg)+) };
}
