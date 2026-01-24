/* pipe.h
 *
 * Header file for pipe.c
 *
 * Author: Ryan McHenry
 * Created: January 23, 2026
 * Last Modified: January 23, 2026
 */

#ifndef PIPE_H
#define PIPE_H

#include "types/types.h"

void check_piping(SHrimpCommand *cmd);
void check_delayed_piping(DelayedCommand *del_cmd);
void pipe_command(SHrimpCommand *cmd, SHrimpState *state);
void pipe_delayed_command(DelayedCommand *del_cmd, SHrimpState *state);

#endif