// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/buffer.hpp>
#include <gg/ipc/client.hpp>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <system_error>
extern "C" {
#include <gg/ipc/mock.h>
#include <gg/ipc/packet_sequences.h>
#include <gg/test.h>
#include <sys/types.h>
#include <unistd.h>
#include <unity.h>

GgError gg_process_wait(pid_t pid) noexcept;
}

static constexpr std::string_view THING_NAME = "MyThing";
static constexpr std::string_view SHADOW_NAME = "myShadow";
static constexpr std::string_view PAYLOAD = "hello";
static constexpr std::string_view PAYLOAD_B64 = "aGVsbG8=";

extern "C" {
namespace tests {
    namespace {

        GG_TEST_DEFINE(get_thing_shadow_okay) {
            pid_t pid = fork();
            if (pid < 0) {
                TEST_IGNORE_MESSAGE("fork() failed.");
            }

            if (pid == 0) {
                auto &client = gg::ipc::Client::get();
                GG_TEST_ASSERT_OK(client.connect().value());

                std::byte mem[64];
                std::string_view payload;
                GG_TEST_ASSERT_OK(
                    client
                        .get_thing_shadow(
                            THING_NAME, SHADOW_NAME, std::span(mem), payload
                        )
                        .value()
                );
                TEST_ASSERT_EQUAL_size_t(PAYLOAD.size(), payload.size());
                TEST_ASSERT_EQUAL_STRING_LEN(
                    PAYLOAD.data(), payload.data(), PAYLOAD.size()
                );

                TEST_PASS();
            }

            GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
                gg_test_shadow_get_accepted_sequence(
                    1,
                    gg::Buffer { THING_NAME },
                    gg::Buffer { SHADOW_NAME },
                    gg::Buffer { PAYLOAD_B64 }
                ),
                5
            ));

            GG_TEST_ASSERT_OK(gg_process_wait(pid));
        }

        GG_TEST_DEFINE(get_thing_shadow_rejected) {
            pid_t pid = fork();
            if (pid < 0) {
                TEST_IGNORE_MESSAGE("fork() failed.");
            }

            if (pid == 0) {
                auto &client = gg::ipc::Client::get();
                GG_TEST_ASSERT_OK(client.connect().value());

                std::byte mem[64];
                std::string_view payload;
                GG_TEST_ASSERT_BAD(
                    client
                        .get_thing_shadow(
                            THING_NAME, SHADOW_NAME, std::span(mem), payload
                        )
                        .value()
                );

                TEST_PASS();
            }

            GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
                gg_test_shadow_get_error_sequence(
                    1, gg::Buffer { THING_NAME }, gg::Buffer { SHADOW_NAME }
                ),
                5
            ));

            GG_TEST_ASSERT_OK(gg_process_wait(pid));
        }

        GG_TEST_DEFINE(update_thing_shadow_okay) {
            pid_t pid = fork();
            if (pid < 0) {
                TEST_IGNORE_MESSAGE("fork() failed.");
            }

            if (pid == 0) {
                auto &client = gg::ipc::Client::get();
                GG_TEST_ASSERT_OK(client.connect().value());

                GG_TEST_ASSERT_OK(
                    client
                        .update_thing_shadow(
                            THING_NAME, SHADOW_NAME, gg::Buffer { PAYLOAD }
                        )
                        .value()
                );

                TEST_PASS();
            }

            GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
                gg_test_shadow_update_accepted_sequence(
                    1,
                    gg::Buffer { THING_NAME },
                    gg::Buffer { SHADOW_NAME },
                    gg::Buffer { PAYLOAD_B64 },
                    gg::Buffer { PAYLOAD_B64 }
                ),
                5
            ));

            GG_TEST_ASSERT_OK(gg_process_wait(pid));
        }

        GG_TEST_DEFINE(update_thing_shadow_rejected) {
            pid_t pid = fork();
            if (pid < 0) {
                TEST_IGNORE_MESSAGE("fork() failed.");
            }

            if (pid == 0) {
                auto &client = gg::ipc::Client::get();
                GG_TEST_ASSERT_OK(client.connect().value());

                GG_TEST_ASSERT_BAD(
                    client
                        .update_thing_shadow(
                            THING_NAME, SHADOW_NAME, gg::Buffer { PAYLOAD }
                        )
                        .value()
                );

                TEST_PASS();
            }

            GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
                gg_test_shadow_update_error_sequence(
                    1,
                    gg::Buffer { THING_NAME },
                    gg::Buffer { SHADOW_NAME },
                    gg::Buffer { PAYLOAD_B64 }
                ),
                5
            ));

            GG_TEST_ASSERT_OK(gg_process_wait(pid));
        }

        GG_TEST_DEFINE(delete_thing_shadow_okay) {
            pid_t pid = fork();
            if (pid < 0) {
                TEST_IGNORE_MESSAGE("fork() failed.");
            }

            if (pid == 0) {
                auto &client = gg::ipc::Client::get();
                GG_TEST_ASSERT_OK(client.connect().value());

                GG_TEST_ASSERT_OK(
                    client.delete_thing_shadow(THING_NAME, SHADOW_NAME).value()
                );

                TEST_PASS();
            }

            GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
                gg_test_shadow_delete_accepted_sequence(
                    1,
                    gg::Buffer { THING_NAME },
                    gg::Buffer { SHADOW_NAME },
                    gg::Buffer { "" }
                ),
                5
            ));

            GG_TEST_ASSERT_OK(gg_process_wait(pid));
        }

        GG_TEST_DEFINE(delete_thing_shadow_rejected) {
            pid_t pid = fork();
            if (pid < 0) {
                TEST_IGNORE_MESSAGE("fork() failed.");
            }

            if (pid == 0) {
                auto &client = gg::ipc::Client::get();
                GG_TEST_ASSERT_OK(client.connect().value());

                GG_TEST_ASSERT_BAD(
                    client.delete_thing_shadow(THING_NAME, SHADOW_NAME).value()
                );

                TEST_PASS();
            }

            GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
                gg_test_shadow_delete_error_sequence(
                    1, gg::Buffer { THING_NAME }, gg::Buffer { SHADOW_NAME }
                ),
                5
            ));

            GG_TEST_ASSERT_OK(gg_process_wait(pid));
        }

        GG_TEST_DEFINE(list_named_shadows_okay) {
            pid_t pid = fork();
            if (pid < 0) {
                TEST_IGNORE_MESSAGE("fork() failed.");
            }

            if (pid == 0) {
                auto &client = gg::ipc::Client::get();
                GG_TEST_ASSERT_OK(client.connect().value());

                size_t count = 0;
                GG_TEST_ASSERT_OK(client
                                      .list_named_shadows_for_thing(
                                          THING_NAME,
                                          +[](void *ctx,
                                              std::string_view name) {
                                              TEST_ASSERT_EQUAL_STRING_LEN(
                                                  SHADOW_NAME.data(),
                                                  name.data(),
                                                  SHADOW_NAME.size()
                                              );
                                              (*static_cast<size_t *>(ctx))++;
                                          },
                                          &count
                                      )
                                      .value());
                TEST_ASSERT_EQUAL_UINT(1, count);

                TEST_PASS();
            }

            GgObject result_item = gg_obj_buf(gg::Buffer { SHADOW_NAME });
            GgList results = { .items = &result_item, .len = 1 };

            GG_TEST_ASSERT_OK(gg_test_accept_client_handshake(5));

            GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
                gg_test_shadow_list_accepted_sequence(
                    1,
                    gg::Buffer { THING_NAME },
                    nullptr,
                    results,
                    1773436831.0,
                    nullptr
                ),
                5
            ));

            GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(5));

            GG_TEST_ASSERT_OK(gg_process_wait(pid));
        }

        GG_TEST_DEFINE(list_named_shadows_rejected) {
            pid_t pid = fork();
            if (pid < 0) {
                TEST_IGNORE_MESSAGE("fork() failed.");
            }

            if (pid == 0) {
                auto &client = gg::ipc::Client::get();
                GG_TEST_ASSERT_OK(client.connect().value());

                size_t count = 0;
                GG_TEST_ASSERT_BAD(client
                                       .list_named_shadows_for_thing(
                                           THING_NAME,
                                           +[](void *ctx, std::string_view) {
                                               (*static_cast<size_t *>(ctx))++;
                                           },
                                           &count
                                       )
                                       .value());
                TEST_ASSERT_EQUAL_UINT(0, count);

                TEST_PASS();
            }

            GG_TEST_ASSERT_OK(gg_test_connect_request_disconnect_sequence(
                gg_test_shadow_list_error_sequence(
                    1, gg::Buffer { THING_NAME }
                ),
                5
            ));

            GG_TEST_ASSERT_OK(gg_process_wait(pid));
        }

        GG_TEST_DEFINE(list_named_shadows_paginated_okay) {
            pid_t pid = fork();
            if (pid < 0) {
                TEST_IGNORE_MESSAGE("fork() failed.");
            }

            if (pid == 0) {
                auto &client = gg::ipc::Client::get();
                GG_TEST_ASSERT_OK(client.connect().value());

                static constexpr std::string_view expected[]
                    = { "shadow1", "shadow2" };
                size_t count = 0;
                GG_TEST_ASSERT_OK(
                    client
                        .list_named_shadows_for_thing(
                            THING_NAME,
                            +[](void *ctx, std::string_view name) {
                                auto &i = *static_cast<size_t *>(ctx);
                                TEST_ASSERT_EQUAL_STRING_LEN(
                                    expected[i].data(),
                                    name.data(),
                                    expected[i].size()
                                );
                                i++;
                            },
                            &count
                        )
                        .value()
                );
                TEST_ASSERT_EQUAL_UINT(2, count);

                TEST_PASS();
            }

            GgBuffer next_token1 = gg::Buffer { "token123" };
            GgBuffer next_token2 = gg::Buffer { "token456" };

            GgObject page1_item = gg_obj_buf(gg::Buffer { "shadow1" });
            GgList page1 = { .items = &page1_item, .len = 1 };

            GgObject page2_item = gg_obj_buf(gg::Buffer { "shadow2" });
            GgList page2 = { .items = &page2_item, .len = 1 };

            GgList empty = { .items = nullptr, .len = 0 };

            GG_TEST_ASSERT_OK(gg_test_accept_client_handshake(5));

            GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
                gg_test_shadow_list_accepted_sequence(
                    1,
                    gg::Buffer { THING_NAME },
                    nullptr,
                    page1,
                    1773436831.0,
                    &next_token1
                ),
                5
            ));

            GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
                gg_test_shadow_list_accepted_sequence(
                    2,
                    gg::Buffer { THING_NAME },
                    &next_token1,
                    page2,
                    1773436831.0,
                    &next_token2
                ),
                5
            ));

            GG_TEST_ASSERT_OK(gg_test_expect_packet_sequence(
                gg_test_shadow_list_accepted_sequence(
                    3,
                    gg::Buffer { THING_NAME },
                    &next_token2,
                    empty,
                    1773436831.0,
                    nullptr
                ),
                5
            ));

            GG_TEST_ASSERT_OK(gg_test_wait_for_client_disconnect(5));

            GG_TEST_ASSERT_OK(gg_process_wait(pid));
        }

    }
}
}
