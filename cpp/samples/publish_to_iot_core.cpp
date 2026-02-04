// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Publish a message to AWS IoT Core

#include <gg/ipc/client.hpp>
#include <iostream>

int main() {
    auto &client = gg::ipc::Client::get();

    auto error = client.connect();
    if (error) {
        std::cerr << "Failed to establish IPC connection.\n";
        exit(-1);
    }

    std::string_view message = "Hello, World";
    std::string_view topic = "my/topic";
    uint8_t qos = 1;

    error = client.publish_to_iot_core(topic, message, qos);
    if (error) {
        std::cerr << "Failed to publish to topic: " << topic << "\n";
        exit(-1);
    }

    std::cout << "Successfully published to topic: " << topic << "\n";
}
