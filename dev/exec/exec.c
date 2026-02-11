/* exec.c
 *
 * Contains the logic for executing a command
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
#include <string.h>        // strcmp()
#include <stdio.h>         // printf(), perror()
#include <stdlib.h>        // exit()
#include <unistd.h>        // chdir(), execvp()
#include "config/macros.h" // RED_TEXT, RESET_COLOR
#include "types/types.h"   // SHrimpCommand, DelayedCommand, SHrimpState
#include "exec/redirect.h" // redirect()
#include "exec/exec.h"

//======================================================================================

/**
 * @brief Executes the built-in Linux command cd using chdir().
 *
 * @param args 2D char array containing the command and all its arguments.
 *
 * @return 0 to denote a successful directory change, 1 to denote an insuccessful
 * directory change.
 */
int cd(char **args) {
    if(args[2] != NULL) {
        printf(RED_TEXT "cd: too many arguments" RESET_COLOR "\n");
        return 1;        
    } 

    // cd to home dir
    if(args[1] == NULL || strcmp(args[1], "~") == 0) {
        char *home = getenv("HOME");
        if(home != NULL) {
            if(chdir(home) == -1) {
                printf(RED_TEXT "SHrimp: cd home: No home directory found" RESET_COLOR "\n");
                return 1;
            }
        } else {
            printf(RED_TEXT "SHrimp: cd: error finding home directory" RESET_COLOR "\n");
            return 1;
        }
        return 0;
    }

    // cd to dir passed as args[1]
    if(chdir(args[1]) == -1) {
        printf(RED_TEXT "SHrimp: cd: %s: No such file or directory" RESET_COLOR "\n", args[1]);
        return 1;
    }

    return 0;
}

//======================================================================================

/**
 * @brief Executes the user command.
 *
 * @param cmd SHrimpCommand object containing all needed values to execute a unix command.
 * @param state SHrimpState object allowing access to the shell's job_number variable.
 *
 * @return 0 if command successfully execited, 1 if execution was insuccessful.
 *
 * @details Executes the user command. The function first pipes the command if applicable. Then,
 * the process is forked and on the child process the command is redirected if applicable, and 
 * then executed. If the process is not run in the background, the parent process waits for the
 * child process to finish executing the command.
 */
int exec_pipeline(Pipeline *pipeline, SHrimpState *state) {
    // Create file descriptors for each command in the pipeline
    int fd[pipeline->command_amt - 1][2];

    // Create pipeline->command_amt - 1 pipes
    for(int i = 0; i < pipeline->command_amt - 1; i++) {
        if(pipe(fd[i]) < 0) {
            perror("pipe failed");
            exit(1);
        }
    }

    // Fork pipeline->command_amt child processes. For each one set the correct fd depending
    // on its position in the pipeline, redirect if applicable and then execute
    pid_t pids[pipeline->command_amt];
    for(int i = 0; i < pipeline->command_amt; i++) {
        pids[i] = fork();
        if(pids[i] < 0) {
            perror("fork failed");
            exit(1);
        } else if(pids[i] > 0) { // parent
            // Close file descriptors in the parent process
            if(i > 0)
                close(fd[i-1][0]);
            if(i < pipeline->command_amt - 1)
                close(fd[i][1]);
        } else if(pids[i] == 0) { // child
            // Set the correct fd
            if(i > 0) {
                dup2(fd[i-1][0], STDIN_FILENO);
            }
            if(i < pipeline->command_amt - 1) {
                dup2(fd[i][1], STDOUT_FILENO);
            }
            for(int j = 0; j < pipeline->command_amt - 1; j++) {
                close(fd[j][0]);
                close(fd[j][1]);
            }

            // Redirect if applicable
            if(pipeline->commands[i]->input_redirect == 1 || pipeline->commands[i]->output_redirect == 1 || pipeline->commands[i]->append_redirect == 1) {
                redirect(pipeline->commands[i]);
            }

            // Execute the current command in the pipeline
            execvp(pipeline->commands[i]->args[0], pipeline->commands[i]->args);
            printf(RED_TEXT "%s: command not found" RESET_COLOR "\n", pipeline->commands[i]->args[0]); 
            exit(1); 
        }
    }

    if(pipeline->background == 0) {
        for(int i = 0; i < pipeline->command_amt; i++) {
            waitpid(pids[i], NULL, 0);
        }
    } else {
        for(int i = 0; i < pipeline->command_amt; i++) {
            printf("[%d] %d\n", state->job_number++, pids[i]);
        } 
    }

    return 0;
}

//======================================================================================

