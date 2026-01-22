# AWS Greengrass Component SDK

The `aws-greengrass-component-sdk` is a small-footprint library providing APIs
for making AWS IoT Greengrass IPC calls in C, Rust, and C++. It enables
Greengrass components to interact with the Greengrass runtime (Greengrass
Nucleus or Greeengrass Nucleus Lite) with less binary overhead. Components can
use this SDK as an alternative to the `aws-iot-device-sdk-cpp-v2` or other
language-specific device SDKs.

### ⚠️ Important Notice ⚠️

This library is in pre-release status. We encourage users to test it and report
any bugs or missing features.

For development purposes, it is recommended to pin your usage to a specific
commit or tag.

## Building

For building the SDK and samples for C and/or C++, see the
[build guide](docs/BUILD.md).

For Rust, the `rust` subdirectory provides a Rust crate.

## Supported Operations

The following Greengrass v2 IPC operations are currently supported by this SDK:

- [PublishToTopic](https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-publish-subscribe.html#ipc-operation-publishtotopic)
- [SubscribeToTopic](https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-publish-subscribe.html#ipc-operation-subscribetotopic)
- [PublishToIoTCore](https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-iot-core-mqtt.html#ipc-operation-publishtoiotcore)
- [SubscribeToIoTCore](https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-iot-core-mqtt.html#ipc-operation-subscribetoiotcore)
- [UpdateState](https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-component-lifecycle.html#ipc-operation-updatestate)
- [GetConfiguration](https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-component-configuration.html#ipc-operation-getconfiguration)
- [UpdateConfiguration](https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-component-configuration.html#ipc-operation-updateconfiguration)
- [SubscribeToConfigurationUpdate](https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-component-configuration.html#ipc-operation-subscribetoconfigurationupdate)
- RestartComponent

## Sample Generic Components

For deployment instructions, see the
[samples deployment guide](samples/README.md).

C samples:

- [IoT Core MQTT Pub/Sub](samples/iot_core_mqtt/)
- [Update State](samples/update_state/)
- [Configuration Operations](samples/config/)
- [Subscribe to Configuration Update](samples/subscribe_to_configuration_update/)
- [Restart Component](samples/restart_component/)

Rust samples (use same recipe as corresponding C sample):

- [IoT Core MQTT Pub/Sub](rust/examples/iot_core_mqtt.rs)
- [Update State](rust/examples/update_state.rs)
- [Configuration Operations](rust/examples/config.rs)
- [Subscribe To Configuration Update](rust/examples/subscribe_to_configuration_update.rs)
- [Restart Component](rust/examples/restart_component.rs)

C++ samples:

- [Local Pub/Sub](cpp/samples/pubsub_cpp)
- [Configuration Operations](cpp/samples/config_ipc)
- [Manipulaing SDK data structures](cpp/samples/object_manipulation)

## Security

See [CONTRIBUTING](docs/CONTRIBUTING.md#security-issue-notifications) for more
information.

## License

This project is licensed under the Apache-2.0 License.
