// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Subscribe to IoT Core MQTT connection status changes

#include <gg/ipc/client.hpp>
#include <unistd.h>
#include <iostream>

class ConnectionStatusHandler : public gg::ipc::ConnectionStatusCallback {
    void operator()(bool connected, gg::ipc::Subscription &handle) override {
        (void) handle;
        std::cout << "IoT Core connection status changed: "
                  << (connected ? "CONNECTED" : "DISCONNECTED") << "\n";
    }
};

int main() {
    auto &client = gg::ipc::Client::get();

    auto error = client.connect();
    if (error) {
        std::cerr << "Failed to establish IPC connection.\n";
        exit(-1);
    }

    static ConnectionStatusHandler handler;
    error = client.subscribe_to_iot_core_connection_status(handler);
    if (error) {
        std::cerr << "Failed to subscribe to IoT Core connection status.\n";
        exit(-1);
    }

    std::cout << "Successfully subscribed to IoT Core connection status.\n";

    // Keep the main thread alive, or the process will exit.
    while (1) {
        sleep(10);
    }
}
