#!/bin/bash

export BOARD="zybo"

# Mise en place de l'arborescence
export ROOT_DIR=$(pwd)

df . | grep /dev > /dev/null
if [[ $? -eq 1 ]] ; then
	echo -e "\e[31mPlease use a local drive\e[0m"
	df | grep /dev/
	return 1
fi


export PATH=$ROOT_DIR/.bin/bin:$PATH

# Choix de la version de Vivado
# export VIVADO_ROOT=/softslin/vivado_21.1/Vivado/2021.1
export VIVADO_ROOT=/softslin/vivado_15.1/Vivado/2015.1
source $VIVADO_ROOT/settings64.sh
export VIVADO_LIBS=$VIVADO_ROOT/data

# Mise en place de Vivado 15 (CIME Nanotech)
#source /softslin/vivado_15.1/Vivado/2015.1/settings64.sh

# Mise en place de Vivado 17 (CIME Nanotech)
#source /softslin/vivado_17.1/Vivado/2017.1/settings64.sh
#source /softslin/vivado_17.1/SDK/2017.1/settings64.sh
#export VIVADO_LIBS=/softslin/vivado_17.1/Vivado/2017.1/data/verilog/src

if [ -n "${LM_LICENSE_FILE}" ] ; then
    export LM_LICENSE_FILE="${LM_LICENSE_FILE}:2110@cimekey1"
else
    export LM_LICENSE_FILE="2110@cimekey1"
fi

if [ -n "${LM_LICENSE_FILE}" ] ; then
    export LM_LICENSE_FILE="${LM_LICENSE_FILE}:/rech/cimel/rollandl/.Xilinx/license_pr_cimepc86.lic"
else
    export LM_LICENSE_FILE="2110@cimekey1:/rech/cimel/rollandl/.Xilinx/license_pr_cimepc86.lic"
fi

# Mise en place de QuestaSim

#
# MENTOR GRAPHICS QUESTASIM 10.7c
#

#export QUESTA_HOME=/softslin/questa_core_prime_10_7c/questasim
export QUESTA_HOME=/softslin/questa_core_prime_2024_2_2/questasim
export PATH="$PATH:$QUESTA_HOME/`$QUESTA_HOME/vco`"


if [ -n "${LM_LICENSE_FILE}" ] ; then
    export LM_LICENSE_FILE="${LM_LICENSE_FILE}:1718@cimekey1"
else
    export LM_LICENSE_FILE="1718@cimekey1"
fi

#export MODELSIM="${HOME}/demo_modelsim/modelsim.ini"

#
# MENTOR GRAPHICS Leonardo 2017a session
#

export EXEMPLAR=/softslin/leonardo2017a
export PATH="${PATH}:${EXEMPLAR}/bin"

if [ -n "${LM_LICENSE_FILE}" ] ; then
    export LM_LICENSE_FILE="${LM_LICENSE_FILE}:1718@cimekey1"
else
    export LM_LICENSE_FILE="1718@cimekey1"
fi


cd $ROOT_DIR

# Synchronisation des ips
if [ ! -d $ROOT_DIR/ips/riscv ] ; then
	echo "Updating IPs"
    exec ./update-ips.py
    exec ./generate-scripts.py
fi	

# Installation du compilateur si manquant
if [ ! -d $ROOT_DIR/.bin/bin ] ; then
	echo "Extracting RI5CY Toolchain"
	mkdir -p $ROOT_DIR/.bin
	tar -xzvf $ROOT_DIR/tools/ri5cy_toolchain.tar.gz -C .bin 
fi

# Installation des bibliothèques Xilinx
 if [ ! -d $ROOT_DIR/vsim/xilinx_libs ] ; then
    echo "Extracting Xilinx Libraries"
    mkdir -p $ROOT_DIR/vsim
    tar -xzvf $ROOT_DIR/tools/xilinx_libs.tar.gz -C $ROOT_DIR/vsim
fi

if [ ! -d $ROOT_DIR/simu ] ; then 
    mkdir -p $ROOT_DIR/simu
    cp $ROOT_DIR/tools/cmake_configure.riscv.gcc.sh simu
fi
