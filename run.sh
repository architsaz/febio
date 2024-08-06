#!/bin/sh

#OpenMP
ncpu=18
export OMP_NUM_THREADS=$ncpu
export KMP_STACKSIZE=1g
export OMP_DYNAMIC=FALSE

#Limits
ulimit -s unlimited
ulimit -v unlimited

#executable
EXE=/serapis1/jcebral/zsft/abcell-1.0/build/opt/bin/ang
if [ ! -e ${EXE} ]; then
  echo "Could not find ${EXE}"
  exit 1
fi

#Echo
echo "using ${EXE}"
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

#Run
ECHO=
if [ "x$1" = "x-i" -o "x$1" = "x--interactive" ]; then
  ${ECHO} $EXE
else
  if [ -z ${ECHO} ]; then
    nohup $EXE > out 2>&1 &
  fi
  PID=$!
  echo "running in background ... [${PID}]"
  echo "pid=${PID}" >> ${INFO}
fi
