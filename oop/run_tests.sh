#!/bin/env bash

set -euo pipefail
IFS=$'\n\t'
cp college.h tests/college.h
cd tests

use_valgrind=false

if [[ $# -gt 0 && "$1" == "-v" ]]; then
  use_valgrind=true
fi

run_test() {
  local test_num=$1
  if $use_valgrind; then
    valgrind --error-exitcode=123 --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --run-cxx-freeres=yes -q ./tescik && echo -e "\e[32mTest $test_num OK\e[0m" || echo -e "\e[31mTest $test_num Error\e[0m"
  else
    ./tescik && echo -e "\e[32mTest $test_num OK\e[0m" || echo -e "\e[31mTest $test_num Error\e[0m"
  fi
}

for i in {101..104} {201..201} {301..301} {401..401} {501..502} {601..602}; do
  g++ -Wall -Wextra -std=c++20 -DTEST_NUM="$i" college_test.cc -o tescik
  run_test "$i";
done

# These should NOT compile
for i in {202..203} {302..305} {402..408} {503..505}; do
  if ! g++ -Wall -Wextra -std=c++20 -DTEST_NUM="$i" college_test.cc -o tescik 2> /dev/null; then
    echo -e "\e[32mTest $i OK\e[0m"
  else
    echo -e "\e[31mTest $i Error\e[0m"
  fi
done
rm tescik
rm college.h
