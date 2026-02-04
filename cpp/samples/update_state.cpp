// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Update component state

#include <gg/ipc/client.hpp>
#include <iostream>

int main() {
    auto &client = gg::ipc::Client::get();

    auto error = client.connect();
    if (error) {
        std::cerr << "Failed to establish IPC connection.\n";
        exit(-1);
    }

    // Update component state to RUNNING
    error = client.update_component_state(GG_COMPONENT_STATE_RUNNING);
    if (error) {
        std::cerr << "Failed to update component state.\n";
        exit(-1);
    }

    std::cout << "Successfully updated component state to RUNNING.\n";
}
