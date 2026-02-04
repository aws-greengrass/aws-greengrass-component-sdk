// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Update a configuration value

#include <gg/ipc/client.hpp>
#include <iostream>

int main() {
    auto &client = gg::ipc::Client::get();

    auto error = client.connect();
    if (error) {
        std::cerr << "Failed to establish IPC connection.\n";
        exit(-1);
    }

    // Update configuration value at key path ["mqtt", "port"] to 443
    std::array key_path = { gg::Buffer { "mqtt" }, gg::Buffer { "port" } };

    error = client.update_config(key_path, 443);
    if (error) {
        std::cerr << "Failed to update configuration.\n";
        exit(-1);
    }

    std::cout << "Successfully updated configuration.\n";
}
