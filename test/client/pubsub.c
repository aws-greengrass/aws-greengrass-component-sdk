#include "test_subscriptions.h"
#include <gg/ipc/client.h>
#include <gg/ipc/mock.h>
#include <gg/ipc/packet_sequences.h>
#include <gg/log.h>
#include <gg/process_wait.h>
#include <gg/sdk.h>
#include <gg/test.h>
#include <gg/types.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <unity.h>
#include <stdatomic.h>
#include <stdint.h>

#define GG_MODULE "test_local"

static uint8_t too_large_payload[0x20000];

typedef struct {
    GgBuffer payload;
    GgBuffer payload_base64;
} PayloadPairs;

static PayloadPairs payloads[]
    = { { GG_STR("Hello world!"), GG_STR("SGVsbG8gd29ybGQh") },
        { GG_BUF(too_large_payload), /* don't need to encode */ { 0 } } };

GG_TEST_DEFINE(publish_to_topic_okay) {
    GgBuffer payload = payloads[0].payload;

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GG_TEST_ASSERT_OK(
            ggipc_publish_to_topic_binary(GG_STR("my/topic"), payload)
        );
        TEST_PASS();
    }

    GgBuffer payload_base64 = payloads[0].payload_base64;

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_local_publish_accepted_sequence(
            1, GG_STR("my/topic"), gg_obj_buf(payload_base64)
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(publish_to_topic_bad_alloc) {
    GgBuffer payload = payloads[1].payload;

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        TEST_ASSERT_EQUAL(
            GG_ERR_NOMEM,
            ggipc_publish_to_topic_binary(GG_STR("my/topic"), payload)
        );
        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_accept_client_handshake(1));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(30));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(publish_to_topic_rejected) {
    GgBuffer payload = payloads[0].payload;

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GG_TEST_ASSERT_BAD(
            ggipc_publish_to_topic_binary(GG_STR("my/topic"), payload)
        );
        TEST_PASS();
    }

    GgBuffer payload_base64 = payloads[0].payload_base64;

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_local_publish_error_sequence(
            1, GG_STR("my/topic"), gg_obj_buf(payload_base64)
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

static GgTestSubscribeContext subscribe_okay_context
    = GG_TEST_OKAY_CONTEXT_INITIALIZER;

static void subscribe_to_topic_okay_subscription_response(
    void *ctx, GgBuffer topic, GgObject payload, GgIpcSubscriptionHandle handle
) {
    TEST_ASSERT_EQUAL_PTR(
        &subscribe_okay_context, (GgTestSubscribeContext *) ctx
    );
    GG_TEST_ASSERT_BUF_EQUAL_STR(GG_STR("my/topic"), topic);

    TEST_ASSERT_EQUAL_INT(GG_TYPE_BUF, gg_obj_type(payload));

    GG_TEST_ASSERT_BUF_EQUAL(payloads[0].payload, gg_obj_into_buf(payload));

    GG_TEST_SUBSCRIPTION_SIGNAL_CALLBACK(ctx, handle);
}

GG_TEST_DEFINE(subscribe_to_topic_okay) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GG_TEST_ASSERT_OK(ggipc_subscribe_to_topic(
            GG_STR("my/topic"),
            subscribe_to_topic_okay_subscription_response,
            &subscribe_okay_context,
            &subscribe_okay_context.handle
        ));

        TEST_ASSERT_NOT_EQUAL_UINT32(0U, subscribe_okay_context.handle.val);

        GG_TEST_ASSERT_SUBSCRIPTION_CALLED(&subscribe_okay_context, 5);

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_local_subscribe_accepted_sequence(
            1,
            GG_STR("my/topic"),
            gg_obj_buf(payloads[0].payload_base64),
            GG_TEST_EXPECTED_TIMES_CALLED
        ),
        30
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}
