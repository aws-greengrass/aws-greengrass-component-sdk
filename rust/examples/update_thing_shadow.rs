// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Update a thing shadow

use gg_sdk::Sdk;

fn main() {
    let sdk = Sdk::init();
    sdk.connect().expect("Failed to establish IPC connection");

    let thing_name = std::env::var("AWS_IOT_THING_NAME").unwrap();
    let shadow_name = "my-shadow";
    let payload = br#"{"state":{"reported":{"temperature":25}}}"#;

    sdk.update_thing_shadow(&thing_name, Some(shadow_name), payload, None)
        .expect("Failed to update shadow");

    println!("Shadow updated successfully.");
}
