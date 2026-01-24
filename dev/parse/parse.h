/* parse.h
 *
 * Header file for parse.c
 *
 * Author: Ryan McHenry
 * Created: January 23, 2026
 * Last Modified: January 23, 2026
 */

#ifndef PARSE_H
#define PARSE_H

#include "types/types.h"

char *get_input(int);
void parse_input(SHrimpCommand *, DelayedCommand *, ThreadQueue *, SHrimpState *state);

#endif