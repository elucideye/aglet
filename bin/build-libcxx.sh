#!/bin/bash

TOOLCHAIN=libcxx
CONFIG=Release
polly.py --toolchain ${TOOLCHAIN} --verbose --config ${CONFIG} --reconfig --test --fwd AGLET_BUILD_TESTS=ON HUNTER_CONFIGURATION_TYPES=${CONFIG}

