// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/buffer.h>
#include <gg/ipc/client.h>
#include <gg/ipc/mock.h>
#include <gg/ipc/packet_sequences.h>
#include <gg/map.h>
#include <gg/object.h>
#include <gg/process_wait.h>
#include <gg/sdk.h>
#include <gg/test.h>
#include <gg/types.h>
#include <sys/types.h>
#include <unistd.h>
#include <unity.h>
#include <stdint.h>

#define GG_MODULE "test_create_local_deployment"

GG_TEST_DEFINE(create_local_deployment_okay) {
    GgBuffer expected_id = GG_STR("abc123-deployment-id");

    // componentToConfiguration =
    //   { "aws.greengrass.NucleusLite":
    //       { "merge": { "networkProxy": { "proxy": { "url":
    //       "http://127.0.0.1:3129" } } } } }
    GgObject merge_value = gg_obj_map(GG_MAP(gg_kv(
        GG_STR("proxy"),
        gg_obj_map(GG_MAP(
            gg_kv(GG_STR("url"), gg_obj_buf(GG_STR("http://127.0.0.1:3129")))
        ))
    )));
    GgObject network_proxy
        = gg_obj_map(GG_MAP(gg_kv(GG_STR("networkProxy"), merge_value)));
    GgObject merge = gg_obj_map(GG_MAP(gg_kv(GG_STR("merge"), network_proxy)));
    GgMap component_to_config
        = GG_MAP(gg_kv(GG_STR("aws.greengrass.NucleusLite"), merge));

    uint8_t id_mem[32] = { 0 };

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GgCreateLocalDeploymentArgs args = { 0 };
        args.component_to_configuration = component_to_config;

        GgBuffer id_buf = GG_BUF(id_mem);
        GG_TEST_ASSERT_OK(ggipc_create_local_deployment(&args, &id_buf));

        GG_TEST_ASSERT_BUF_EQUAL_STR(expected_id, id_buf);

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

GG_TEST_DEFINE(create_local_deployment_no_output_okay) {
    // When the caller doesn't need the deployment id (deployment_id = NULL),
    // ggipc_create_local_deployment must still succeed and the mock server
    // must still receive the same request.
    GgMap component_to_config = GG_MAP(gg_kv(
        GG_STR("aws.greengrass.NucleusLite"),
        gg_obj_map(GG_MAP(gg_kv(GG_STR("merge"), gg_obj_map(GG_MAP()))))
    ));

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GgCreateLocalDeploymentArgs args = { 0 };
        args.component_to_configuration = component_to_config;

        GG_TEST_ASSERT_OK(ggipc_create_local_deployment(&args, NULL));

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

GG_TEST_DEFINE(create_local_deployment_rejected) {
    GgMap component_to_config = GG_MAP(gg_kv(
        GG_STR("aws.greengrass.NucleusLite"),
        gg_obj_map(GG_MAP(gg_kv(GG_STR("merge"), gg_obj_map(GG_MAP()))))
    ));

    uint8_t id_mem[32] = { 0 };

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GgCreateLocalDeploymentArgs args = { 0 };
        args.component_to_configuration = component_to_config;

        GgBuffer id_buf = GG_BUF(id_mem);
        GG_TEST_ASSERT_BAD(ggipc_create_local_deployment(&args, &id_buf));

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_create_local_deployment_error_sequence(
            1, gg_obj_map(component_to_config)
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

// Type-safety: GgMap field prevents non-map values at compile time.
// No runtime rejection test needed.

GG_TEST_DEFINE(create_local_deployment_with_versions_okay) {
    GgBuffer expected_id = GG_STR("deploy-versions-123");

    // rootComponentVersionsToAdd = { "com.example.MyComponent": "1.0.0" }
    GgMap versions_map = GG_MAP(
        gg_kv(GG_STR("com.example.MyComponent"), gg_obj_buf(GG_STR("1.0.0")))
    );

    uint8_t id_mem[64] = { 0 };

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GgCreateLocalDeploymentArgs args = { 0 };
        args.root_component_versions_to_add = versions_map;

        GgBuffer id_buf = GG_BUF(id_mem);
        GG_TEST_ASSERT_OK(ggipc_create_local_deployment(&args, &id_buf));

        GG_TEST_ASSERT_BUF_EQUAL_STR(expected_id, id_buf);

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_create_local_deployment_versions_accepted_sequence(
            1, gg_obj_map(versions_map), expected_id
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}
