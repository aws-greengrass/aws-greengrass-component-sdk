// aws-greengrass-lite - AWS IoT Greengrass runtime for constrained devices
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "gg/ipc/mock.h"
#include "packets.h"
#include <gg/ipc/client.h>
#include <gg/ipc/packet_sequences.h>
#include <gg/map.h>
#include <gg/object.h>
#include <gg/types.h>

GgipcPacket gg_test_update_state_request_packet(
    int32_t stream_id, GgBuffer state
) {
    static GgKV payload[1];
    payload[0] = gg_kv(GG_STR("state"), gg_obj_buf(state));
    size_t payload_len = sizeof(payload) / sizeof(payload[0]);

    return (GgipcPacket) {
        .direction = CLIENT_TO_SERVER,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = payload, .len = payload_len }),
        .headers
        = GG_IPC_REQUEST_HEADERS(stream_id, "aws.greengrass#UpdateState"),
        .header_count = GG_IPC_REQUEST_HEADERS_COUNT
    };
}

GgipcPacket gg_test_update_state_response_packet(int32_t stream_id) {
    return (GgipcPacket) { .direction = SERVER_TO_CLIENT,
                           .has_payload = false,
                           .headers = GG_IPC_ACCEPTED_HEADERS(
                               stream_id, "aws.greengrass#UpdateStateAccepted"
                           ),
                           .header_count = GG_IPC_ACCEPTED_HEADERS_COUNT };
}

GgipcPacketSequence gg_test_update_state_accepted_sequence(
    int32_t stream_id, GgBuffer state
) {
    return (GgipcPacketSequence) {
        .packets = { gg_test_update_state_request_packet(stream_id, state),
                     gg_test_update_state_response_packet(stream_id) },
        .len = 2
    };
}

GgipcPacketSequence gg_test_update_state_error_sequence(
    int32_t stream_id, GgBuffer state
) {
    return (GgipcPacketSequence) {
        .packets = { gg_test_update_state_request_packet(stream_id, state),
                     gg_test_ipc_permissions_error_packet(stream_id) },
        .len = 2
    };
}

GgipcPacket gg_test_restart_component_request_packet(
    int32_t stream_id, GgBuffer component_name
) {
    static GgKV payload[1];
    payload[0] = gg_kv(GG_STR("componentName"), gg_obj_buf(component_name));
    size_t payload_len = sizeof(payload) / sizeof(payload[0]);

    return (GgipcPacket) {
        .direction = CLIENT_TO_SERVER,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = payload, .len = payload_len }),
        .headers
        = GG_IPC_REQUEST_HEADERS(stream_id, "aws.greengrass#RestartComponent"),
        .header_count = GG_IPC_REQUEST_HEADERS_COUNT
    };
}

GgipcPacket gg_test_restart_component_response_packet(
    int32_t stream_id, GgBuffer restart_status
) {
    static GgKV payload[1];
    payload[0] = gg_kv(GG_STR("restartStatus"), gg_obj_buf(restart_status));
    size_t payload_len = sizeof(payload) / sizeof(payload[0]);

    return (GgipcPacket) {
        .direction = SERVER_TO_CLIENT,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = payload, .len = payload_len }),
        .headers = GG_IPC_ACCEPTED_HEADERS(
            stream_id, "aws.greengrass#RestartComponentAccepted"
        ),
        .header_count = GG_IPC_ACCEPTED_HEADERS_COUNT
    };
}

GgipcPacketSequence gg_test_restart_component_accepted_sequence(
    int32_t stream_id, GgBuffer component_name, GgBuffer restart_status
) {
    return (GgipcPacketSequence) {
        .packets
        = { gg_test_restart_component_request_packet(stream_id, component_name),
            gg_test_restart_component_response_packet(
                stream_id, restart_status
            ) },
        .len = 2
    };
}

