// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef GG_IPC_TYPES_H
#define GG_IPC_TYPES_H

#include <gg/attr.h>
#include <stdint.h>

typedef struct {
    uint32_t val;
} GgIpcSubscriptionHandle;

/// Component state values for UpdateState
typedef enum ENUM_EXTENSIBILITY(closed) {
    GG_COMPONENT_STATE_RUNNING,
    GG_COMPONENT_STATE_ERRORED
} GgComponentState;

#endif
