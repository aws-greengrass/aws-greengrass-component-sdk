// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Publish a message to AWS IoT Core

use gg_sdk::{Qos, Sdk};

fn main() {
    let sdk = Sdk::init();
    sdk.connect().expect("Failed to establish IPC connection");

    let message = b"Hello, World";
    let topic = "my/topic";
    let qos = Qos::AtLeastOnce;

    sdk.publish_to_iot_core(topic, message, qos)
        .expect("Failed to publish to topic");

    println!("Successfully published to topic: {topic}");
}
