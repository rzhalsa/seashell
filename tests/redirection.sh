#!/bin/bash
#
# redirection.sh
#
# Tests input, output, and append redirection
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

# Set shell binary to be the first argument
SHRIMP_BIN=$1

# Input redirection
echo 'echo "to shell or not to shell" | wc -w' > test.txt
INPUT=$("$SHRIMP_BIN" < test.txt)
INPUT_EXPECTED=6
rm test.txt

if [ "$INPUT" -ne "$INPUT_EXPECTED" ]; then
    echo "redirection.sh: INPUT TEST FAILED"
    echo "Expected: "$INPUT_EXPECTED"
    echo "Output: "$INPUT"
    exit 1
fi

# Output redirection
echo 'echo crustaceans have shells' | ${SHRIMP_BIN} > out.txt
OUTPUT=$(cat out.txt)
OUTPUT_EXPECTED="crustaceans have shells"

if [ "$OUTPUT" != "$OUTPUT_EXPECTED" ]; then
    echo "redirection.sh: OUTPUT TEST FAILED"
    echo "Expected: "$OUTPUT_EXPECTED"
    echo "Output: "$OUTPUT"
    exit 1
fi

# Append redirection
echo 'echo and live under the water' | ${SHRIMP_BIN} >> out.txt
APPENDPUT=$(cat out.txt)
APPEND_EXPECTED=$'crustaceans have shells\nand live under the water'
rm out.txt

if [ "$APPENDPUT" != "$APPEND_EXPECTED" ]; then
    echo "redirection.sh: APPEND TEST FAILED"
    echo "Expected: "$APPEND_EXPECTED"
    echo "Output: "$APPENDPUT"
    exit 1
fi

