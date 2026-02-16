#include "test_subscriptions.h"
#include <gg/ipc/client.h>
#include <gg/ipc/mock.h>
#include <gg/ipc/packet_sequences.h>
#include <gg/log.h>
#include <gg/process_wait.h>
#include <gg/sdk.h>
#include <gg/test.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <unity.h>
#include <stdatomic.h>
#include <stdint.h>

#define GG_MODULE "test_mqtt"

static uint8_t too_large_payload[0x20000];

typedef struct {
    GgBuffer payload;
    GgBuffer payload_base64;
} PayloadPairs;

static PayloadPairs payloads[]
    = { { GG_STR("Hello world!"), GG_STR("SGVsbG8gd29ybGQh") },
        { GG_BUF(too_large_payload), /* don't need to encode */ { 0 } } };

GG_TEST_DEFINE(publish_to_iot_core_okay) {
    GgBuffer payload = payloads[0].payload;

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GG_TEST_ASSERT_OK(
            ggipc_publish_to_iot_core(GG_STR("my/topic"), payload, 0)
        );
        TEST_PASS();
    }

    GgBuffer payload_base64 = payloads[0].payload_base64;

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_mqtt_publish_accepted_sequence(
            1, GG_STR("my/topic"), payload_base64, GG_STR("0")
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(publish_to_iot_core_bad_alloc) {
    GgBuffer payload = payloads[1].payload;

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        TEST_ASSERT_EQUAL(
            GG_ERR_NOMEM,
            ggipc_publish_to_iot_core(GG_STR("my/topic"), payload, 0)
        );
        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_accept_client_handshake(1));

    GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(30));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(publish_to_iot_core_rejected) {
    GgBuffer payload = payloads[0].payload;

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GG_TEST_ASSERT_BAD(
            ggipc_publish_to_iot_core(GG_STR("my/topic"), payload, 0)
        );
        TEST_PASS();
    }

    GgBuffer payload_base64 = payloads[0].payload_base64;

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_mqtt_publish_error_sequence(
            1, GG_STR("my/topic"), payload_base64, GG_STR("0")
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

GG_TEST_DEFINE(publish_to_iot_core_invalid_qos) {
    GgBuffer payload = payloads[0].payload;

    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GG_TEST_ASSERT_BAD(
            ggipc_publish_to_iot_core(GG_STR("my/topic"), payload, 10)
        );
        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_mqtt_publish_error_sequence(
            1,
            GG_STR("my/topic"),
            payloads[0].payload_base64,
            GG_BUF((uint8_t[]) { '0' + 10 })
        ),
        5
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}

static GgTestSubscribeContext subscribe_okay_context
    = GG_TEST_OKAY_CONTEXT_INITIALIZER;

static void subscribe_to_iot_core_okay_subscription_response(
    void *ctx, GgBuffer topic, GgBuffer payload, GgIpcSubscriptionHandle handle
) {
    TEST_ASSERT_EQUAL_PTR(
        &subscribe_okay_context, (GgTestSubscribeContext *) ctx
    );
    GG_TEST_ASSERT_BUF_EQUAL_STR(GG_STR("my/topic"), topic);

    GG_TEST_ASSERT_BUF_EQUAL(payloads[0].payload, payload);

    GG_TEST_SUBSCRIPTION_SIGNAL_CALLBACK(ctx, handle);
}

GG_TEST_DEFINE(subscribe_to_iot_core_okay) {
    pid_t pid = fork();
    TEST_ASSERT_TRUE_MESSAGE(pid >= 0, "fork failed");

    if (pid == 0) {
        gg_sdk_init();
        GG_TEST_ASSERT_OK(ggipc_connect());
        GG_TEST_ASSERT_OK(ggipc_subscribe_to_iot_core(
            GG_STR("my/topic"),
            0,
            subscribe_to_iot_core_okay_subscription_response,
            &subscribe_okay_context,
            &subscribe_okay_context.handle
        ));

        TEST_ASSERT_NOT_EQUAL_UINT32(0U, subscribe_okay_context.handle.val);

        GG_TEST_ASSERT_SUBSCRIPTION_CALLED(&subscribe_okay_context, 5);

        TEST_PASS();
    }

    GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
        gg_test_mqtt_subscribe_accepted_sequence(
            1,
            GG_STR("my/topic"),
            payloads[0].payload_base64,
            GG_STR("0"),
            GG_TEST_EXPECTED_TIMES_CALLED
        ),
        30
    ));

    GG_TEST_ASSERT_OK(gg_process_wait(pid));
}
