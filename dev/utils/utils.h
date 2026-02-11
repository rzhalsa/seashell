/* utils.h
 *
 * Contains generic helper functions for SHrimp. Header file for utils.c.
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
 * Created: Feberuary 4, 2026
 * Last Modified: February 4, 2026
 */

#ifndef UTILS_H
#define UTILS_H

#include <stddef.h> // size_t

void *safe_malloc(size_t size, const char *context);

#endif