/* utils.c
 *
 * Contains the actual logic to implemen generic helper functions for SHrimp.
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

#include <stdio.h>          // fprintf()
#include <stdlib.h>         // malloc()
#include "config/macros.h"  // RED_TEXT, RESET_COLOR
#include <string.h>         // memset()
#include "utils/utils.h"

//======================================================================================

/**
 * @brief Safely allocates memory and returns a void pointer to it. Safely handles malloc
 * failures.
 *
 * @param size the amount of memory to allocate in bytes
 * @param context a string used in the case of malloc failures to print the context of where
 * the failure occured.
 */
void *safe_malloc(size_t size, const char *context) {
    void *ptr = malloc(size);
    if(!ptr){ // malloc failure
        fprintf(stderr, RED_TEXT "Fatal Error: malloc() failed to allocate memory for %s. Terminating SHrimp now.\n" RESET_COLOR, context);
        exit(1);
    }

    // Initialize memory to zero
    memset(ptr, 0, size);

    return ptr;
}

//======================================================================================