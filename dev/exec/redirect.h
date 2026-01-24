/* redirect.h
 *
 * Header file for redirect.c
 *
 * Author: Ryan McHenry
 * Created: January 23, 2026
 * Last Modified: January 23, 2026
 */

#ifndef REDIRECT_H
#define REDIRECT_H

#include "types/types.h"

void check_redirection(SHrimpCommand *cmd);
void check_delayed_redirection(DelayedCommand *del_cmd);
void redirect(char **, int, int, int, int, int, int);

#endif