/* exec.h
 *
 * Header file for exec.c
 *
 * Author: Ryan McHenry
 * Created: January 23, 2026
 * Last Modified: January 23, 2026
 */

#ifndef EXEC_H
#define EXEC_H

#include "types/types.h"

void execute_command(SHrimpCommand *cmd, SHrimpState *state);
void execute_delayed_command(DelayedCommand *del_cmd, SHrimpState *state);

#endif