#!/bin/bash

TOOLCHAIN=android-ndk-r10e-api-19-armeabi-v7a-neon
CONFIG=Release

ARGS=(
    AGLET_BUILD_TESTS=ON
    AGLET_OPENGL_ES3=ON
    HUNTER_CONFIGURATION_TYPES=${CONFIG}
)

polly.py --toolchain ${TOOLCHAIN} --verbose --config ${CONFIG} --reconfig --open --test --fwd ${ARGS[@]}
