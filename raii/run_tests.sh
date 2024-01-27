#!/bin/bash

# Define colors
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Check if the '-v' argument is passed to the script
USE_VALGRIND=false
if [ "$1" == "-v" ]; then
    USE_VALGRIND=true
fi

cp stack.h tests/stack.h
# Define an array of test number ranges.
TEST_NUM_RANGES=(101-110 201-202 301-305 401-407 501-513 601-604)

# Define the name of the executable file.
executable_name="stack_test"

# Loop over the array of test number ranges.
for range in "${TEST_NUM_RANGES[@]}"
do
    IFS='-' read -r -a nums <<< "$range"
    start=${nums[0]}
    end=${nums[1]}

    for ((test_num=start; test_num<=end; test_num++))
    do
        # Compile the C++ code with the current test number.
        g++ -Wall -Wextra -O2 -std=c++20 -DTEST_NUM=$test_num tests/stack_test.cc tests/stack_test_external.cc -o $executable_name

        if [ $? -eq 0 ]; then
            # Run the executable with or without Valgrind based on the user's choice.
            if [ "$USE_VALGRIND" = true ]; then
                valgrind --error-exitcode=123 --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --run-cxx-freeres=yes -q ./$executable_name 2> /dev/null
                valgrind_result=$?
            else
                ./$executable_name 2> /dev/null
                valgrind_result=$?
            fi

            # Check the result and print appropriate message.
            if [ $valgrind_result -eq 0 ]; then
                echo -e "Test $test_num: ${GREEN}Ok${NC}"
            elif [ $valgrind_result -eq 123 ]; then
                echo -e "Test $test_num: ${RED}Memory leak or error detected${NC}"
            else
                echo -e "Test $test_num: ${RED}Error${NC}"
            fi
        else
            echo -e "Test $test_num: ${RED}Compilation failed${NC}"
        fi
    done
done

# Cleanup: remove the executable file.
rm -f $executable_name
rm tests/stack.h
