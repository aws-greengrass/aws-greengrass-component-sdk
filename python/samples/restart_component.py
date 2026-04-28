# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Example: Restart a component

import gg_sdk

sdk = gg_sdk.Sdk()
sdk.connect()

sdk.restart_component("com.example.HelloWorld")

print("Successfully requested restart for component: com.example.HelloWorld")
