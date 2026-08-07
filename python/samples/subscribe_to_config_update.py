# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Example: Subscribe to configuration updates

import time

import gg_sdk

sdk = gg_sdk.Sdk()
sdk.connect()


def on_config_update(component_name: str, key_path: list[str]):
    print(f"Received configuration update for component: {component_name}")
    print(f"Key path: {key_path}")


# Subscribe to configuration updates for key path ["mqtt"]
with sdk.subscribe_to_configuration_update(["mqtt"], on_config_update):
    print("Successfully subscribed to configuration updates.")

    # Keep the main thread alive, or the process will exit.
    while True:
        time.sleep(10)
