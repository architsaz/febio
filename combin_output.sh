file_name="output.txt"
study_name="pst.2"
output_file="combin_output_pst.2.txt"
if [ -f "$output_file" ]; then
    echo "ERROR: file $output_file exist."
    exit 1
fi
found_files=$(find /dagon1/achitsaz/FEBio/runfebio/*/$study_name/* -type f -name $file_name)
# Count how many files are found
file_count=$(echo "$found_files" | wc -l)

first_file=true

if [ -z "$found_files" ]; then
    echo "No file named $file_name found."
else
    echo "File(s) found: $file_count"
    #echo "$found_files"
    # Append the contents of each found file to the output file
    for file in $found_files; do
        if [ "$first_file" = true ]; then
            cat "$file" >> "$output_file"
            first_file=false
        else
            tail -n +2 "$file" >> "$output_file"
        fi
    done
fi

mean_meanSmax_aneu=$(grep "mean" $output_file | awk '{ if ($4 != 0) { sum += $4; count++ } } END { if (count > 0) print sum / count; }')
mean_meanSmax_red=$(grep "mean" $output_file | awk '{ if ($4 != 0) { sum += $5; count++ } } END { if (count > 0) print sum / count; }')
mean_meanSmax_wht=$(grep "mean" $output_file | awk '{ if ($4 != 0) { sum += $7; count++ } } END { if (count > 0) print sum / count; }')
mean_meanSmax_dom=$(grep "mean" $output_file | awk '{ if ($4 != 0) { sum += $8; count++ } } END { if (count > 0) print sum / count; }')
mean_meanSmax_bod=$(grep "mean" $output_file | awk '{ if ($4 != 0) { sum += $9; count++ } } END { if (count > 0) print sum / count; }')
mean_meanSmax_nek=$(grep "mean" $output_file | awk '{ if ($4 != 0) { sum += $10; count++ } } END { if (count > 0) print sum / count; }')
mean_meanSmax_part=$(grep "mean" $output_file | awk '{ if ($4 != 0) { sum += $11; count++ } } END { if (count > 0) print sum / count; }')
mean_meanSmax_pres=$(grep "mean" $output_file | awk '{ if ($4 != 0) { sum += $12; count++ } } END { if (count > 0) print sum / count; }')

mean_stddevSmax_aneu=$(grep "stddev" $output_file | awk '{ if ($4 != 0) { sum += $4; count++ } } END { if (count > 0) print sum / count; }')
mean_stddevSmax_red=$(grep "stddev" $output_file | awk '{ if ($4 != 0) { sum += $5; count++ } } END { if (count > 0) print sum / count; }')
mean_stddevSmax_wht=$(grep "stddev" $output_file | awk '{ if ($4 != 0) { sum += $7; count++ } } END { if (count > 0) print sum / count; }')
mean_stddevSmax_dom=$(grep "stddev" $output_file | awk '{ if ($4 != 0) { sum += $8; count++ } } END { if (count > 0) print sum / count; }')
mean_stddevSmax_bod=$(grep "stddev" $output_file | awk '{ if ($4 != 0) { sum += $9; count++ } } END { if (count > 0) print sum / count; }')
mean_stddevSmax_nek=$(grep "stddev" $output_file | awk '{ if ($4 != 0) { sum += $10; count++ } } END { if (count > 0) print sum / count; }')
mean_stddevSmax_part=$(grep "stddev" $output_file | awk '{ if ($4 != 0) { sum += $11; count++ } } END { if (count > 0) print sum / count; }')
mean_stddevSmax_pres=$(grep "stddev" $output_file | awk '{ if ($4 != 0) { sum += $12; count++ } } END { if (count > 0) print sum / count; }')


echo -e "Allcases\tstudy\tmean\t$mean_meanSmax_aneu\t$mean_meanSmax_red\t0\t$mean_meanSmax_wht\t$mean_meanSmax_dom\t$mean_meanSmax_bod\t$mean_meanSmax_nek\t$mean_meanSmax_part\t$mean_meanSmax_pres" >> "$output_file"
echo -e "Allcases\tstudy\tstddev\t$mean_stddevSmax_aneu\t$mean_stddevSmax_red\t0\t$mean_stddevSmax_wht\t$mean_stddevSmax_dom\t$mean_stddevSmax_bod\t$mean_stddevSmax_nek\t$mean_stddevSmax_part\t$mean_stddevSmax_pres" >> "$output_file"