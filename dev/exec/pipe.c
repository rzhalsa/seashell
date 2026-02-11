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

#include <sys/types.h>      // pid_t
#include <sys/wait.h>       // waitpid()
#include <unistd.h>         // pipe(), close(), dup2(), execvp()
#include <stdlib.h>         // malloc(), exit(), free()
#include <string.h>         // strcmp()
#include <stdio.h>          // perror(), printf()
#include "exec/pipe.h"      
#include "config/macros.h"
#include "types/types.h"    // SHrimpCommand, DelayedCommand, SHrimpState
#include "utils/utils.h"    // safe_malloc()

char * strdup( const char *str1 );

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

/**
 * @brief Pipes the user command.
 *
 * @param cmd SHrimpCommand object containing all needed values to pipe a command.
 * @param state SHrimpState object allowing access to the shell's job_number variable.
 *
 * @details The function separates the commands before and after the pipe into command_1 and
 * command_2. Then, the function creates a pipe using the created file descriptors. 
 * Afterwards, pid1 is forked and in the child process the output of command_1 is set to fd[1]. 
 * Then, the input of command_1 is redirected if applicable, and then command_1 is executed. 
 * Next, the same happens with pid2. It is forked, the input of command_2 is set of fd[0], the 
 * output of command_2 is redirected if applicable, and then command_2 is executed.
 * Meanwhile, the parent process closes both of its created file descriptors. If the process is
 * not run in the background, the parent process waits for both pid1 and pid2, if it is, the
 * parent prints out the job number of the child processes to the terminal. Finally, the parent
 * frees the memory allocated to command_1 and command_2.
 *//*
void pipe_command(SHrimpCommand *cmd, SHrimpState *state) {
    int fd[2];                      // file descriptors
    int offset = (cmd->pipedex + 1);     // offset for indexing redirection after pipe
    char **command_1;               // command before pipe
    char **command_2;               // command after pipe
    int command_2_counter = 0;      // counter to index command_2 properly

    command_1 = malloc(MAX_ARGS * sizeof(char *));
    command_2 = malloc(MAX_ARGS * sizeof(char *));

    for(int i = 0; i < cmd->pipedex; i++) {
        command_1[i] = cmd->args[i];
    }

    for(int j = offset; j < MAX_ARGS; j++) {
        if(cmd->args[j] == NULL)
            break;
        command_2[command_2_counter++] = cmd->args[j];
    }

    command_1[cmd->pipedex] = NULL;
    command_2[command_2_counter] = NULL;

    if(pipe(fd) < 0) {
        perror("pipe failed");
        exit(1);
    }

    pid_t pid1 = fork();
    if(pid1 < 0) {
        perror("first fork failed");
        exit(1);
    } else if(pid1 == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        if(cmd->input_redirect == 1 && cmd->index < cmd->pipedex) {
            redirect(command_1, cmd->input_redirect, cmd->output_redirect, cmd->append_redirect, 
                cmd->index, cmd->outdex, cmd->appenddex);
        } 

        execvp(command_1[0], command_1);
        printf("%s: command not found\n", command_1[0]); 
        exit(1);
    } 

    pid_t pid2 = fork();
    if(pid2 < 0) {
        perror("second fork failed");
        exit(1);
    } else if(pid2 == 0) {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

        if(cmd->output_redirect == 1 && cmd->outdex > cmd->pipedex) {
            redirect(command_2, cmd->input_redirect, cmd->output_redirect, cmd->append_redirect, 
                        cmd->index, cmd->outdex - offset, cmd->appenddex);
        }

        if(cmd->append_redirect == 1 && cmd->appenddex > cmd->pipedex) {
            redirect(command_2, cmd->input_redirect, cmd->output_redirect, cmd->append_redirect, 
                        cmd->index, cmd->outdex, cmd->appenddex - offset);
        }

        execvp(command_2[0], command_2);
        printf("%s: command not found\n", command_2[0]); 
        exit(1);
    } 

    // Parent
    close(fd[0]);
    close(fd[1]);
    if(cmd->background == 0) {
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    } else {
        printf("[%d] %d\n", state->job_number++, pid1);
        printf("[%d] %d\n", state->job_number++, pid2);
    }

    free(command_1);
    free(command_2);
}
*/
//======================================================================================


