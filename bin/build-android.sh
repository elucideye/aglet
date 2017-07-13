#!/bin/bash

CONFIG=Release
TOOLCHAIN=android-ndk-r10e-api-19-armeabi-v7a-neon 
polly.py --toolchain ${TOOLCHAIN} --verbose --config ${CONFIG} --reconfig --open --test --fwd AGLET_BUILD_TESTS=ON HUNTER_CONFIGURATION_TYPES=${CONFIG}
