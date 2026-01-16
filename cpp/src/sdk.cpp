// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/sdk.hpp>

extern "C" {
#include <gg/sdk.h>
}

namespace gg {
Sdk::Sdk() noexcept {
    gg_sdk_init();
}
}
