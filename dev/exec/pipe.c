/* pipe.c
 *
 * Contains the logic for piping a command.
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
 * Last Modified: February 10, 2026
 */

#include <sys/types.h>     // pid_t
#include <sys/wait.h>      // waitpid()
#include <unistd.h>        // pipe(), close(), dup2(), execvp()
#include <stdlib.h>        // malloc(), exit(), free()
#include <string.h>        // strcmp()
#include <stdio.h>         // perror(), printf()
#include "config/macros.h" // RED_TEXT, RESET_COLOR
#include "types/types.h"   // SHrimpCommand, DelayedCommand, SHrimpState
#include "utils/utils.h"   // safe_malloc()
#include "exec/pipe.h"      

char *strdup( const char *str1 );

//======================================================================================

/**
 * @brief Parses cmd->args to check for the pipe token |. The function parses cmd->args
 * and copies into the appropriate pipeline->commands[] indices.
 *
 * @param cmd SHrimpCommand object used to access the args, background flag, and delay
 * flag of the the current command.
 * @param pipeline Pipeline object used to store the parsed commands.
 */
ParseCode check_piping(SHrimpCommand *cmd, Pipeline *pipeline) {

    // Baton pass background, delay, and has_builtin flags
    pipeline->background = cmd->background;
    pipeline->has_builtin = cmd->has_builtin;

    // Initialize temp struct object to use for pipe parsing
    SHrimpCommand *temp = safe_malloc(sizeof(SHrimpCommand), "pipe: temp");
    temp->args = safe_malloc(MAX_ARGS * sizeof(char *), "pipe: temp->args");
    for(int m = 0; m < MAX_ARGS; m++) { // init all indices to NULL
        temp->args[m] = NULL;
    }
    int temp_args_count = 0;
    
    for(int i = 0; i < MAX_ARGS; i++) {
        // Reached end of args stream
        if(cmd->args[i] == NULL) { 
            // Create a copy of temp for the pipeline to own the memory of before temp resets
            SHrimpCommand *copy = safe_malloc(sizeof(SHrimpCommand), "pipe: copy");
            copy->args = safe_malloc((temp_args_count + 1) * sizeof(char *), "pipe: copy->args");
            for(int j = 0; j < temp_args_count; j++) {
                copy->args[j] = strdup(temp->args[j]);
            }
            pipeline->commands[pipeline->command_amt++] = copy;

            // Free temp
            for(int m = 0; m < MAX_ARGS; m++) {
                free(temp->args[m]);
                temp->args[m] = NULL;
            }
            free(temp->args);
            free(temp);

            return PARSE_OK;
        } 

        // Check if a pipe character was found
        if(strcmp(cmd->args[i], "|") == 0) {

            // Safety check to catch edge cases such as "echo one two three |"
            // and "| echo hi"
            if(i == 0 || cmd->args[i + 1] == NULL) {
                // Free temp
                for(int m = 0; m < MAX_ARGS; m++) {
                    free(temp->args[m]);
                    temp->args[m] = NULL;
                }
                free(temp->args);
                free(temp);

                return PARSE_INVALID_PIPE;
            }

            // Create a copy of temp for the pipeline to own the memory of before temp resets
            SHrimpCommand *copy = safe_malloc(sizeof(SHrimpCommand), "pipe: copy");
            copy->args = safe_malloc((temp_args_count + 1) * sizeof(char *), "pipe: copy->args");
            for(int j = 0; j < temp_args_count; j++) {
                copy->args[j] = strdup(temp->args[j]);
            }

            // Assign the memory of copy to pipeline
            pipeline->commands[pipeline->command_amt++] = copy;

            for(int m = 0; m < MAX_ARGS; m++) { // init all indices to NULL
                free(temp->args[m]);
                temp->args[m] = NULL;
            }
            temp_args_count = 0;
            pipeline->has_pipe = 1; // true
            continue;
        }

        temp->args[temp_args_count] = strdup(cmd->args[i]);
        if(!temp->args[temp_args_count]) { // malloc within strdup failed
            fprintf(stderr, RED_TEXT"Fatal Error: strdup(): malloc() failed to allocate memory for %s. Terminating SHrimp now.\n" RESET_COLOR, "temp->args");
            for(int m = 0; m < MAX_ARGS; m++) {
                free(temp->args[m]);
                temp->args[m] = NULL;
            }
            free(temp->args);
            free(temp);

            exit(1);
        }
        temp_args_count++;
    }

    // Fixed args buffer for the moment, so simply return out of range enum
    return PARSE_CMD_OUT_OF_RANGE;
}

//======================================================================================