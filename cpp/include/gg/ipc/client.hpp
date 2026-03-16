// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef GG_IPC_CLIENT_HPP
#define GG_IPC_CLIENT_HPP

#include <gg/buffer.hpp>
#include <gg/list.hpp>
#include <gg/map.hpp>
#include <gg/object.hpp>
#include <gg/sdk.hpp>
#include <cstddef>
#include <cstdint>
#include <chrono>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

extern "C" {
#include <gg/ipc/types.h>
}

namespace gg::ipc {

class Subscription;

class AuthToken {
    Buffer token;

public:
    constexpr AuthToken() noexcept = default;

    explicit AuthToken(std::string_view token) noexcept
        : token { gg::Buffer { token } } { };

    explicit constexpr AuthToken(const gg::Buffer &token) noexcept
        : token { token } {
    }

    explicit operator Buffer() const noexcept {
        return Buffer { token };
    }

    static std::optional<AuthToken> from_environment() noexcept;
};

using ComponentState = GgComponentState;

class LocalTopicCallback {
public:
    virtual ~LocalTopicCallback() noexcept = default;
    virtual void operator()(
        std::string_view topic, gg::Object payload, Subscription &handle
    ) = 0;
};

class IotTopicCallback {
public:
    virtual ~IotTopicCallback() noexcept = default;
    virtual void operator()(
        std::string_view topic, gg::Buffer payload, Subscription &handle
    ) = 0;
};

class ConfigurationUpdateCallback {
public:
    virtual ~ConfigurationUpdateCallback() noexcept = default;
    virtual void operator()(
        std::string_view component_name, gg::List key_path, Subscription &handle
    ) = 0;
};

class Client {
private:
    constexpr Client() noexcept = default;

public:
    static Client &get() noexcept {
        (void) Sdk::get();
        static Client singleton {};
        return singleton;
    }

    /// Connect to the Greengrass Nucleus from a component.
    /// Uses SVCUID and AWS_GG_NUCLEUS_DOMAIN_SOCKET_FILEPATH_FOR_COMPONENT
    /// environment variables.
    /// Not thread-safe if another thread modifies environment variables.
    std::error_code connect() noexcept;

    /// Connect to a GG-IPC socket with a given auth token.
    /// Thread-safe alternative to connect() that does not read environment
    /// variables.
    std::error_code connect(
        std::string_view socket_path, AuthToken auth_token
    ) noexcept;

    /// Publish a binary message to a local pub/sub topic.
    /// Sends messages to other Greengrass components subscribed to the topic.
    /// Requires aws.greengrass#PublishToTopic authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-publish-subscribe.html#ipc-operation-publishtotopic>
    std::error_code publish_to_topic(
        std::string_view topic, Buffer bytes
    ) noexcept;

    /// Publish a JSON message to a local pub/sub topic.
    /// Sends messages to other Greengrass components subscribed to the topic.
    /// Requires aws.greengrass#PublishToTopic authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-publish-subscribe.html#ipc-operation-publishtotopic>
    std::error_code publish_to_topic(
        std::string_view topic, const Map &json
    ) noexcept;

    /// Subscribe to messages on a local pub/sub topic.
    /// Receives messages from other Greengrass components publishing to the
    /// topic.
    /// Requires aws.greengrass#SubscribeToTopic authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-publish-subscribe.html#ipc-operation-subscribetotopic>
    std::error_code subscribe_to_topic(
        std::string_view topic,
        LocalTopicCallback &callback,
        Subscription *handle = nullptr
    ) noexcept;

    /// Publish an MQTT message to AWS IoT Core.
    /// Sends messages to AWS IoT Core MQTT broker with specified QoS.
    /// Requires aws.greengrass#PublishToIoTCore authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-iot-core-mqtt.html#ipc-operation-publishtoiotcore>
    std::error_code publish_to_iot_core(
        std::string_view topic, Buffer bytes, uint8_t qos
    ) noexcept;

    /// Subscribe to MQTT messages from AWS IoT Core.
    /// Receives messages from AWS IoT Core MQTT broker on matching topics.
    /// Requires aws.greengrass#SubscribeToIoTCore authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-iot-core-mqtt.html#ipc-operation-subscribetoiotcore>
    std::error_code subscribe_to_iot_core(
        std::string_view topic_filter,
        std::uint8_t qos,
        IotTopicCallback &callback,
        Subscription *handle = nullptr
    ) noexcept;

    /// Update component configuration.
    /// Merges the provided value into the component's configuration at the key
    /// path.
    /// Requires aws.greengrass#UpdateConfiguration authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-component-configuration.html#ipc-operation-updateconfiguration>
    std::error_code update_config(
        std::span<const Buffer> key_path,
        const Object &value,
        std::chrono::system_clock::time_point timestamp = {}
    ) noexcept;

    /// Update the state of this component.
    /// Reports component state to the Greengrass nucleus.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-component-lifecycle.html#ipc-operation-updatestate>
    std::error_code update_component_state(ComponentState state) noexcept;

