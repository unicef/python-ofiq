#!/bin/bash
set -e

echo "=== Building OFIQ from source ==="

if command -v yum &> /dev/null; then
    yum install -y git
elif command -v apt-get &> /dev/null; then
    apt-get update && apt-get install -y git
fi

OFIQ_DIR="/opt/OFIQ-Project"
if [ ! -d "$OFIQ_DIR" ]; then
    echo "Cloning OFIQ v1.1.2..."
    git clone --depth 1 --branch v1.1.2 https://github.com/BSI-OFIQ/OFIQ-Project.git "$OFIQ_DIR"
fi

cd "$OFIQ_DIR"

if [ -x "/opt/python/cp311-cp311/bin/python" ]; then
    PYTHON_BIN="/opt/python/cp311-cp311/bin/python"
elif command -v python3 &> /dev/null; then
    PYTHON_BIN="python3"
else
    echo "ERROR: No Python found"
    exit 1
fi

echo "Using Python: $PYTHON_BIN"

$PYTHON_BIN -m venv /tmp/ofiq_build_env
source /tmp/ofiq_build_env/bin/activate
pip install --upgrade pip
pip install conan==2.18.1

echo "Building OFIQ..."
cd scripts
bash build.sh

echo "OFIQ build complete!"

INSTALL_DIR="install_x86_64_linux"
if [ -f "$OFIQ_DIR/$INSTALL_DIR/Release/lib/libofiq_lib.so" ]; then
    echo "OFIQ library found — build successful"
else
    echo "ERROR: OFIQ library not found"
    ls -la "$OFIQ_DIR/$INSTALL_DIR/Release/lib/" || true
    exit 1
fi
