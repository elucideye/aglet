#!/bin/bash

TOOLCHAIN=xcode
CONFIG=Release

AGLET_OPENGL_ES3="OFF"
if [[ "$#" -gt 0 ]] ; then
    AGLET_OPENGL_ES3="ON"
fi

ARGS=(
    AGLET_BUILD_TESTS=ON
    AGLET_OPENGL_ES3=${AGLET_OPENGL_ES3} # not really, but enable pbo etc
    HUNTER_CONFIGURATION_TYPES=${CONFIG}
)
polly.py --toolchain ${TOOLCHAIN} --verbose --config ${CONFIG} --reconfig --test --fwd ${ARGS[@]}


