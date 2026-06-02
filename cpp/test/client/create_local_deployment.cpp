// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/ipc/client.hpp>
#include <gg/map.hpp>
#include <gg/object.hpp>
#include <cstddef>
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

GG_TEST_DEFINE(cpp_create_local_deployment_okay) {
    GgBuffer expected_id = GG_STR("cpp-deploy-id-123");

    // { "aws.greengrass.NucleusLite": { "merge": { "url": "http://..." } } }
    gg::KV inner_kv("url", gg_obj_buf(GG_STR("http://127.0.0.1:3129")));
    gg::Map inner_map(&inner_kv, 1);

    gg::KV merge_kv("merge", gg_obj_map(inner_map));
    gg::Map merge_map(&merge_kv, 1);

    gg::KV comp_kv("aws.greengrass.NucleusLite", gg_obj_map(merge_map));
    gg::Map component_to_config(&comp_kv, 1);

    std::array<std::byte, 64> id_mem {};

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        auto &client = gg::ipc::Client::get();
        GG_TEST_ASSERT_OK(client.connect().value());

        gg::ipc::CreateLocalDeploymentArgs args {};
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
    gg::KV inner_kv("url", gg_obj_buf(GG_STR("http://127.0.0.1:3129")));
    gg::Map inner_map(&inner_kv, 1);

    gg::KV merge_kv("merge", gg_obj_map(inner_map));
    gg::Map merge_map(&merge_kv, 1);

    gg::KV comp_kv("aws.greengrass.NucleusLite", gg_obj_map(merge_map));
    gg::Map component_to_config(&comp_kv, 1);

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        auto &client = gg::ipc::Client::get();
        GG_TEST_ASSERT_OK(client.connect().value());

        gg::ipc::CreateLocalDeploymentArgs args {};
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
