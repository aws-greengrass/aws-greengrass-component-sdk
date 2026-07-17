# Release Notes v1.0.5

## New Features

- The CreateLocalDeployment IPC command is now supported.

## Bug Fixes

- Asserts are now stripped in release modes. This may reduce binary size.
- Fix Rust bindings double free in raw call/subscribe APIs.

# Release Notes v1.0.4

## Bug Fixes

- Notify Nucleus of cleaned up request when a request times out.

# Release Notes v1.0.3

## Bug Fixes

- Ensure error callback is called when subscription request fails.
- Send IPC stream terminations to server when subscription is cleaned up by user
  callback error.

## Rust Improvements

- Enable Rust crate build on older GCC versions.
- Set bindgen clang target argument for Rust crate cross-compilation.

# Release Notes v1.0.2

## Bug Fixes

- Fixed Rust crate build on 32-bit ARM targets.
- Fixed Rust crate build with GCC 12 or earlier.

# Release Notes v1.0.1

## Bug Fixes

- Fixed Rust crate packaging to include C source files required for compilation.

# Release Notes v1.0.0

This is the initial release of `aws-greengrass-component-sdk`. This SDK provides
Greengrass components with APIs in C, C++, and Rust for making Greengrass IPC
calls.
