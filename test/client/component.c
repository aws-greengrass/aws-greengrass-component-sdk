// aws-greengrass-lite - AWS IoT Greengrass runtime for constrained devices
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/ipc/client.h>
#include <gg/ipc/mock.h>
#include <gg/ipc/packet_sequences.h>
#include <gg/ipc/types.h>
#include <gg/process_wait.h>
#include <gg/sdk.h>
#include <gg/test.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <unity.h>
#include <stdlib.h>

GG_TEST_DEFINE(update_state_okay) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GG_TEST_ASSERT_OK(ggipc_update_state(GG_COMPONENT_STATE_RUNNING));

        GG_TEST_ASSERT_OK(ggipc_update_state(GG_COMPONENT_STATE_ERRORED));

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_accept_client_handshake(5));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_update_state_accepted_sequence(1, GG_STR("RUNNING")), 5
    ));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_update_state_accepted_sequence(2, GG_STR("ERRORED")), 5
    ));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(5));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(update_state_rejected) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GG_TEST_ASSERT_BAD(ggipc_update_state(GG_COMPONENT_STATE_RUNNING));

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_update_state_error_sequence(1, GG_STR("RUNNING")), 5
    ));
}

GG_TEST_DEFINE(update_state_invalid_enum) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GG_TEST_ASSERT_BAD(ggipc_update_state((GgComponentState) INT_MAX));

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        (GgipcPacketSequence) { .packets = {}, .len = 0 }, 5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(restart_component_okay) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GG_TEST_ASSERT_OK(ggipc_restart_component(GG_STR("MyComponent")));

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_restart_component_accepted_sequence(
            1, GG_STR("MyComponent"), GG_STR("Restarted")
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(restart_component_failed) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GG_TEST_ASSERT_BAD(ggipc_restart_component(GG_STR("MyComponent")));

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_restart_component_accepted_sequence(
            1, GG_STR("MyComponent"), GG_STR("FAILED")
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(restart_component_rejected) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GG_TEST_ASSERT_BAD(ggipc_restart_component(GG_STR("MyComponent")));

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_restart_component_error_sequence(1, GG_STR("MyComponent")), 5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}
