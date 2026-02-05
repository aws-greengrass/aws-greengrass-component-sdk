// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Subscribe to messages from AWS IoT Core

use gg_sdk::{Qos, Sdk};
use std::{thread, time::Duration};

fn main() {
    let sdk = Sdk::init();
    sdk.connect().expect("Failed to establish IPC connection");

    let topic = "my/topic";
    let qos = Qos::AtLeastOnce;

    let callback = |topic: &str, payload: &[u8]| {
        let message = String::from_utf8_lossy(payload);
        println!("Received new message on topic {topic}: {message}");
    };

    let _sub = sdk
        .subscribe_to_iot_core(topic, qos, &callback)
        .expect("Failed to subscribe to topic");

    println!("Successfully subscribed to topic: {topic}");

    // Keep the main thread alive, or the process will exit.
    loop {
        thread::sleep(Duration::from_secs(10));
    }
}
