#!/bin/bash
#
# pipes.sh
#
# Tests a pipe command
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

# Single pipe
OUTPUT=$(echo 'echo "cats dogs birds snakes sharks" | wc -w' | "$SHRIMP_BIN")
EXPECTED=5

if [ "$OUTPUT" = "$EXPECTED" ]; then
    exit 0
else
    echo "pipes.sh: SINGLE PIPE TEST FAILED"
    echo "Expected: "$EXPECTED""
    echo "Output: "$OUTPUT""
    exit 1
fi

# Multiple pipes
OUTPUT=$(echo 'echo "one two three" | grep one | wc -w' | "$SHRIMP_BIN")
EXPECTED=3

if [ "$OUTPUT" = "$EXPECTED" ]; then
    exit 0
else
    echo "pipes.sh: MULTIPLE PIPES TEST FAILED"
    echo "Expected: "$EXPECTED""
    echo "Output: "$OUTPUT""
    exit 1
fi