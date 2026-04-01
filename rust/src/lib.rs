// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//! Lightweight AWS IoT Greengrass SDK for making IPC calls.

#![no_std]
#![warn(missing_docs, clippy::pedantic, clippy::cargo)]
#![allow(clippy::enum_glob_use)]

#[cfg(test)]
extern crate std;

mod buffer;
mod c {
    #![allow(non_upper_case_globals)]
    #![allow(non_camel_case_types)]
    #![allow(non_snake_case)]
    #![allow(dead_code)]
    #![allow(clippy::pedantic)]
    include!(concat!(env!("OUT_DIR"), "/c.rs"));
}
mod error;
mod ipc;
mod object;

pub use error::{Error, Result};
pub use ipc::{
    ComponentState, Qos, Sdk, SubscribeToTopicPayload, Subscription,
};
pub use object::{Kv, List, Map, Object, UnpackedObject};
