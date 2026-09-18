#!/usr/bin/env bash
set -euo pipefail
: "${ANDROID_SDK_ROOT:?Set ANDROID_SDK_ROOT}"
: "${JAVA_HOME:?Set JAVA_HOME}"
: "${JYMOD_KEYSTORE:?Set JYMOD_KEYSTORE}"
: "${JYMOD_PASSWORD_FILE:?Set JYMOD_PASSWORD_FILE}"
if [ "$#" -ne 2 ]; then echo "Usage: $0 unsigned.apk signed.apk" >&2; exit 2; fi
build_tools="$ANDROID_SDK_ROOT/build-tools/${ANDROID_BUILD_TOOLS_VERSION:-34.0.0}"
temp_dir=$(mktemp -d)
trap 'rm -rf "$temp_dir"' EXIT
"$build_tools/zipalign" -f -p 4 "$1" "$temp_dir/aligned.apk"
"$build_tools/apksigner" sign --ks "$JYMOD_KEYSTORE" --ks-key-alias "${JYMOD_KEY_ALIAS:-jymod-release}" --ks-pass "file:$JYMOD_PASSWORD_FILE" --out "$2" "$temp_dir/aligned.apk"
"$build_tools/apksigner" verify --verbose --print-certs "$2"
"$build_tools/zipalign" -c -p 4 "$2"
