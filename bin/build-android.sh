#!/bin/bash

#TOOLCHAIN=android-ndk-r16b-api-21-armeabi-clang-libcxx
#TOOLCHAIN=android-ndk-r10e-api-19-armeabi-v7a-neon
TOOLCHAIN=android-ndk-r16b-api-24-armeabi-v7a-neon-clang-libcxx14
CONFIG=Release

AGLET_OPENGL_ES3="OFF"
if [[ "$#" -gt 0 ]] ; then
    AGLET_OPENGL_ES3="ON"
fi

ARGS=(
    AGLET_BUILD_TESTS=ON
    AGLET_OPENGL_ES3=${AGLET_OPENGL_ES3}
    HUNTER_CONFIGURATION_TYPES=${CONFIG}
    GAUZE_ANDROID_USE_EMULATOR=NO
)

polly.py --toolchain ${TOOLCHAIN} --verbose --config ${CONFIG} --reconfig --test --fwd ${ARGS[@]}
