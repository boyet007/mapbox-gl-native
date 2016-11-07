#!/bin/bash

set -e
set -o pipefail

if [ ${BUILDTYPE} = "Release" ]; then
    ARCH=$(uname -m)
    RENDER=build/linux-${ARCH}/${BUILDTYPE}/mbgl-render

    # Generate a stripped version of the binary
    strip -s -x ${RENDER} -o ${RENDER}-stripped

    mapbox_time "aws binary size logging" \
    aws --region us-east-1 cloudwatch put-metric-data \
        --namespace "Mapbox/GL" \
        --metric-name "BinarySize" \
        --unit "Bytes" \
        --value `stat --printf="%s" ${RENDER}-stripped` \
        --dimensions "Platform=Linux-${_CXX},Arch=${ARCH}"
fi
