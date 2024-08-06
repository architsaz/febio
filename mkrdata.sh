#!/bin/sh
mkdir ./data/$1
# find .labels_srf.zfem
path=$(find "/dagon1/jcebral/aneuDB/" -maxdepth 3 -name $1)
cp $path/etc/labels_srf.zfem.gz ./data/$1
# find .flds.zfem
path=$(find "/dagon1/jcebral/region/R01/wall" -maxdepth 1 -name "$1.flds.zfem")
cp $path ./data/$1/
# find .wall 
path=$(find "/dagon1/jcebral/region/R01/wall" -maxdepth 1 -name "$1.wall")
cp $path ./data/$1/
# unzip files
gzip -d ./data/$1/*.gz
# find input.txt file in working directory
path=$(find . -maxdepth 2 -name "input.txt")
cp $path ./data/$1/
