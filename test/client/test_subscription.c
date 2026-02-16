// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "test_subscriptions.h"
#include <gg/cleanup.h>
#include <gg/log.h>
#include <pthread.h>
#include <unity.h>

#define GG_MODULE "gg-test"

void gg_test_subscription_signal_callback(
    GgTestSubscribeContext *context,
    GgIpcSubscriptionHandle handle,
    size_t expected_times_called,
    unsigned int line_number
) {
    UnityAssertEqualNumber(
        (UNITY_INT) (UNITY_UINT32) ((context->handle.val)),
        (UNITY_INT) (UNITY_UINT32) ((handle.val)),
        (((void *) 0)),
        (UNITY_UINT) (line_number),
        UNITY_DISPLAY_STYLE_UINT32
    );

    GG_MTX_SCOPE_GUARD(&context->mut);

    size_t calls_remaining_prev
        = atomic_fetch_sub(&context->calls_remaining, 1);
    if ((calls_remaining_prev - 1 == 0)
        || (calls_remaining_prev > expected_times_called)) {
        pthread_cond_signal(&context->cond);
    }
}

void gg_test_assert_subscription_called(
    GgTestSubscribeContext *context,
    int timeout_seconds,
    size_t expected_times_called,
    unsigned int line_number
) {
    int pthread_ret = 0;
    size_t calls_remaining;
    struct timespec wait_until;
    clock_gettime(CLOCK_REALTIME, &wait_until);
    wait_until.tv_sec += timeout_seconds;

    {
        GG_MTX_SCOPE_GUARD(&context->mut);
        while (((calls_remaining = atomic_load(&context->calls_remaining)) != 0)
               && (calls_remaining <= expected_times_called)) {
            pthread_ret = pthread_cond_timedwait(
                &context->cond, &context->mut, &wait_until
            );
            if (pthread_ret != 0) {
                GG_LOGW("pthread_cond_timedwait timed out");
                calls_remaining = atomic_load(&context->calls_remaining);
                break;
            }
        }

        UnityAssertEqualNumber(
            (UNITY_INT) expected_times_called,
            (UNITY_INT) (expected_times_called - calls_remaining),
            "Subscription not called the expected number of times.",
            (UNITY_UINT) line_number,
            UNITY_DISPLAY_STYLE_UINT
        );

        UnityAssertEqualNumber(
            (UNITY_INT) 0,
            (UNITY_INT) pthread_ret,
            "Timed out waiting for subscription response(s)",
            (UNITY_UINT) line_number,
            UNITY_DISPLAY_STYLE_INT
        );
    }
}
