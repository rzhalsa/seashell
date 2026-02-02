/* parse.h
 *
 * Header file for parse.c
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Author: Ryan McHenry
 * Created: January 23, 2026
 * Last Modified: February 1, 2026
 */

#ifndef PARSE_H
#define PARSE_H

#include "types/types.h"

char *get_input(int);
void parse_commands(char *input, Commands *cmds);
void parse_input(char*, SHrimpCommand *, DelayedCommand *, ThreadQueue *, SHrimpState *state);

#endif