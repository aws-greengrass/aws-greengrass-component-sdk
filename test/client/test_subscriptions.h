// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef GG_TEST_SUBSCRIPTION_H
#define GG_TEST_SUBSCRIPTION_H

/// Helpers for subscription IPCs

#include <gg/ipc/client.h>
#include <gg/ipc/types.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>

/// passed to the subscription callback.
/// Used to signal the main client process test thread that the subscription
/// callback was called.
/// Subscription handle needs to be passed as a pointer to the subscription
/// request function, and the entire struct passed in the context pointer
typedef struct {
    pthread_mutex_t mut;
    _Atomic(size_t) calls_remaining;
    pthread_cond_t cond;
    GgIpcSubscriptionHandle handle;
} GgTestSubscribeContext;

#define GG_TEST_EXPECTED_TIMES_CALLED (3U)
#define GG_TEST_OKAY_CONTEXT_INITIALIZER \
    { .mut = PTHREAD_MUTEX_INITIALIZER, \
      .calls_remaining = GG_TEST_EXPECTED_TIMES_CALLED, \
      .cond = PTHREAD_COND_INITIALIZER, \
      .handle = { 0 } };

/// Call inside of the subscription callback thread
/// to signal the main thread about callbacks occuring
void gg_test_subscription_signal_callback(
    GgTestSubscribeContext *context,
    GgIpcSubscriptionHandle handle,
    size_t expected_times_called,
    unsigned int line_number
);

#define GG_TEST_SUBSCRIPTION_SIGNAL_CALLBACK(context, subscription_handle) \
    gg_test_subscription_signal_callback( \
        (GgTestSubscribeContext *) (context), \
        (subscription_handle), \
        GG_TEST_EXPECTED_TIMES_CALLED, \
        __LINE__ \
    )

/// Call in the client process' main thread to await the callback
void gg_test_assert_subscription_called(
    GgTestSubscribeContext *context,
    int timeout_seconds,
    size_t expected_times_called,
    unsigned int line_number
);

#define GG_TEST_ASSERT_SUBSCRIPTION_CALLED(context, timeout_seconds) \
    gg_test_assert_subscription_called( \
        (context), (timeout_seconds), GG_TEST_EXPECTED_TIMES_CALLED, __LINE__ \
    )

#endif
