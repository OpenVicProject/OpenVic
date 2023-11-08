#!/usr/bin/env bash

echo "\n\nFormatting openvic-extension with astyle:\n"
astyle --options=.astylesrc --recursive ./extension/src/*.?pp

if [ -d ./extension/deps/openvic-simulation ]; then
    cd ./extension/deps/openvic-simulation
    if [ -f ./astyle.sh ]; then
        ./astyle.sh
    fi
fi

exit 0