    /// Restart a Greengrass component.
    /// Requests the nucleus to restart the specified component.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-local-deployments-components.html#ipc-operation-restartcomponent>
    std::error_code restart_component(std::string_view component_name) noexcept;

    /// Get component configuration value as a string.
    /// Alternative API to the Object overload for string type values.
    /// Pass std::nullopt for component_name to use the current component.
    /// Requires aws.greengrass#GetConfiguration authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-component-configuration.html#ipc-operation-getconfiguration>
    std::error_code get_config(
        std::span<const Buffer> key_path,
        std::optional<std::string_view> component_name,
        std::span<std::byte> value_mem,
        std::string_view &value
    ) noexcept;

    /// Get component configuration value.
    /// Retrieves configuration for the specified key path.
    /// Pass empty span for complete config.
    /// Pass std::nullopt for component_name to use the current component.
    /// Requires aws.greengrass#GetConfiguration authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-component-configuration.html#ipc-operation-getconfiguration>
    std::error_code get_config(
        std::span<const Buffer> key_path,
        std::optional<std::string_view> component_name,
        std::span<std::byte> value_mem,
        Object &value
    ) noexcept;

    /// Get component configuration value as an integer.
    /// Alternative API to the Object overload for integer type values.
    /// Pass std::nullopt for component_name to use the current component.
    /// Requires aws.greengrass#GetConfiguration authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-component-configuration.html#ipc-operation-getconfiguration>
    std::error_code get_config(
        std::span<const Buffer> key_path,
        std::optional<std::string_view> component_name,
        std::int64_t &value
    ) noexcept;

    /// Get component configuration value as a double.
    /// Alternative API to the Object overload for floating point type values.
    /// Pass std::nullopt for component_name to use the current component.
    /// Requires aws.greengrass#GetConfiguration authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-component-configuration.html#ipc-operation-getconfiguration>
    std::error_code get_config(
        std::span<const Buffer> key_path,
        std::optional<std::string_view> component_name,
        double &value
    ) noexcept;

    /// Get component configuration value as a boolean.
    /// Alternative API to the Object overload for boolean type values.
    /// Pass std::nullopt for component_name to use the current component.
    /// Requires aws.greengrass#GetConfiguration authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-component-configuration.html#ipc-operation-getconfiguration>
    std::error_code get_config(
        std::span<const Buffer> key_path,
        std::optional<std::string_view> component_name,
        bool &value
    ) noexcept;

    /// Get the shadow for a thing.
    /// Retrieves the shadow document for the specified thing and shadow name.
    /// Pass std::nullopt for shadow_name to use the thing's classic shadow.
    /// Requires aws.greengrass#GetThingShadow authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-local-shadows.html#ipc-operation-getthingshadow>
    std::error_code get_thing_shadow(
        std::string_view thing_name,
        std::optional<std::string_view> shadow_name,
        std::span<std::byte> payload_mem,
        std::string_view &payload
    ) noexcept;

    /// Update the shadow for a thing.
    /// Updates the shadow document for the specified thing and shadow name.
    /// Pass std::nullopt for shadow_name to use the thing's classic shadow.
    /// If response is non-null, response_mem must be large enough to hold the
    /// decoded result.
    /// Requires aws.greengrass#UpdateThingShadow authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-local-shadows.html#ipc-operation-updatethingshadow>
    std::error_code update_thing_shadow(
        std::string_view thing_name,
        std::optional<std::string_view> shadow_name,
        std::string_view payload,
        std::span<std::byte> response_mem = {},
        std::string_view *response = nullptr
    ) noexcept;

    /// Delete the shadow for a thing.
    /// Deletes the shadow document for the specified thing and shadow name.
    /// Pass std::nullopt for shadow_name to use the thing's classic shadow.
    /// Requires aws.greengrass#DeleteThingShadow authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-local-shadows.html#ipc-operation-deletethingshadow>
    std::error_code delete_thing_shadow(
        std::string_view thing_name, std::optional<std::string_view> shadow_name
    ) noexcept;

    /// List named shadows for a thing.
    /// Lists all named shadows for the specified thing, handling pagination
    /// internally.
    /// The callback is invoked once per shadow name.
    /// Requires aws.greengrass#ListNamedShadowsForThing authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-local-shadows.html#ipc-operation-listnamedshadowsforthing>
    std::error_code list_named_shadows_for_thing(
        std::string_view thing_name,
        void (*callback)(void *ctx, std::string_view shadow_name),
        void *ctx = nullptr
    ) noexcept;

    /// Subscribe to component configuration updates.
    /// Receives notifications when configuration changes for the specified key
    /// path.
    /// Pass std::nullopt for component_name to use the current component.
    /// Requires aws.greengrass#SubscribeToConfigurationUpdate authorization.
    /// See:
    /// <https://docs.aws.amazon.com/greengrass/v2/developerguide/ipc-component-configuration.html#ipc-operation-subscribetoconfigurationupdate>
    std::error_code subscribe_to_configuration_update(
        std::span<const Buffer> key_path,
        std::optional<std::string_view> component_name,
        ConfigurationUpdateCallback &callback,
        Subscription *handle = nullptr
    ) noexcept;
};

}

#endif
