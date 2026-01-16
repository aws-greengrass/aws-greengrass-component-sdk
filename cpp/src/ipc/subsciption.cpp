// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/ipc/subscription.hpp>

extern "C" {
#include <gg/ipc/client.h>
#include <gg/ipc/types.h>
}

namespace gg::ipc {

void Subscription::close() noexcept {
    // This check avoids locking a mutex when Subscription is
    // default-initialized
    if (holds_subscription()) {
        ggipc_close_subscription(release());
    }
}
}
