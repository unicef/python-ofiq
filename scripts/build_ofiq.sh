#!/bin/bash
set -e

echo "=== Building OFIQ from source ==="

OFIQ_VERSION="v1.1.2"

UNAME_S="$(uname -s)"
case "$UNAME_S" in
    Linux*)
        PLATFORM=linux
        OFIQ_DIR="/opt/OFIQ-Project"
        ;;
    Darwin*)
        PLATFORM=macos
        OFIQ_DIR="${HOME}/OFIQ-Project"
        ;;
    *)
        echo "ERROR: Unsupported platform: $UNAME_S"
        exit 1
        ;;
esac

echo "Platform: $PLATFORM"
echo "OFIQ directory: $OFIQ_DIR"

if [ "$PLATFORM" = "linux" ]; then
    if command -v yum &> /dev/null; then
        yum install -y git
    elif command -v apt-get &> /dev/null; then
        apt-get update && apt-get install -y git
    fi
else
    if ! command -v git &> /dev/null; then
        echo "ERROR: git is required on macOS (install via 'brew install git' or Xcode CLT)"
        exit 1
    fi
    if ! command -v cmake &> /dev/null; then
        echo "ERROR: cmake is required on macOS (install via 'brew install cmake')"
        exit 1
    fi
fi

if [ ! -d "$OFIQ_DIR" ]; then
    echo "Cloning OFIQ $OFIQ_VERSION..."
    mkdir -p "$(dirname "$OFIQ_DIR")"
    git clone --depth 1 --branch "$OFIQ_VERSION" \
        https://github.com/BSI-OFIQ/OFIQ-Project.git "$OFIQ_DIR"
fi

cd "$OFIQ_DIR"

if [ "$PLATFORM" = "linux" ] && [ -x "/opt/python/cp311-cp311/bin/python" ]; then
    PYTHON_BIN="/opt/python/cp311-cp311/bin/python"
elif command -v python3 &> /dev/null; then
    PYTHON_BIN="python3"
else
    echo "ERROR: No Python found"
    exit 1
fi

echo "Using Python: $PYTHON_BIN"

BUILD_VENV="/tmp/ofiq_build_env"
"$PYTHON_BIN" -m venv "$BUILD_VENV"
# shellcheck disable=SC1091
source "$BUILD_VENV/bin/activate"
pip install --upgrade pip
pip install conan==2.18.1

echo "Building OFIQ..."
cd scripts
if [ "$PLATFORM" = "macos" ]; then
    bash build.sh --os macos
    INSTALL_DIR_NAME="install_arm64_mac"
    LIB_FILENAME="libofiq_lib.dylib"
else
    bash build.sh
    INSTALL_DIR_NAME="install_x86_64_linux"
    LIB_FILENAME="libofiq_lib.so"
fi

deactivate

LIB_FILE="$OFIQ_DIR/$INSTALL_DIR_NAME/Release/lib/$LIB_FILENAME"
if [ -f "$LIB_FILE" ]; then
    echo "OFIQ library found at $LIB_FILE — build successful"
else
    echo "ERROR: OFIQ library not found at $LIB_FILE"
    ls -la "$(dirname "$LIB_FILE")" || true
    exit 1
fi

echo "=== OFIQ build complete ==="
