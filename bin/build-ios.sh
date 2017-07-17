#!/bin/bash


TOOLCHAIN=ios-10-1-arm64-dep-8-0-hid-sections
CONFIG=Release

AGLET_OPENGL_ES3="OFF"
if [[ "$#" -gt 0 ]] ; then
    AGLET_OPENGL_ES3="ON"
fi

ARGS=(
    AGLET_BUILD_TESTS=ON
    AGLET_OPENGL_ES3=${AGLET_OPENGL_ES3}
    HUNTER_CONFIGURATION_TYPES=${CONFIG}
)

polly.py --toolchain ${TOOLCHAIN} --verbose --config ${CONFIG} --reconfig --test --fwd ${ARGS[@]}

