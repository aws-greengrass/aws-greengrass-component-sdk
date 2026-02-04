// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Restart a component

use gg_sdk::Sdk;

fn main() {
    let sdk = Sdk::init();
    sdk.connect().expect("Failed to establish IPC connection");

    let component_name = "com.example.HelloWorld";

    sdk.restart_component(component_name)
        .expect("Failed to restart component");

    println!("Successfully requested restart for component: {component_name}");
}
