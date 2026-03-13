// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/buffer.hpp>
#include <gg/error.hpp>
#include <gg/ipc/client.hpp>
#include <optional>
#include <string_view>
#include <system_error>

extern "C" {
#include <gg/ipc/client.h>
}

namespace gg::ipc {

std::error_code Client::delete_thing_shadow(
    std::string_view thing_name, std::optional<std::string_view> shadow_name
) noexcept {
    Buffer shadow_buf { shadow_name.value_or(std::string_view {}) };
    return ggipc_delete_thing_shadow(
        Buffer { thing_name }, shadow_name.has_value() ? &shadow_buf : nullptr
    );
}

}
