// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

use crate::c;

impl From<&str> for c::GgBuffer {
    fn from(s: &str) -> Self {
        Self {
            data: s.as_ptr().cast_mut(),
            len: s.len(),
        }
    }
}

impl From<&[u8]> for c::GgBuffer {
    fn from(s: &[u8]) -> Self {
        Self {
            data: s.as_ptr().cast_mut(),
            len: s.len(),
        }
    }
}
