#!/bin/bash

PATCH=$ROOT_DIR/riscv.patch

rm $PATCH
cd $ROOT_DIR/ips/riscv
git add .
git diff --cached > $PATCH
git reset .

cd $ROOT_DIR
