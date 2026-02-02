/* redirect.h
 *
 * Header file for redirect.c
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

#ifndef REDIRECT_H
#define REDIRECT_H

#include "types/types.h"

void check_redirection(SHrimpCommand *cmd);
void check_delayed_redirection(DelayedCommand *del_cmd);
void redirect(char **, int, int, int, int, int, int);

#endif