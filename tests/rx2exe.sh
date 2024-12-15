crexx_home=/Users/adrian/CLionProjects/CREXX/cmake-build-debug/
$crexx_home/compiler/rxc -i /Users/adrian/CLionProjects/CREXX/cmake-build-debug/lib/rxfnsb $1
$crexx_home/assembler/rxas $1
$crexx_home/cpacker/rxcpack $1 /Users/adrian/CLionProjects/CREXX/cmake-build-debug/lib/rxfnsb/library
gcc -o $1 \
    -lrxvml -lmachine -lavl_tree -lplatform -lm  \
    -L$crexx_home/interpreter \
    -L$crexx_home/machine \
    -L$crexx_home/avl_tree \
    -L$crexx_home/platform \
    $1.c
upx $1
