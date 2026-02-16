
#include "gg/ipc/packet_sequences.h"
#include "packets.h"
#include <assert.h>
#include <gg/ipc/mock.h>
#include <gg/map.h>
#include <gg/object.h>
#include <gg/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

GgipcPacket gg_test_local_publish_request_packet(
    int32_t stream_id, GgBuffer topic, GgObject payload
) {
    static GgKV message_pairs[1];
    message_pairs[0] = gg_kv(GG_STR("message"), payload);
    size_t message_pairs_len = sizeof(message_pairs) / sizeof(message_pairs[0]);

    static GgKV publish_message_pairs[1];

    GgObjectType payload_type = gg_obj_type(payload);
    publish_message_pairs[0] = gg_kv(
        payload_type == GG_TYPE_BUF ? GG_STR("binaryMessage")
                                    : GG_STR("jsonMessage"),
        gg_obj_map((GgMap) { .len = message_pairs_len, .pairs = message_pairs })
    );

    size_t publish_message_pairs_len
        = sizeof(publish_message_pairs) / sizeof(publish_message_pairs[0]);

    static GgKV pairs[2];
    pairs[0] = gg_kv(GG_STR("topic"), gg_obj_buf(topic));
    pairs[1] = gg_kv(
        GG_STR("publishMessage"),
        gg_obj_map((GgMap) { .pairs = publish_message_pairs,
                             .len = publish_message_pairs_len })
    );

    size_t pairs_len = sizeof(pairs) / sizeof(pairs[0]);

    return (GgipcPacket) {
        .direction = CLIENT_TO_SERVER,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = pairs, .len = pairs_len }),
        .headers
        = GG_IPC_REQUEST_HEADERS(stream_id, "aws.greengrass#PublishToTopic"),
        .header_count = GG_IPC_REQUEST_HEADERS_COUNT
    };
}

GgipcPacket gg_test_local_publish_accepted_packet(int32_t stream_id) {
    return (GgipcPacket) { .direction = SERVER_TO_CLIENT,
                           .has_payload = false,
                           .headers = GG_IPC_ACCEPTED_HEADERS(
                               stream_id, "aws.greengrass#PublishToTopic"
                           ),
                           .header_count = GG_IPC_ACCEPTED_HEADERS_COUNT };
}

GgipcPacketSequence gg_test_local_publish_accepted_sequence(
    int32_t stream_id, GgBuffer topic, GgObject payload
) {
    return (GgipcPacketSequence) {
        .packets
        = { gg_test_local_publish_request_packet(stream_id, topic, payload),
            gg_test_local_publish_accepted_packet(stream_id) },
        .len = 2
    };
}

GgipcPacketSequence gg_test_local_publish_error_sequence(
    int32_t stream_id, GgBuffer topic, GgObject payload
) {
    return (GgipcPacketSequence) {
        .packets
        = { gg_test_local_publish_request_packet(stream_id, topic, payload),
            gg_test_ipc_service_error_packet(stream_id) },
        .len = 2
    };
}

GgipcPacket gg_test_local_message_packet(
    int32_t stream_id, GgBuffer topic, GgObject payload
) {
    static GgKV context_pairs[1];
    context_pairs[0] = gg_kv(GG_STR("topic"), gg_obj_buf(topic));

    size_t context_pairs_len = sizeof(context_pairs) / sizeof(context_pairs[0]);

    static GgKV inner_message_pairs[2];
    inner_message_pairs[0] = gg_kv(GG_STR("message"), payload);
    inner_message_pairs[1] = gg_kv(
        GG_STR("context"),
        gg_obj_map((GgMap) { .len = context_pairs_len, .pairs = context_pairs })
    );
    size_t inner_message_pairs_len
        = sizeof(inner_message_pairs) / sizeof(inner_message_pairs[0]);

    static GgKV messages_pairs[2];
    messages_pairs[0] = gg_kv(GG_STR("topic"), gg_obj_buf(topic));
    GgObjectType payload_type = gg_obj_type(payload);
    messages_pairs[1] = gg_kv(
        payload_type == GG_TYPE_BUF ? GG_STR("binaryMessage")
                                    : GG_STR("jsonMessage"),
        gg_obj_map((GgMap) { .len = inner_message_pairs_len,
                             .pairs = inner_message_pairs })
    );
    size_t messages_pairs_len
        = sizeof(messages_pairs) / sizeof(messages_pairs[0]);

    return (GgipcPacket) {
        .direction = SERVER_TO_CLIENT,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = messages_pairs,
                                        .len = messages_pairs_len }),
        .headers = GG_IPC_SUBSCRIBE_MESSAGE_HEADERS(
            stream_id, "aws.greengrass#SubscriptionResponseMessage"
        ),
        .header_count = GG_IPC_SUBSCRIBE_MESSAGE_HEADERS_COUNT,
    };
}

GgipcPacket gg_test_local_subscribe_request_packet(
    int32_t stream_id, GgBuffer topic
) {
    static GgKV pairs[1];
    pairs[0] = gg_kv(GG_STR("topic"), gg_obj_buf(topic));
    size_t pairs_len = sizeof(pairs) / sizeof(pairs[0]);

    return (GgipcPacket) {
        .direction = CLIENT_TO_SERVER,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = pairs, .len = pairs_len }),
        .headers
        = GG_IPC_REQUEST_HEADERS(stream_id, "aws.greengrass#SubscribeToTopic"),
        .header_count = GG_IPC_REQUEST_HEADERS_COUNT
    };
}

GgipcPacket gg_test_local_subscribe_accepted_packet(int32_t stream_id) {
    return (GgipcPacket) {
        .direction = SERVER_TO_CLIENT,
        .has_payload = false,
        .headers = GG_IPC_SUBSCRIBE_MESSAGE_HEADERS(
            stream_id, "aws.greengrass#SubscribeToTopicAccepted"
        ),
        .header_count = GG_IPC_SUBSCRIBE_MESSAGE_HEADERS_COUNT
    };
}

GgipcPacketSequence gg_test_local_subscribe_accepted_sequence(
    int32_t stream_id, GgBuffer topic, GgObject payload, size_t messages
) {
    GgipcPacket response_packet
        = gg_test_local_message_packet(stream_id, topic, payload);

    GgipcPacketSequence seq
        = { .packets
            = { gg_test_local_subscribe_request_packet(stream_id, topic),
                gg_test_local_subscribe_accepted_packet(stream_id) },
            .len = 2 };

    size_t max_len = (sizeof(seq.packets) / sizeof(seq.packets[0]));
    assert(messages <= (max_len - seq.len));

    for (uint32_t i = 0; (i != messages) && (seq.len != max_len);
         ++i, ++seq.len) {
        seq.packets[seq.len] = response_packet;
    }

    return seq;
}
