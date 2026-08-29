# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Example: Subscribe to messages from AWS IoT Core

import time

import gg_sdk

sdk = gg_sdk.Sdk()
sdk.connect()


def on_message(topic: str, payload: bytes):
    print(f"Received new message on topic {topic}: {payload.decode()}")


with sdk.subscribe_to_iot_core("my/topic", 1, on_message):
    print("Successfully subscribed to topic: my/topic")

    # Keep the main thread alive, or the process will exit.
    while True:
        time.sleep(10)
