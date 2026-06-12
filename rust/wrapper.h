// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// output bindgen for __attribute__((hidden)) functions
#include <gg/attr.h>
#undef VISIBILITY
#define VISIBILITY(v)

#include <gg/ipc/client.h>
#include <gg/ipc/client_raw.h>
#include <gg/ipc/mock.h>
#include <gg/ipc/packet_sequences.h>
#include <gg/log.h>
#include <gg/map.h>
#include <gg/sdk.h>
#include <time.h>
