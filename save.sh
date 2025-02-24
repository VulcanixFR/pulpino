#!/bin/bash
source set_env.sh

PATCH=$ROOT_DIR/riscv.patch

# Check if the patch file exists before attempting to remove it
if [ -f "$PATCH" ]; then
    rm "$PATCH"
else
    echo "Warning: Patch file does not exist: $PATCH"
fi

# Check if the directory exists before attempting to change into it
if [ -d "$ROOT_DIR/ips/riscv" ]; then
    cd "$ROOT_DIR/ips/riscv" || exit
else
    echo "Error: Directory does not exist: $ROOT_DIR/ips/riscv"
    exit 1
fi

git add .
git diff --cached > $PATCH
git reset .

cd $ROOT_DIR || exit
