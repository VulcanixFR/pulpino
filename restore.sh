#!/bin/bash

PATCH=$ROOT_DIR/riscv.patch

cd $ROOT_DIR/ips/riscv
git reset --hard f5475852ec3ab6895ef5da78f3ceb70fbacbd091
git clean -f
git apply $PATCH

cd $ROOT_DIR
