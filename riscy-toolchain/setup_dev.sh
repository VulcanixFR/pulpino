#!/bin/bash

# This script sets-up the ri5cy toolchain compilation workflow
THIS_DIR=$(pwd)
ARCHIVE=$THIS_DIR/riscv_tools_delta.tar.gz
OUT=$THIS_DIR/ri5cy_toolchain.tar.gz

# Making the necessary tar.gz file
cd $THIS_DIR/tweaks
tar -czvf $ARCHIVE binutils gcc newlib src
cd $THIS_DIR

# Cloning pulpino's repo
if [ ! -d $THIS_DIR/ri5cy_gnu_toolchain ] ; then
    git clone https://github.com/pulp-platform/ri5cy_gnu_toolchain.git
fi
cd $THIS_DIR/ri5cy_gnu_toolchain
cp $ARCHIVE .

# Compiling the toolchain
make

# Linking the install folder to .bin 
BIN_FOLDER=$THIS_DIR/../.bin
if [ ! -L $BIN_FOLDER ]; then
    if [ -d $BIN_FOLDER ]; then mv $BIN_FOLDER $THIS_DIR/../.bin.bak; fi
    ln -s $THIS_DIR/ri5cy_gnu_toolchain/install $BIN_FOLDER 
fi
