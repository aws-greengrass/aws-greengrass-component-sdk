# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Example: Get a thing shadow

import os

import gg_sdk

sdk = gg_sdk.Sdk()
sdk.connect()

thing_name = os.environ["AWS_IOT_THING_NAME"]

payload = sdk.get_thing_shadow(thing_name, shadow_name="my-shadow")

print(f"Shadow document: {payload.decode()}")
