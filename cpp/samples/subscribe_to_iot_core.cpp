// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Subscribe to messages from AWS IoT Core

#include <gg/ipc/client.hpp>
#include <unistd.h>
#include <iostream>

class ResponseHandler : public gg::ipc::IotTopicCallback {
    void operator()(
        std::string_view topic,
        gg::Buffer payload,
        gg::ipc::Subscription &handle
    ) override {
        (void) handle;
        std::cout << "Received new message on topic " << topic << ": "
                  << payload << "\n";
    }
};

int main() {
    auto &client = gg::ipc::Client::get();

    auto error = client.connect();
    if (error) {
        std::cerr << "Failed to establish IPC connection.\n";
        exit(-1);
    }

    std::string_view topic = "my/topic";
    uint8_t qos = 1;

    static ResponseHandler handler;
    error = client.subscribe_to_iot_core(topic, qos, handler);
    if (error) {
        std::cerr << "Failed to subscribe to topic: " << topic << "\n";
        exit(-1);
    }

    std::cout << "Successfully subscribed to topic: " << topic << "\n";

    // Keep the main thread alive, or the process will exit.
    while (1) {
        sleep(10);
    }
}
