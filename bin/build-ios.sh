#!/bin/bash


TOOLCHAIN=ios-10-1-arm64-dep-8-0-hid-sections
CONFIG=Release

ARGS=(
    AGLET_BUILD_TESTS=ON
    AGLET_OPENGL_ES3=ON
    HUNTER_CONFIGURATION_TYPES=${CONFIG}
)

polly.py --toolchain ${TOOLCHAIN} --verbose --config ${CONFIG} --reconfig --open --fwd ${ARGS[@]}

