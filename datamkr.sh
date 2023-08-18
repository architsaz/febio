#!/bin/bash
 
# save working directory
root=$(pwd)
#echo "$root"

# check access to dagon1
FEBIOdir=/dagon1/jcebral
if [ -d "$FEBIOdir" ]; then
    echo "This machine access to dagon1"
else
	echo "This machine does not have a premission to access dagon1"
	exit 1
fi

#check febio install in this machine or not :
if ! command -v febio3 &> /dev/null
then
    echo "febio3 could not be found"
    exit 1
fi

if ! command -v febio4 &> /dev/null
then
    echo "febio4 could not be found"
    exit 1
fi
# create or check required directories in working directory : febio/script/temp febio/script/src febio/output
FEBIOdir=febio
if [ -d "$FEBIOdir" ]; then
    echo "$FEBIOdir exist."
else
	mkdir $FEBIOdir
fi
FEBIOdir=febio/script
if [ -d "$FEBIOdir" ]; then
    echo "$FEBIOdir exist."
else
	mkdir $FEBIOdir
fi
FEBIOdir=febio/script/temp
if [ -d "$FEBIOdir" ]; then
    echo "$FEBIOdir exist."
else
	mkdir $FEBIOdir
fi
FEBIOdir=febio/script/src
if [ -d "$FEBIOdir" ]; then
    echo "$FEBIOdir exist."
else
	mkdir $FEBIOdir
fi
FEBIOdir=febio/output
if [ -d "$FEBIOdir" ]; then
    echo "$FEBIOdir exist."
else
	mkdir $FEBIOdir
fi

FEBIOdir=febio/output/error_report
if [ -d "$FEBIOdir" ]; then
    echo "$FEBIOdir exist."
else
	mkdir $FEBIOdir
fi
# copy or check exist febio script
FEBIOscr=febio/script/src/inputmkr 
if [ -f "$FEBIOscr" ]; then
    echo "$FEBIOscr exist."
else
	cp /dagon1/achitsaz/scripts/febio/inputmkr $FEBIOscr
fi

FEBIOscr=febio/script/src/febmkr 
if [ -f "$FEBIOscr" ]; then
    echo "$FEBIOscr exist."
else
	cp /dagon1/achitsaz/scripts/febio/febmkr $FEBIOscr
fi

# Finding files needed for this project
cd /dagon1/jcebral/region/R01/wall/
FILE=$(ls *.wall) 

# back to working directory
cd $root

# Run C program to copy and past required files for each case in properd directories
cd febio/script/src 

for eachfile in $FILE
do
   casename=$(basename $eachfile .wall)
   ./inputmkr $casename 2>../../output/error_report/$casename.input.txt
done

for eachfile in $FILE
do
   casename=$(basename $eachfile .wall)
#   ./febmkr $casename febio4  2>../../output/error_report/$casename.feb.txt
done

