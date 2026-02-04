// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// Example: Subscribe to local publish/subscribe messages

#include <gg/ipc/client.hpp>
#include <gg/object.hpp>
#include <unistd.h>
#include <cassert>
#include <iostream>

class ResponseHandler : public gg::ipc::LocalTopicCallback {
    void operator()(
        std::string_view topic,
        gg::Object payload,
        gg::ipc::Subscription &handle
    ) override {
        (void) handle;
        if (payload.index() == GG_TYPE_BUF) {
            std::cout << "Received new message on topic " << topic << ": "
                      << get<gg::Buffer>(payload) << "\n";
        } else {
            assert(payload.index() == GG_TYPE_MAP);
            std::cout << "Received new message on topic " << topic
                      << ": (JSON message)\n";
        }
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

    static ResponseHandler handler;
    error = client.subscribe_to_topic(topic, handler);
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