GgipcPacketSequence gg_test_restart_component_error_sequence(
    int32_t stream_id, GgBuffer component_name
) {
    return (GgipcPacketSequence) {
        .packets
        = { gg_test_restart_component_request_packet(stream_id, component_name),
            gg_test_ipc_permissions_error_packet(stream_id) },
        .len = 2
    };
}

GgipcPacket gg_test_create_local_deployment_request_packet(
    int32_t stream_id, GgObject component_to_configuration
) {
    static GgKV payload[1];
    payload[0]
        = gg_kv(GG_STR("componentToConfiguration"), component_to_configuration);
    size_t payload_len = sizeof(payload) / sizeof(payload[0]);

    return (GgipcPacket) { .direction = CLIENT_TO_SERVER,
                           .has_payload = true,
                           .payload = gg_obj_map((GgMap) {
                               .pairs = payload, .len = payload_len }),
                           .headers = GG_IPC_REQUEST_HEADERS(
                               stream_id, "aws.greengrass#CreateLocalDeployment"
                           ),
                           .header_count = GG_IPC_REQUEST_HEADERS_COUNT };
}

GgipcPacket gg_test_create_local_deployment_response_packet(
    int32_t stream_id, GgBuffer deployment_id
) {
    static GgKV payload[1];
    payload[0] = gg_kv(GG_STR("deploymentId"), gg_obj_buf(deployment_id));
    size_t payload_len = sizeof(payload) / sizeof(payload[0]);

    return (GgipcPacket) { .direction = SERVER_TO_CLIENT,
                           .has_payload = true,
                           .payload = gg_obj_map((GgMap) {
                               .pairs = payload, .len = payload_len }),
                           .headers = GG_IPC_ACCEPTED_HEADERS(
                               stream_id, "aws.greengrass#CreateLocalDeployment"
                           ),
                           .header_count = GG_IPC_ACCEPTED_HEADERS_COUNT };
}

GgipcPacketSequence gg_test_create_local_deployment_accepted_sequence(
    int32_t stream_id,
    GgObject component_to_configuration,
    GgBuffer deployment_id
) {
    return (GgipcPacketSequence) {
        .packets = { gg_test_create_local_deployment_request_packet(
                         stream_id, component_to_configuration
                     ),
                     gg_test_create_local_deployment_response_packet(
                         stream_id, deployment_id
                     ) },
        .len = 2
    };
}

GgipcPacketSequence gg_test_create_local_deployment_error_sequence(
    int32_t stream_id, GgObject component_to_configuration
) {
    return (GgipcPacketSequence) {
        .packets = { gg_test_create_local_deployment_request_packet(
                         stream_id, component_to_configuration
                     ),
                     gg_test_ipc_permissions_error_packet(stream_id) },
        .len = 2
    };
}

GgipcPacket gg_test_create_local_deployment_versions_request_packet(
    int32_t stream_id, GgObject root_component_versions_to_add
) {
    static GgKV payload[1];
    payload[0] = gg_kv(
        GG_STR("rootComponentVersionsToAdd"), root_component_versions_to_add
    );
    size_t payload_len = sizeof(payload) / sizeof(payload[0]);

    return (GgipcPacket) { .direction = CLIENT_TO_SERVER,
                           .has_payload = true,
                           .payload = gg_obj_map((GgMap) {
                               .pairs = payload, .len = payload_len }),
                           .headers = GG_IPC_REQUEST_HEADERS(
                               stream_id, "aws.greengrass#CreateLocalDeployment"
                           ),
                           .header_count = GG_IPC_REQUEST_HEADERS_COUNT };
}

GgipcPacketSequence gg_test_create_local_deployment_versions_accepted_sequence(
    int32_t stream_id,
    GgObject root_component_versions_to_add,
    GgBuffer deployment_id
) {
    return (GgipcPacketSequence) {
        .packets = { gg_test_create_local_deployment_versions_request_packet(
                         stream_id, root_component_versions_to_add
                     ),
                     gg_test_create_local_deployment_response_packet(
                         stream_id, deployment_id
                     ) },
        .len = 2
    };
}
