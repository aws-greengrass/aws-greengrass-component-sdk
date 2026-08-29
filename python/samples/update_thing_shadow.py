# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Example: Update a thing shadow

import os

import gg_sdk

sdk = gg_sdk.Sdk()
sdk.connect()

thing_name = os.environ["AWS_IOT_THING_NAME"]

sdk.update_thing_shadow(
    thing_name,
    b'{"state":{"reported":{"temperature":25}}}',
    shadow_name="my-shadow",
)

print("Shadow updated successfully.")
