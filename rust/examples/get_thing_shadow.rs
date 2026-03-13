// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Get a thing shadow

use std::mem::MaybeUninit;

use gg_sdk::Sdk;

fn main() {
    let sdk = Sdk::init();
    sdk.connect().expect("Failed to establish IPC connection");

    let thing_name = std::env::var("AWS_IOT_THING_NAME").unwrap();
    let shadow_name = "my-shadow";

    let mut buf = [MaybeUninit::uninit(); 8192];
    let payload = sdk
        .get_thing_shadow(&thing_name, Some(shadow_name), &mut buf)
        .expect("Failed to get shadow");

    println!(
        "Shadow document: {}",
        core::str::from_utf8(payload).unwrap()
    );
}
