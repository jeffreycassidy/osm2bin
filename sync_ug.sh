HOST=ug70.eecg.utoronto.ca
USER=cassidy

rsync -rizt --progress --files-from=streetsdatabase.files.txt . $USER@$HOST:~/ECE297/golden_solution_2/libstreetsdatabase/src
rsync -rizt --progress --files-from=osm2bin.files.txt . $USER@$HOST:~/ECE297/golden_solution_2/osm_to_bin/src

