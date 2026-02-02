#!/bin/bash
#
# echo.sh
#
# Tests simple echo commands on SHrimp. Also tests for multiple command parsing.
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

# Simple echo test
echo 'echo testing' | "$SHRIMP_BIN" | grep -q 'testing' || { echo "Echo test failed"; exit 1; }

# Test two commands at once
echo 'echo a cat; echo a dog; echo a bird' | "$SHRIMP_BIN" | grep -q 'bird' || { echo "Multiple commands test failed"; exit 1; }
