# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Example: List named shadows for a thing

import os

import gg_sdk

sdk = gg_sdk.Sdk()
sdk.connect()

thing_name = os.environ["AWS_IOT_THING_NAME"]

shadows = sdk.list_named_shadows(thing_name)

print("Named shadows:")
for name in shadows:
    print(f"  - {name}")
