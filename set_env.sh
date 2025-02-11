#!/bin/bash

# Mise en place de l'arboréscence
export ROOT_DIR=$(pwd)

df . | grep /dev > /dev/null
if [[ $? -eq 1 ]] ; then
	echo -e "\e[31mPlease use a local drive\e[0m"
	df | grep /dev/
	return 1
fi


export PATH=$ROOT_DIR/.bin/bin:$PATH

# Mise en place de Vivado 15 (CIME Nanotech)
source /softslin/vivado_15.1/Vivado/2015.1/settings64.sh

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

export QUESTA_HOME=/softslin/questa_core_prime_10_7c/questasim
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
if [ ! -d $ROOT_DIR/ipstools ] ; then
	exec ./update-ips.py
fi	

# Installation du compilateur si manquant
if [ ! -d $ROOT_DIR/.bin ] ; then
	echo "Extracting RI5CY Toolchain"
	mkdir $ROOT_DIR/.bin
	tar -xzvf $ROOT_DIR/tools/ri5cy_toolchain.tar.gz --strip-components=1 -C .bin installed_ri5cy/
fi

