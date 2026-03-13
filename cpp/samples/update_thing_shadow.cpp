// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Update a thing shadow

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
    std::optional<std::string_view> shadow_name = "my-shadow";
    std::string_view payload = R"({"state":{"reported":{"temperature":25}}})";

    error = client.update_thing_shadow(
        thing_name, shadow_name, gg::Buffer { payload }
    );
    if (error) {
        std::cerr << "Failed to update shadow.\n";
        exit(-1);
    }

    std::cout << "Shadow updated successfully.\n";
}
