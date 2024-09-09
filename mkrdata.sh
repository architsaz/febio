#!/bin/sh
dirname=$2
datadir=./$dirname/$1/data
msa1dir=./$dirname/$1/msa.1
msa2dir=./$dirname/$1/msa.2
pst1dir=./$dirname/$1/pst.1
pst2dir=./$dirname/$1/pst.2
if [ ! -d "./$dirname" ]; then
   mkdir ./$dirname
fi
if [ ! -d "./$dirname/$1" ]; then
   mkdir ./$dirname/$1
fi
if [ ! -d $datadir ]; then
   mkdir $datadir
fi
if [ ! -d $msa1dir ]; then
   mkdir $msa1dir
fi
if [ ! -d $msa2dir ]; then
   mkdir $msa2dir
fi
if [ ! -d $pst1dir ]; then
   mkdir $pst1dir
fi
if [ ! -d $pst2dir ]; then
   mkdir $pst2dir
fi
# find .labels_srf.zfem
path=$(find "/dagon1/jcebral/aneuDB/" -maxdepth 3 -name $1)
cp $path/etc/labels_srf.zfem.gz $datadir
# find .flds.zfem
path=$(find "/dagon1/jcebral/region/R01/wall" -maxdepth 1 -name "$1.flds.zfem")
cp $path $datadir
# find .wall 
path=$(find "/dagon1/jcebral/region/R01/wall" -maxdepth 1 -name "$1.wall")
cp $path $datadir
# unzip files
gzip -d -f  $datadir/*.gz
# cp input file 
cp /dagon1/achitsaz/FEBio/scripts/input_homo.txt $msa1dir/input.txt

cp /dagon1/achitsaz/FEBio/scripts/input_hete.txt $msa2dir/input.txt
# cp run file 
cp /dagon1/achitsaz/FEBio/scripts/run.sh $msa1dir/
cp /dagon1/achitsaz/FEBio/scripts/run.sh $msa2dir/
