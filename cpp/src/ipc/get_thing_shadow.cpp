// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/buffer.hpp>
#include <gg/error.hpp>
#include <gg/ipc/client.hpp>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <system_error>

extern "C" {
#include <gg/ipc/client.h>
}

namespace gg::ipc {

std::error_code Client::get_thing_shadow(
    std::string_view thing_name,
    std::optional<std::string_view> shadow_name,
    std::span<std::byte> payload_mem,
    std::string_view &payload
) noexcept {
    Buffer shadow_buf { shadow_name.value_or(std::string_view {}) };
    GgBuffer result = { .data = reinterpret_cast<uint8_t *>(payload_mem.data()),
                        .len = payload_mem.size() };
    GgError ret = ggipc_get_thing_shadow(
        Buffer { thing_name },
        shadow_name.has_value() ? &shadow_buf : nullptr,
        &result
    );
    if (ret != GgError::GG_ERR_OK) {
        return ret;
    }
    payload = std::string_view { reinterpret_cast<char *>(result.data),
                                 result.len };
    return {};
}

}
