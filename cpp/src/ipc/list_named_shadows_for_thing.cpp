// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/buffer.hpp>
#include <gg/error.hpp>
#include <gg/ipc/client.hpp>
#include <string_view>
#include <system_error>

extern "C" {
#include <gg/ipc/client.h>
}

namespace gg::ipc {

extern "C" {
namespace {

    struct ListCtx {
        void (*callback)(void *ctx, std::string_view shadow_name);
        void *ctx;
    };

    void list_trampoline(void *raw_ctx, GgBuffer shadow_name) {
        auto *list_ctx = static_cast<ListCtx *>(raw_ctx);
        list_ctx->callback(
            list_ctx->ctx,
            std::string_view { reinterpret_cast<char *>(shadow_name.data),
                               shadow_name.len }
        );
    }

}
}

std::error_code Client::list_named_shadows_for_thing(
    std::string_view thing_name,
    void (*callback)(void *ctx, std::string_view shadow_name),
    void *ctx
) noexcept {
    ListCtx list_ctx { callback, ctx };
    return ggipc_list_named_shadows_for_thing(
        Buffer { thing_name }, list_trampoline, &list_ctx
    );
}

}
