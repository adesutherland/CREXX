rxc -i /Users/rvjansen/apps/crexx_release/lib/rxfns/ crexx
rxas crexx
rxcpack crexx ~/apps/crexx_release/lib/rxfns/library
./ccomp crexx.c
cp a.out crexx
