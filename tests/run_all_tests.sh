#!/bin/bash
#
# run_all_tests.sh
#
# Runs all tests for SHrimp from a single script
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
# 
# Author: Ryan McHenry
# Created: February 1, 2026
# Last Modified: February 1, 2026

# Exit if a command failed
set -e

# Set shell binary to be the first argument
SHRIMP_BIN=$1

# Ensure the first argument exists
if [ -z "$SHRIMP_BIN" ]; then
    echo "Usage: $0 tmp_install/bin/shrimp"
    exit 1
fi

# Exit if more than one argument is passed
if [ -n "$2" ]; then
    echo "Too many arguments. Only one expected."
    exit 1
fi

# Run all other test scripts in this directory
for tests in "$PWD"/tests/*.sh; do
    # Skip run_all_tests.sh to prevent recursion
    [ "$tests" = "$PWD/tests/run_all_tests.sh" ] && continue
    bash "$tests" "$SHRIMP_BIN"
done

echo "All tests passed"