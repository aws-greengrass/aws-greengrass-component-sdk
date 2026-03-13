// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: List named shadows for a thing

use gg_sdk::Sdk;

fn main() {
    let sdk = Sdk::init();
    sdk.connect().expect("Failed to establish IPC connection");

    let thing_name = std::env::var("AWS_IOT_THING_NAME").unwrap();

    println!("Named shadows:");

    sdk.list_named_shadows_for_thing(&thing_name, &mut |name| {
        println!("  - {name}");
    })
    .expect("Failed to list named shadows");
}
