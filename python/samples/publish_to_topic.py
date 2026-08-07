# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Example: Publish a binary message to a local topic

import gg_sdk

sdk = gg_sdk.Sdk()
sdk.connect()

sdk.publish_to_topic("my/topic", b"Hello, World")

print("Successfully published to topic: my/topic")
