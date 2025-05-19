#!/bin/bash

# Description:
# This script sets up a working directory for a given case ID and base directory name.
# It creates necessary subdirectories, finds and copies required data files from
# predefined database locations, unzips files, and places input scripts for execution.

# Usage:
#   ./mkrdara.sh <case_id> <base_dir>
#
# Arguments:
#   <case_id>   : The identifier (e.g. 'a06161.1') of the aneurysm case.
#   <base_dir>  : The base directory where case-specific folders will be created.
#
# Example:
#   ./mkrdara.sh P123456 cases_dir
#
# Help:
#   ./mkrdara.sh --help

# Function to display help message
show_help() {
    grep '^#' "$0" | cut -c 3-
    exit 0
}

# Check for help option
if [ "$1" = "--help" ]; then
    show_help
fi

# Check if both arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Error: You must provide <case_id> and <base_dir> as arguments."
    echo "Use './mkrdata.sh --help' for usage."
    exit 1
fi

case_id="$1"
base_dir="$2"

# Define directories
datadir="$base_dir/$case_id/data"
msa1dir="$base_dir/$case_id/msa.1"
msa2dir="$base_dir/$case_id/msa.2"
pst1dir="$base_dir/$case_id/pst.1"
pst2dir="$base_dir/$case_id/pst.2"
compdir="$base_dir/$case_id/comp.1.2"

# Create directories if they don't exist
for dir in "$base_dir" "$base_dir/$case_id" "$datadir" "$msa1dir" "$msa2dir" "$pst1dir" "$pst2dir" "$compdir"; do
    mkdir -p "$dir"
done

# Find and copy required data files

# Find and copy labels_srf.zfem.gz ---- region mask
label_path=$(find "/dagon1/jcebral/aneuDB/" -maxdepth 3 -type d -name "$case_id")
if [ -z "$label_path" ]; then
    echo "Warning: Case directory '$case_id' not found in aneuDB."
else
    cp "$label_path/etc/labels_srf.zfem.gz" "$datadir/" 2>/dev/null || echo "Warning: labels_srf.zfem.gz not found in $label_path/etc/"
fi

# Copy .flds.zfem file --- used this for geometric data (points, elems)
flds_file=$(find "/dagon1/ykarnam/wall/marks/master_wall" -maxdepth 2 -name "$case_id.wssm.zfem")
if [ -n "$flds_file" ]; then
    cp "$flds_file" "$datadir/$case_id.flds.zfem"
else
    echo "Warning: $case_id.flds.zfem not found."
fi

# Copy wall mask ----- colored mask
wall_file=$(find "/dagon1/ykarnam/wall/marks/master_wall" -maxdepth 2 -name "$case_id.wssm.zfem.labels")
if [ -n "$wall_file" ]; then
    cp "$wall_file" "$datadir/$case_id.wall"
else
    echo "Warning: $case_id.wall not found."
fi

# Unzip any .gz files in the data directory
gzip -d -f "$datadir"/*.gz 2>/dev/null

# Copy input files -- input parameters that need to run febio
cp /dagon1/achitsaz/FEBio/scripts.feb/input_homo.txt "$msa1dir/input.txt"
cp /dagon1/achitsaz/FEBio/scripts.feb/input_hete.txt "$msa2dir/input.txt"

# Copy run.sh to both msa directories --- run bash script
cp /dagon1/achitsaz/FEBio/scripts.feb/run.sh "$msa1dir/"
cp /dagon1/achitsaz/FEBio/scripts.feb/run.sh "$msa2dir/"
