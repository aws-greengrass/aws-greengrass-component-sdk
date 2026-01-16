// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef GG_SDK_HPP
#define GG_SDK_HPP

extern "C" {
#include <gg/sdk.h>
}

namespace gg {
class Sdk {
private:
    Sdk() noexcept;

public:
    static Sdk &get() noexcept {
        static Sdk singleton {};
        return singleton;
    }
};

inline Sdk::Sdk() noexcept {
    gg_sdk_init();
}

}

#endif
