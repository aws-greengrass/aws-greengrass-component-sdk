// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/ipc/client.hpp>
#include <gg/object.hpp>
#include <cstddef>
#include <cstdint>
#include <array>
#include <span>
#include <string_view>
#include <system_error>

extern "C" {
#include <gg/buffer.h>
#include <gg/ipc/mock.h>
#include <gg/ipc/packet_sequences.h>
#include <gg/map.h>
#include <gg/object.h>
#include <gg/test.h>
#include <sys/types.h>
#include <unistd.h>
#include <unity.h>

GgError gg_process_wait(pid_t pid) noexcept;
}

namespace {

// Build { "aws.greengrass.NucleusLite": { "merge": { merge_key: merge_val } } }
// programmatically. C compound-literal macros (GG_MAP/gg_kv) don't compile
// cleanly in C++, so we construct the nested structure with direct calls.
GgMap build_nucleus_merge_map(
    GgBuffer merge_key,
    GgBuffer merge_val,
    GgKV *kv_storage,
    GgObject *obj_storage
) {
    // Innermost: { merge_key: merge_val }
    kv_storage[0] = gg_kv(merge_key, gg_obj_buf(merge_val));
    obj_storage[0] = gg_obj_map(GgMap { .pairs = &kv_storage[0], .len = 1 });

    // Middle: { "merge": <inner> }
    kv_storage[1] = gg_kv(GG_STR("merge"), obj_storage[0]);
    obj_storage[1] = gg_obj_map(GgMap { .pairs = &kv_storage[1], .len = 1 });

    // Outer: { "aws.greengrass.NucleusLite": <middle> }
    kv_storage[2] = gg_kv(GG_STR("aws.greengrass.NucleusLite"), obj_storage[1]);
    return GgMap { .pairs = &kv_storage[2], .len = 1 };
}

} // namespace

GG_TEST_DEFINE(cpp_create_local_deployment_okay) {
    GgBuffer expected_id = GG_STR("cpp-deploy-id-123");

    GgKV kv_storage[3];
    GgObject obj_storage[2];
    GgMap component_to_config = build_nucleus_merge_map(
        GG_STR("url"), GG_STR("http://127.0.0.1:3129"), kv_storage, obj_storage
    );

    std::array<std::byte, 64> id_mem {};

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        auto &client = gg::ipc::Client::get();
        GG_TEST_ASSERT_OK(client.connect().value());

        GgCreateLocalDeploymentArgs args = {};
        args.component_to_configuration = component_to_config;

        std::string_view deployment_id;
        GG_TEST_ASSERT_OK(client
                              .create_local_deployment(
                                  args, std::span(id_mem), &deployment_id
                              )
                              .value());

        TEST_ASSERT_EQUAL_STRING_LEN(
            "cpp-deploy-id-123", deployment_id.data(), deployment_id.size()
        );

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_create_local_deployment_accepted_sequence(
            1, gg_obj_map(component_to_config), expected_id
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(cpp_create_local_deployment_no_output) {
    GgKV kv_storage[3];
    GgObject obj_storage[2];
    GgMap component_to_config = build_nucleus_merge_map(
        GG_STR("url"), GG_STR("http://127.0.0.1:3129"), kv_storage, obj_storage
    );

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        auto &client = gg::ipc::Client::get();
        GG_TEST_ASSERT_OK(client.connect().value());

        GgCreateLocalDeploymentArgs args = {};
        args.component_to_configuration = component_to_config;

        GG_TEST_ASSERT_OK(client.create_local_deployment(args).value());

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_create_local_deployment_accepted_sequence(
            1, gg_obj_map(component_to_config), GG_STR("any-id")
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}
