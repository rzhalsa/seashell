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
 * Last Modified: February 1, 2026
 */

#include <sys/types.h>      // pid_t
#include <sys/wait.h>       // waitpid()
#include <unistd.h>         // pipe(), close(), dup2(), execvp()
#include <stdlib.h>         // malloc(), exit(), free()
#include <string.h>         // strcmp()
#include <stdio.h>          // perror(), printf()
#include "exec/pipe.h"      
#include "exec/redirect.h"  // redirect()
#include "types/types.h"    // SHrimpCommand, DelayedCommand, SHrimpState

//======================================================================================

/**
 * @brief Parses args to check for the pipe token |.
 *
 * @param cmd SHrimpCommand object used to access the args, pipe_flag, and pipedex of
 * the current command.
 */
void check_piping(SHrimpCommand *cmd) {
    for(int i = 0; i < MAX_ARGS; i++) {
        if(cmd->args[i] == NULL)
            return;
        if(strcmp(cmd->args[i], "|") == 0) {
            cmd->pipe_flag = 1;
            cmd->pipedex = i;
            return;
        }
    }
}

//======================================================================================

/**
 * @brief Parses args to check for the pipe token |.
 *
 * @param del_cmd DelayedCommand object used to access the args, pipe_flag, and pipedex of
 * the current delayed command.
 */
void check_delayed_piping(DelayedCommand *del_cmd) {
    for(int i = 0; i < MAX_ARGS; i++) {
        if(del_cmd->args[i] == NULL)
            return;
        if(strcmp(del_cmd->args[i], "|") == 0) {
            del_cmd->pipe_flag = 1;
            del_cmd->pipedex = i;
            return;
        }
    }
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
 */
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

//======================================================================================


/**
 * @brief Pipes the user delayed command.
 *
 * @param del_cmd DelayedCommand object containing all needed values to pipe a command.
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
 */
void pipe_delayed_command(DelayedCommand *del_cmd, SHrimpState *state) {
    int fd[2];                            // file descriptors
    int offset = (del_cmd->pipedex + 1);  // offset for indexing redirection after pipe
    char **command_1;                     // command before pipe
    char **command_2;                     // command after pipe
    int command_2_counter = 0;            // counter to index command_2 properly

    command_1 = malloc(MAX_ARGS * sizeof(char *));
    command_2 = malloc(MAX_ARGS * sizeof(char *));

    for(int i = 0; i < del_cmd->pipedex; i++) {
        command_1[i] = del_cmd->args[i];
    }

    for(int j = offset; j < MAX_ARGS; j++) {
        if(del_cmd->args[j] == NULL)
            break;
        command_2[command_2_counter++] = del_cmd->args[j];
    }

    command_1[del_cmd->pipedex] = NULL;
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

        if(del_cmd->input_redirect == 1 && del_cmd->index < del_cmd->pipedex) {
            redirect(command_1, del_cmd->input_redirect, del_cmd->output_redirect, del_cmd->append_redirect, 
                del_cmd->index, del_cmd->outdex, del_cmd->appenddex);
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

        if(del_cmd->output_redirect == 1 && del_cmd->outdex > del_cmd->pipedex) {
            redirect(command_2, del_cmd->input_redirect, del_cmd->output_redirect, del_cmd->append_redirect, 
                        del_cmd->index, del_cmd->outdex - offset, del_cmd->appenddex);
        }

        if(del_cmd->append_redirect == 1 && del_cmd->appenddex > del_cmd->pipedex) {
            redirect(command_2, del_cmd->input_redirect, del_cmd->output_redirect, del_cmd->append_redirect, 
                        del_cmd->index, del_cmd->outdex, del_cmd->appenddex - offset);
        }

        execvp(command_2[0], command_2);
        printf("%s: command not found\n", command_2[0]); 
        exit(1);
    } 

    // Parent
    close(fd[0]);
    close(fd[1]);
    if(del_cmd->background == 0) {
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    } else {
        printf("[%d] %d\n", state->job_number++, pid1);
        printf("[%d] %d\n", state->job_number++, pid2);
    }

    free(command_1);
    free(command_2);
}

//======================================================================================