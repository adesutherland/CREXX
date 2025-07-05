rxc -i $CREXX_HOME/lib/rxfns/ crexx
rxas crexx
rxcpack crexx $CREXX_HOME/lib/rxfns/library
./ccomp crexx.c
cp a.out crexx
cp crexx $CREXX_HOME/bin
