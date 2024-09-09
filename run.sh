#!/bin/sh
#sed -i 's/\r//' run.sh
# Checking provided argument
if [ "$#" -ne 4 ]; then
    echo " # of arguments: $#"
    echo "- Required: four arguments"
    echo "- Usage: <casename> <option> <start_step> <last_step>"
    echo "- Available options: mknjmask|highcurv|nocorr|corrbynj|enhance"
    exit 1
fi

# Get option
option=$2

#OpenMP
ncpu=12
export OMP_NUM_THREADS=$ncpu
export KMP_STACKSIZE=1g
export OMP_DYNAMIC=FALSE

#Limits
ulimit -s unlimited
ulimit -v unlimited

#
if [ "$(hostname)" = "L-P4000-N05218" ]; then
    febmkr_EXE="../../../build/febmkr_exec"
    febio4_EXE="/opt/FEBioStudio/bin/febio4"
else
    febmkr_EXE="/dagon1/achitsaz/FEBio/scripts/build/febmkr_exec"
    febio4_EXE="/dagon1/achitsaz/app/FEBioStudio/bin/febio4"
fi
if [ ! -e ${febmkr_EXE} ]; then
  echo "Could not find ${febmkr_EXE}"
  exit 1
fi
if [ ! -e ${febio4_EXE} ]; then
  echo "Could not find ${febio4_EXE}"
  exit 1
fi
#Echo
echo "using ${febio4_EXE}"
echo "using ${febmkr_EXE}"
env | grep OMP_NUM_THREADS

#Info
INFO=run.info
if [ -e ${INFO} -o -e ${INFO}.gz ]; then
  echo "Error: ${INFO} already exists!"
  exit 1
fi
echo "host=`hostname`" > ${INFO}
echo "wdir=`pwd`" >> ${INFO}
echo `env | grep OMP_NUM_THREADS` >> ${INFO}
PID=$!
echo "running in background ... [${PID}]"
echo "pid=${PID}" >> ${INFO}
echo "UniqueID=$(date +%s)-$1-$2" >>${INFO}
#Run
# Perform actions based on the argument
case $2 in
    nocorr|highcurv)
        $febmkr_EXE $1 $2 0  > out 2>&1 
        $febio4_EXE -i pres_0.feb >> out 2>&1 
        checkresult=$(grep "N O R M A L" "pres_0.log")
        if [ "$checkresult" = " N O R M A L   T E R M I N A T I O N" ];then
            echo "$checkresult without Young's Modulus modification."
            break
        fi
        ;;
    mknjmask)
        $febmkr_EXE $1 $2 $3 > out 2>&1 
        ;;    
    corrbynj|enhance)
        for i in $(seq $3 $4); do
	    echo "-starting $i loop of $2 option..."
            if [ $i -eq 0 ]; then
                $febmkr_EXE $1 nocorr 0 > out 2>&1 
                $febio4_EXE -i pres_0.feb >> out 2>&1 
            else
                $febmkr_EXE $1 $2 $i >> out 2>&1 
                $febio4_EXE -i pres_$i.feb >> out 2>&1 
            fi
            checkresult=$(grep "N O R M A L" "pres_$i.log")
            if [ "$checkresult" = " N O R M A L   T E R M I N A T I O N" ];then
                echo "$checkresult at $i loop with ($2) Young's Modulus modification."
                break
            fi
        done
        ;;
    *)
        echo "Invalid argument: $2"
        echo "Usage: $0 {mknjmask|nocorr|corrbynj|enhance}"
        exit 1
        ;;
esac



