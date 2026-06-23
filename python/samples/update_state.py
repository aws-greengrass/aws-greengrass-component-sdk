# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Example: Update component state

import gg_sdk

sdk = gg_sdk.Sdk()
sdk.connect()

sdk.update_state(gg_sdk.ComponentState.RUNNING)

print("Successfully updated component state to RUNNING.")
