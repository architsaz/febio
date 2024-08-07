for folder in /dagon1/achitsaz/FEBio/data/*; do
  echo "$folder"
  ln -s "$folder" ./data/
done



