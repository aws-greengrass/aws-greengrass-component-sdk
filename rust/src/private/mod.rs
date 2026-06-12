// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//! Unstable interfaces for internal functions.

mod log;

pub use log::{LogLevel, log};

pub use crate::log_debug as debug;
pub use crate::log_error as error;
pub use crate::log_info as info;
pub use crate::log_trace as trace;
pub use crate::log_warn as warn;
