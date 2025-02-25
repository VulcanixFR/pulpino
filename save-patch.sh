#!/bin/bash
if [ $# -eq 0 ]; then
    echo "./save-patch.sh <patch_name>"
    exit 1
fi

PATCH=$ROOT_DIR/patches/$1.patch

# Check if the patch file exists before attempting to remove it
if [ -f "$PATCH" ]; then
    rm "$PATCH"
else
    echo "Warning: Patch file does not exist: $PATCH"
fi

# Check if the directory exists before attempting to change into it
if [ -d "$ROOT_DIR/ips/riscv" ]; then
    cd "$ROOT_DIR/ips/riscv"
else
    echo "Error: Directory does not exist: $ROOT_DIR/ips/riscv"
    exit 1
fi

git add . --all
git diff --cached > $PATCH
git reset .

cd $ROOT_DIR
