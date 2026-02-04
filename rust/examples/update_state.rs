// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Update component state

use gg_sdk::{ComponentState, Sdk};

fn main() {
    let sdk = Sdk::init();
    sdk.connect().expect("Failed to establish IPC connection");

    // Update component state to RUNNING
    sdk.update_state(ComponentState::Running)
        .expect("Failed to update component state");

    println!("Successfully updated component state to RUNNING.");
}
