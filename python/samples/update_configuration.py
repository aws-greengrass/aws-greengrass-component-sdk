# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Example: Update a configuration value

import gg_sdk

sdk = gg_sdk.Sdk()
sdk.connect()

# Update configuration value at key path ["mqtt", "port"] to 443
sdk.update_config(["mqtt", "port"], 443)

print("Successfully updated configuration.")
