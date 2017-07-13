#!/bin/bash

CONFIG=Release
TOOLCHAIN=ios-10-1-arm64-dep-8-0-hid-sections
polly.py --toolchain ${TOOLCHAIN} --verbose --config ${CONFIG} --reconfig --open --test --fwd AGLET_BUILD_TESTS=ON HUNTER_CONFIGURATION_TYPES=${CONFIG}

