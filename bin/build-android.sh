#!/bin/bash

TOOLCHAIN=android-ndk-r10e-api-19-armeabi-v7a-neon
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
