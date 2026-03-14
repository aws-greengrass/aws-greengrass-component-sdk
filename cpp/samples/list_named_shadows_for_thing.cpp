// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: List named shadows for a thing

#include <gg/ipc/client.hpp>
#include <cstdlib>
#include <iostream>

int main() {
    auto &client = gg::ipc::Client::get();

    auto error = client.connect();
    if (error) {
        std::cerr << "Failed to establish IPC connection.\n";
        exit(-1);
    }

    std::string_view thing_name = std::getenv("AWS_IOT_THING_NAME");

    std::cout << "Named shadows:\n";

    error = client.list_named_shadows_for_thing(
        thing_name, +[](void *, std::string_view name) {
            std::cout << "  - " << name << "\n";
        }
    );
    if (error) {
        std::cerr << "Failed to list named shadows.\n";
        exit(-1);
    }
}
