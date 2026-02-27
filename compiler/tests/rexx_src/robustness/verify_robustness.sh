#!/bin/bash
BIN_DIR="/Users/adrian/CLionProjects/CREXX/cmake-build-debug/bin"
TEST_DIR="/Users/adrian/CLionProjects/CREXX/compiler/tests/rexx_src/robustness"
LIB_BIN="$BIN_DIR/library.rxbin"

run_test() {
    NAME=$1
    LIBS=$2
    echo "--- Testing $NAME ---"
    
    # Compile Library if needed
    if [[ "$NAME" == *"set"* ]]; then
        SET_NUM=$(echo $NAME | cut -d'_' -f1)
        LIB_BASE="${TEST_DIR}/${SET_NUM}_lib"
        $BIN_DIR/rxc -i $TEST_DIR -i $BIN_DIR -o $LIB_BASE ${LIB_BASE}.rexx > /dev/null 2>&1
        $BIN_DIR/rxas -o ${LIB_BASE}.rxbin ${LIB_BASE}.rxas > /dev/null 2>&1
    fi

    # Compile Main
    $BIN_DIR/rxc -i $TEST_DIR -i $BIN_DIR -o ${TEST_DIR}/$NAME ${TEST_DIR}/${NAME}.rexx > /dev/null 2>&1
    $BIN_DIR/rxas -o ${TEST_DIR}/${NAME}.rxbin ${TEST_DIR}/${NAME}.rxas > /dev/null 2>&1
    
    # Run VM
    $BIN_DIR/rxvm ${TEST_DIR}/${NAME}.rxbin $LIBS $LIB_BIN
    echo ""
}

# Set 1: Basic types and procedures
run_test "set1_main" "${TEST_DIR}/set1_lib.rxbin"

# Set 2: Classes and Objects
run_test "set2_main" "${TEST_DIR}/set2_lib.rxbin"

# Set 3: Arrays and Arrays of Objects
run_test "set3_main" "${TEST_DIR}/set3_lib.rxbin"

# Set 4: Nested namespaces and shadowing
run_test "set4_main" "${TEST_DIR}/set4_mid.rxbin ${TEST_DIR}/set4_base.rxbin"

# Set 5: Private vs Public (Expected Compile Error)
echo "--- Testing set5_main (Expected Compile Error) ---"
$BIN_DIR/rxc -i $TEST_DIR -o ${TEST_DIR}/set5_lib ${TEST_DIR}/set5_lib.rexx > /dev/null 2>&1
$BIN_DIR/rxc -i $TEST_DIR -o ${TEST_DIR}/set5_main ${TEST_DIR}/set5_main.rexx 2>&1 | grep "FUNCTION_NOT_FOUND"
