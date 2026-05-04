# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Example: Publish a message to AWS IoT Core

import gg_sdk

sdk = gg_sdk.Sdk()
sdk.connect()

sdk.publish_to_iot_core("my/topic", b"Hello, World", 1)

print("Successfully published to topic: my/topic")
