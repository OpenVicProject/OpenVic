#!/bin/bash

# $1 = environment name
# $2 = value to set environment
set_var() {
    if [[ -z ${!1+set} ]]; then
        export $1=$2
    fi
}

# $1 = left side
# $2 = right side
# returns $1 and $2 joined together with a single '/''
join_path() {
    result=$1
    if [[ $result == *"/" && $2 == "/"* ]]; then
        result=${result%/}
    fi
    if [[ $result != *"/" && $2 != "/"* ]]; then
        result="$result/"
    fi
    echo "$result$2"
}

# $1 = environment name
set_github_var() {
    if [ "${!1}" == "" ]; then
        echo "::error::$1 environment variable has not been set."
    elif ! curl -o /dev/null --head --silent --fail "${!1}"; then
        echo "::error::${!1} does not exist."
    else
        echo "$1=${!1}" >> $GITHUB_ENV
    fi
}

if [[ $GODOT_BASE_DOWNLOAD_URL == *"downloads.tuxfamily.org/godotengine"* ]]; then
    if [[ $GODOT_BASE_DOWNLOAD_URL != *"${GODOT_VERSION}"* ]]; then
        GODOT_BASE_DOWNLOAD_URL="$(join_path ${GODOT_BASE_DOWNLOAD_URL} ${GODOT_VERSION})"
    fi
    if [[ $GODOT_VERSION_TYPE != "stable" ]]; then
        GODOT_BASE_DOWNLOAD_URL="$(join_path ${GODOT_BASE_DOWNLOAD_URL} ${GODOT_VERSION_TYPE})"
    fi
    set_var GODOT_LINUX_URL "$(join_path ${GODOT_BASE_DOWNLOAD_URL} "Godot_v${GODOT_VERSION}-${GODOT_VERSION_TYPE}_linux.x86_64.zip")"
    set_var GODOT_TEMPLATE_URL "$(join_path ${GODOT_BASE_DOWNLOAD_URL} "Godot_v${GODOT_VERSION}-${GODOT_VERSION_TYPE}_export_templates.tpz")"
elif [[ $GODOT_BASE_DOWNLOAD_URL == *"github.com/godotengine"* ]]; then
    if [[ $GODOT_BASE_DOWNLOAD_URL != *"/releases"* ]]; then
        GODOT_BASE_DOWNLOAD_URL=$(join_path $GODOT_BASE_DOWNLOAD_URL "releases")
    fi
    if [[ $GODOT_BASE_DOWNLOAD_URL != *"/download"* ]]; then
        GODOT_BASE_DOWNLOAD_URL=$(join_path $GODOT_BASE_DOWNLOAD_URL "download")
    fi
    set_var GODOT_LINUX_URL "$(join_path ${GODOT_BASE_DOWNLOAD_URL} "${GODOT_VERSION}-${GODOT_VERSION_TYPE}/Godot_v${GODOT_VERSION}-${GODOT_VERSION_TYPE}_linux.x86_64.zip")"
    set_var GODOT_TEMPLATE_URL "$(join_path ${GODOT_BASE_DOWNLOAD_URL} "${GODOT_VERSION}-${GODOT_VERSION_TYPE}/Godot_v${GODOT_VERSION}-${GODOT_VERSION_TYPE}_export_templates.tpz")"
fi

set_github_var GODOT_LINUX_URL
set_github_var GODOT_TEMPLATE_URL