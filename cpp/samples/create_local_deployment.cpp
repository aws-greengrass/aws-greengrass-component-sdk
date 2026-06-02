// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Create a local deployment that merges configuration into a component

#include <gg/ipc/client.hpp>
#include <cstddef>
#include <array>
#include <iostream>
#include <string_view>

int main() {
    auto &client = gg::ipc::Client::get();

    auto error = client.connect();
    if (error) {
        std::cerr << "Failed to establish IPC connection.\n";
        exit(-1);
    }

    // Build componentToConfiguration:
    // {"com.example.MyComponent": {"MERGE": {"endpoint":
    // "https://example.com"}}}
    gg::KV endpoint_kv("endpoint", gg::Object("https://example.com"));
    gg::Map merge_content(&endpoint_kv, 1);

    gg::KV merge_kv("MERGE", gg::Object(merge_content));
    gg::Map update(&merge_kv, 1);

    gg::KV comp_kv("com.example.MyComponent", gg::Object(update));
    gg::Map config(&comp_kv, 1);

    gg::ipc::CreateLocalDeploymentArgs args {};
    args.component_to_configuration = config;

    std::array<std::byte, 64> id_mem {};
    std::string_view deployment_id;

    error = client.create_local_deployment(args, id_mem, &deployment_id);
    if (error) {
        std::cerr << "Failed to create local deployment.\n";
        exit(-1);
    }

    std::cout << "Deployment created: " << deployment_id << "\n";
}
