#!/bin/bash

# ANSI color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Compiler settings
COMPILER=clang++
CFLAGS="-Wall -Wextra -std=c++20 -O2"

# Copy the header file to the tests directory and change directory
cp ./invoke_intseq.h tests/invoke_intseq.h
cd tests

# Array of test numbers to compile with
TEST_NUMS=(101 201 202 203 204 205 301 401 501 601)

# Special cases
NON_COMPILING_TESTS=(204 205) # Tests expected not to compile
NO_MAIN_TESTS=(202)           # Tests without a main function

# Compile each .cc file with different TEST_NUMs
for file in *.cc; do
    base_name=$(basename "$file" .cc)

    for test_num in "${TEST_NUMS[@]}"; do
        # Skip certain tests for files other than invoke_intseq_test.cc
        if [ "$base_name" != "invoke_intseq_test" ] && [[ " ${NON_COMPILING_TESTS[@]} " =~ " ${test_num} " ]]; then
            continue
        fi

        if [ "$base_name" == "invoke_intseq_test_extern" ] && [[ " ${test_num} " != 202 ]]; then
            continue
        fi

        # Compile (linker errors are suppressed for tests without a main function)
        if [[ " ${NO_MAIN_TESTS[@]} " =~ " ${test_num} " ]] || [[ " ${NON_COMPILING_TESTS[@]} " =~ " ${test_num} " ]]; then
            $COMPILER $CFLAGS -DTEST_NUM=$test_num -c -o "${base_name}_${test_num}.o" "$file" 2>/dev/null
        else
            $COMPILER $CFLAGS -DTEST_NUM=$test_num -o "${base_name}_${test_num}" "$file"
        fi

        # Check if compilation was successful
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}Compilation of $file with TEST_NUM=$test_num succeeded${NC}"
        else
            if [[ " ${NON_COMPILING_TESTS[@]} " =~ " ${test_num} " ]]; then
                echo -e "${YELLOW}Failure (as expected) in compiling $file with TEST_NUM=$test_num${NC}"
            elif [[ " ${NO_MAIN_TESTS[@]} " =~ " ${test_num} " ]]; then
                echo -e "${YELLOW}Skipping linking for $file with TEST_NUM=$test_num (no main function)${NC}"
            else
                echo -e "${RED}Unexpected compilation failure for $file with TEST_NUM=$test_num${NC}"
            fi
        fi
    done
done

# Clean up by removing the copied header file, object files, and executables
rm invoke_intseq.h
rm *.o


#!/bin/bash

# Remove executables, assuming they are named as "${base_name}_${test_num}"
for file in *.cc; do
    base_name=$(basename "$file" .cc)
    for test_num in "${TEST_NUMS[@]}"; do
        if [ -f "${base_name}_${test_num}" ]; then
            rm "${base_name}_${test_num}"
        fi
    done
done

