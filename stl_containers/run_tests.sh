#!/bin/bash

make
# Path to the executable program
PROGRAM=./parking

# Loop over all input files
for infile in tests/test_*.in; do
    # Extract the base name of the file
    base=${infile%.in}

    # Run the program with the input file and capture output and error
    $PROGRAM < "$infile" > "${base}.actual.out" 2> "${base}.actual.err"

    # Check if output and error match expected results
    if diff -q "${base}.out" "${base}.actual.out" >/dev/null && diff -q "${base}.err" "${base}.actual.err" >/dev/null; then
        # Passed
        echo -e "\e[32mTest ${base#test_} passed\e[0m"
    else
        # Failed
        echo -e "\e[31mTest ${base#test_} failed\e[0m"
        echo "Output diff:"
        diff "${base}.out" "${base}.actual.out"
        echo "Error diff:"
        diff "${base}.err" "${base}.actual.err"
    fi

    # Clean up auxiliary files
    rm "${base}.actual.out" "${base}.actual.err"
done
