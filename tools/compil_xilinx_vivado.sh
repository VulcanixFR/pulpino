export VIVADO_ROOT=/softslin/vivado_21.1/Vivado/2021.1
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


#Import de modelsim
export MTI_HOME=/softslin/modelsim10_5c/modeltech
export PATH="$PATH:$MTI_HOME/`$MTI_HOME/vco`"

if [ -n "${LM_LICENSE_FILE}" ] ; then
    export LM_LICENSE_FILE="${LM_LICENSE_FILE}:1718@cimekey1"
else
    export LM_LICENSE_FILE="1718@cimekey1"
fi

#Import de Questasim
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
