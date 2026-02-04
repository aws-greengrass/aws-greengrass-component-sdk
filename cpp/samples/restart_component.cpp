// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Restart a component

#include <gg/ipc/client.hpp>
#include <iostream>

int main() {
    auto &client = gg::ipc::Client::get();

    auto error = client.connect();
    if (error) {
        std::cerr << "Failed to establish IPC connection.\n";
        exit(-1);
    }

    std::string_view component_name = "com.example.HelloWorld";

    error = client.restart_component(component_name);
    if (error) {
        std::cerr << "Failed to restart component: " << component_name << "\n";
        exit(-1);
    }

    std::cout << "Successfully requested restart for component: "
              << component_name << "\n";
}
