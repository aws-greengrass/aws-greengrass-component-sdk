// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Create a local deployment that merges configuration into a component

use core::mem::MaybeUninit;
use gg_sdk::{CreateLocalDeploymentArgs, Kv, Object, Sdk};

fn main() {
    let sdk = Sdk::init();
    sdk.connect().expect("Failed to establish IPC connection");

    // Build componentToConfiguration:
    // { "com.example.MyComponent": { "MERGE": { "endpoint": "https://example.com" } } }
    let merge_val = [Kv::new("endpoint", Object::buf("https://example.com"))];
    let update = [Kv::new("MERGE", Object::map(&merge_val))];
    let config = [Kv::new("com.example.MyComponent", Object::map(&update))];

    let args = CreateLocalDeploymentArgs {
        component_to_configuration: &config,
        ..Default::default()
    };

    let mut id_mem = [MaybeUninit::uninit(); 64];
    let deployment_id = sdk
        .create_local_deployment(&args, &mut id_mem)
        .expect("Failed to create local deployment");

    println!("Deployment created: {deployment_id}");
}
