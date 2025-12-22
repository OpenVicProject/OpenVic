#!/usr/bin/env bash

echo "\n\nFormatting openvic-extension with clang-format:\n"
find ./extension/src/ -iname *.hpp -o -iname *.cpp | xargs clang-format --verbose -i

if [ -d ./extension/deps/openvic-simulation ]; then
    cd ./extension/deps/openvic-simulation
    if [ -f ./clang-format.sh ]; then
        ./clang-format.sh
    fi
fi

exit 0
