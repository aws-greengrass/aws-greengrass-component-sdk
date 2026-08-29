# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Example: Get a configuration value

import gg_sdk

sdk = gg_sdk.Sdk()
sdk.connect()

# Get a configuration value at key path ["mqtt", "port"]
value = sdk.get_config(["mqtt", "port"])

print(f"Configuration value: {value}")
