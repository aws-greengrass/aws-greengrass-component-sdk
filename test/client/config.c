#include <gg/arena.h>
#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/ipc/mock.h>
#include <gg/ipc/packet_sequences.h>
#include <gg/json_encode.h>
#include <gg/log.h>
#include <gg/map.h>
#include <gg/object.h>
#include <gg/process_wait.h>
#include <gg/sdk.h>
#include <gg/test.h>
#include <gg/types.h>
#include <gg/vector.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <unity.h>
#include <stdatomic.h>
#include <stdint.h>

#define GG_MODULE "test_config"

GG_TEST_DEFINE(get_configuration_str_okay) {
    GgBuffer expected = GG_STR("Hello config!");

    uint8_t mem[16] = { 0 };

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GgBuffer actual = GG_BUF(mem);
        GG_TEST_ASSERT_OK(
            ggipc_get_config_str(GG_BUF_LIST(GG_STR("key")), NULL, &actual)
        );

        GG_TEST_ASSERT_BUF_EQUAL_STR(expected, actual);

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_accept_client(1));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_connect_accepted_sequence(gg_test_get_auth_token()), 5
    ));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_config_get_object_sequence(
            1, GG_BUF_LIST(GG_STR("key")), NULL, gg_obj_buf(expected)
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(1));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(get_configuration_str_wrong_type) {
    GgObject bad_type = gg_obj_i64(42);

    uint8_t mem[16] = { 0 };

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GgBuffer actual = GG_BUF(mem);
        GG_TEST_ASSERT_BAD(
            ggipc_get_config_str(GG_BUF_LIST(GG_STR("key")), NULL, &actual)
        );

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_accept_client(1));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_connect_accepted_sequence(gg_test_get_auth_token()), 5
    ));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_config_get_object_sequence(
            1, GG_BUF_LIST(GG_STR("key")), NULL, bad_type
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(1));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(get_configuration_str_cant_allocate) {
    GgBuffer expected = GG_STR("Hello config!");

    uint8_t mem[1] = { 0 };

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GgBuffer actual = GG_BUF(mem);
        GG_TEST_ASSERT_BAD(
            ggipc_get_config_str(GG_BUF_LIST(GG_STR("key")), NULL, &actual)
        );

        GG_TEST_ASSERT_BUF_EQUAL_STR(GG_STR(""), actual);

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_accept_client(1));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_connect_accepted_sequence(gg_test_get_auth_token()), 5
    ));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_config_get_object_sequence(
            1, GG_BUF_LIST(GG_STR("key")), NULL, gg_obj_buf(expected)
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(1));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(get_configuration_map_okay) {
    GgObject expected_object
        = gg_obj_map(GG_MAP(gg_kv(GG_STR("key"), gg_obj_buf(GG_STR("value")))));

    uint8_t mem[64];
    GgArena config_arena = gg_arena_init(GG_BUF(mem));

    GgBufList key_path = GG_BUF_LIST(GG_STR("key"), GG_STR("path"));

    GgBuffer component_name = GG_STR("TestComponent");

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GgObject actual_object = GG_OBJ_NULL;
        GG_TEST_ASSERT_OK(ggipc_get_config(
            key_path, &component_name, &config_arena, &actual_object
        ));

        GG_TEST_ASSERT_OBJ_EQUAL(
            gg_obj_map(GG_MAP(gg_kv(GG_STR("path"), expected_object))),
            actual_object
        );

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_accept_client(1));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_connect_accepted_sequence(gg_test_get_auth_token()), 5
    ));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_config_get_object_sequence(
            1, key_path, &component_name, expected_object
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(1));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(get_configuration_not_found) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        TEST_ASSERT_EQUAL(
            GG_ERR_NOENTRY, ggipc_get_config(GG_BUF_LIST(), NULL, NULL, NULL)
        );

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_accept_client(1));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_connect_accepted_sequence(gg_test_get_auth_token()), 5
    ));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_config_get_not_found_sequence(1), 5
    ));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(1));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(get_configuration_key_path_too_long) {
    GgBufList key_path = GG_BUF_LIST(
        GG_STR("key"),
        GG_STR("path"),
        GG_STR("way"),
        GG_STR("too"),
        GG_STR("long"),
        GG_STR("and"),
        GG_STR("nested"),
        GG_STR("the"),
        GG_STR("client"),
        GG_STR("rejects"),
        GG_STR("it"),
        GG_STR("without"),
        GG_STR("sending"),
        GG_STR("a"),
        GG_STR("request"),
        GG_STR("packet")
    );

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GG_TEST_ASSERT_BAD(ggipc_get_config(key_path, NULL, NULL, NULL));

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_accept_client(1));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_connect_accepted_sequence(gg_test_get_auth_token()), 5
    ));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(1));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(get_configuration_obj_cant_allocate) {
    GgObject expected_object
        = gg_obj_map(GG_MAP(gg_kv(GG_STR("key"), gg_obj_buf(GG_STR("value")))));

    uint8_t mem[1];
    GgArena config_arena = gg_arena_init(GG_BUF(mem));

    GgBufList key_path = GG_BUF_LIST(GG_STR("key"), GG_STR("path"));

    GgBuffer component_name = GG_STR("TestComponent");

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GgObject actual_object = GG_OBJ_NULL;
        GG_TEST_ASSERT_BAD(ggipc_get_config(
            key_path, &component_name, &config_arena, &actual_object
        ));

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_accept_client(1));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_connect_accepted_sequence(gg_test_get_auth_token()), 5
    ));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_config_get_object_sequence(
            1, key_path, &component_name, expected_object
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(1));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(get_configuration_bad_server_response) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GG_TEST_ASSERT_BAD(ggipc_get_config(GG_BUF_LIST(), NULL, NULL, NULL));

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_accept_client(1));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_connect_accepted_sequence(gg_test_get_auth_token()), 5
    ));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_config_bad_server_response_sequence(1), 5
    ));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(1));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}
