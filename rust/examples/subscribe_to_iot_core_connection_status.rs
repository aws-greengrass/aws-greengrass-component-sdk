// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Subscribe to IoT Core MQTT connection status changes

use gg_sdk::Sdk;
use std::{thread, time::Duration};

fn main() {
    let sdk = Sdk::init();
    sdk.connect().expect("Failed to establish IPC connection");

    let callback = |connected: bool| {
        let status = if connected {
            "CONNECTED"
        } else {
            "DISCONNECTED"
        };
        println!("IoT Core connection status changed: {status}");
    };

    let _sub = sdk
        .subscribe_to_iot_core_connection_status(&callback)
        .expect("Failed to subscribe to IoT Core connection status");

    println!("Successfully subscribed to IoT Core connection status.");

    // Keep the main thread alive, or the process will exit.
    loop {
        thread::sleep(Duration::from_secs(10));
    }
}
