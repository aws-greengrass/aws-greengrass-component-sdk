#include <gg/ipc/client.h>
#include <gg/ipc/mock.h>
#include <gg/ipc/packet_sequences.h>
#include <gg/process_wait.h>
#include <gg/sdk.h>
#include <gg/test.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <unity.h>
#include <stdint.h>

#define THING_NAME GG_STR("MyThing")
#define SHADOW_NAME GG_STR("myShadow")
#define PAYLOAD GG_STR("hello")
#define PAYLOAD_B64 GG_STR("aGVsbG8=") // spell:disable-line
#define TIMESTAMP 1773436831.0

GG_TEST_DEFINE(get_thing_shadow_okay) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        uint8_t buf[64];
        GgBuffer payload = { .data = buf, .len = sizeof(buf) };
        GgBuffer shadow_name = SHADOW_NAME;
        GG_TEST_ASSERT_OK(
            ggipc_get_thing_shadow(THING_NAME, &shadow_name, &payload)
        );
        GG_TEST_ASSERT_BUF_EQUAL(PAYLOAD, payload);

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_shadow_get_accepted_sequence(
            1, THING_NAME, SHADOW_NAME, PAYLOAD_B64
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(update_thing_shadow_okay) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GgBuffer shadow_name = SHADOW_NAME;
        GG_TEST_ASSERT_OK(
            ggipc_update_thing_shadow(THING_NAME, &shadow_name, PAYLOAD, NULL)
        );

        uint8_t resp_buf[64];
        GgBuffer response = { .data = resp_buf, .len = sizeof(resp_buf) };
        GG_TEST_ASSERT_OK(ggipc_update_thing_shadow(
            THING_NAME, &shadow_name, PAYLOAD, &response
        ));
        GG_TEST_ASSERT_BUF_EQUAL(PAYLOAD, response);

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_accept_client_handshake(5));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_shadow_update_accepted_sequence(
            1, THING_NAME, SHADOW_NAME, PAYLOAD_B64, PAYLOAD_B64
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_shadow_update_accepted_sequence(
            2, THING_NAME, SHADOW_NAME, PAYLOAD_B64, PAYLOAD_B64
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(5));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(delete_thing_shadow_okay) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GgBuffer shadow_name = SHADOW_NAME;
        GG_TEST_ASSERT_OK(ggipc_delete_thing_shadow(THING_NAME, &shadow_name));

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_shadow_delete_accepted_sequence(
            1, THING_NAME, SHADOW_NAME, GG_STR("")
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

static uint8_t listed_shadow_storage[4][64];
static GgBuffer listed_shadows[4];
static size_t listed_count;

static void list_callback(void *ctx, GgBuffer shadow_name) {
    (void) ctx;
    if (listed_count < sizeof(listed_shadows) / sizeof(listed_shadows[0])) {
        memcpy(
            listed_shadow_storage[listed_count],
            shadow_name.data,
            shadow_name.len
        );
        listed_shadows[listed_count]
            = (GgBuffer) { .data = listed_shadow_storage[listed_count],
                           .len = shadow_name.len };
        listed_count++;
    }
}

GG_TEST_DEFINE(list_named_shadows_okay) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        listed_count = 0;
        GG_TEST_ASSERT_OK(
            ggipc_list_named_shadows_for_thing(THING_NAME, list_callback, NULL)
        );
        TEST_ASSERT_EQUAL_UINT(1, listed_count);
        GG_TEST_ASSERT_BUF_EQUAL_STR(SHADOW_NAME, listed_shadows[0]);

        TEST_PASS();
    }

    static GgObject results_items[1];
    results_items[0] = gg_obj_buf(SHADOW_NAME);
    GgList results = { .items = results_items, .len = 1 };

    GG_TEST_ASSERT_OK(gg_test_accept_client_handshake(5));

    // First page: returns one shadow, no nextToken → no pagination
    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_shadow_list_accepted_sequence(
            1, THING_NAME, NULL, results, TIMESTAMP, NULL
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(5));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(get_thing_shadow_rejected) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        uint8_t buf[64];
        GgBuffer payload = { .data = buf, .len = sizeof(buf) };
        GgBuffer shadow_name = SHADOW_NAME;
        GG_TEST_ASSERT_BAD(
            ggipc_get_thing_shadow(THING_NAME, &shadow_name, &payload)
        );

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_shadow_get_error_sequence(1, THING_NAME, SHADOW_NAME), 5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(update_thing_shadow_rejected) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GgBuffer shadow_name = SHADOW_NAME;
        GG_TEST_ASSERT_BAD(
            ggipc_update_thing_shadow(THING_NAME, &shadow_name, PAYLOAD, NULL)
        );

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_shadow_update_error_sequence(
            1, THING_NAME, SHADOW_NAME, PAYLOAD_B64
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(delete_thing_shadow_rejected) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        GgBuffer shadow_name = SHADOW_NAME;
        GG_TEST_ASSERT_BAD(ggipc_delete_thing_shadow(THING_NAME, &shadow_name));

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_shadow_delete_error_sequence(1, THING_NAME, SHADOW_NAME), 5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(list_named_shadows_rejected) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        listed_count = 0;
        GG_TEST_ASSERT_BAD(
            ggipc_list_named_shadows_for_thing(THING_NAME, list_callback, NULL)
        );
        TEST_ASSERT_EQUAL_UINT(0, listed_count);

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_shadow_list_error_sequence(1, THING_NAME), 5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(list_named_shadows_paginated_okay) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());

        listed_count = 0;
        GG_TEST_ASSERT_OK(
            ggipc_list_named_shadows_for_thing(THING_NAME, list_callback, NULL)
        );
        TEST_ASSERT_EQUAL_UINT(2, listed_count);
        GG_TEST_ASSERT_BUF_EQUAL_STR(GG_STR("shadow1"), listed_shadows[0]);
        GG_TEST_ASSERT_BUF_EQUAL_STR(GG_STR("shadow2"), listed_shadows[1]);

        TEST_PASS();
    }

    GgBuffer next_token1 = GG_STR("token123");
    GgBuffer next_token2 = GG_STR("token456");

    static GgObject page1_items[1];
    page1_items[0] = gg_obj_buf(GG_STR("shadow1"));
    GgList page1 = { .items = page1_items, .len = 1 };

    static GgObject page2_items[1];
    page2_items[0] = gg_obj_buf(GG_STR("shadow2"));
    GgList page2 = { .items = page2_items, .len = 1 };

    GgList empty = { .items = NULL, .len = 0 };

    GG_TEST_ASSERT_OK(gg_test_accept_client_handshake(5));

    // First page: returns one shadow with nextToken
    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_shadow_list_accepted_sequence(
            1, THING_NAME, NULL, page1, TIMESTAMP, &next_token1
        ),
        5
    ));

    // Second page: returns one shadow with nextToken
    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_shadow_list_accepted_sequence(
            2, THING_NAME, &next_token1, page2, TIMESTAMP, &next_token2
        ),
        5
    ));

    // Third page: empty results, no nextToken
    GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
        gg_test_shadow_list_accepted_sequence(
            3, THING_NAME, &next_token2, empty, TIMESTAMP, NULL
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(5));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}
