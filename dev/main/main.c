/* main.c
 *
 * Main function of SHrimp. Contains the main shell loop.
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
 * Created: March 21, 2025
 * Last Modified: February 14, 2026
 */

#include <pthread.h>
#include <sys/wait.h>      // waitpid(), WNOHANG
#include <stdlib.h>        // malloc(), free()
#include <stdio.h>         // printf(), clearerr(), stdin
#include <signal.h>        // SIGCHLD, signal()
#include <errno.h>         // errno, EINTR
#include <string.h>        // strcmp()
#include "types/types.h"   // ParseCode, SHrimpCommand, Commands, Pipeline, SHrimpState
#include "exec/pipe.h"     // check_piping()
#include "exec/redirect.h" // check_redirection()
#include "exec/exec.h"     // execute_command()
#include "parse/parse.h"   // get_input(), parse_input()
#include "utils/utils.h"   // safe_malloc()

// function prototypes
void reset_vars(SHrimpCommand *cmd, Pipeline *pipeline);
void sig_handler(int signo);

//======================================================================================

/**
 * @brief main function that contains all vars, initializes everything, and contains the
 * main loop of the shell.
 * 
 * @return 0 on successfully terminating.
 *
 * @details  Declares vars used by all helper functions, sets up the signal handler to catch child
 * processes, initializes the ThreadQueue struct variable as well as the background thread
 * used within the helper function poll().
 *
 * Contains the main loop of the shell itself. The each time the shell initializes a new
 * iteration of the while loop, it executes the following steps:
 *
 *   1. Reset all variables from the previous iteration.
 *   2. Receive the user input.
 *   3. Parse the user input.
 *   4. Further parse the input to check for redirection or piping.
 *   5. If reaching this step without any errors, execute the provided command.
 *
 * After exiting the main loop of the shell, allocated heap memory is freed.
 */
int main() {
    // Vars
    int display = 1;              // flag to display the SHrimp prompt or not
    char *input;                  // string to store CLI input
    SHrimpCommand cmd = {0};      // shell command
    Commands commands = {0};      // list of commands in a line of input
    Pipeline pipeline = {0};      // pipeline of current command to execute
    SHrimpState state = {0};      // shell state
    ParseCode parsecode;          // enum used to handle errors while parsing commands

    // Allocate memory for cmd
    cmd.args = safe_malloc(MAX_ARGS * sizeof(char *), "cmd.args");

    // Set up handler to catch child processes in order to prevent zombies
    signal(SIGCHLD, sig_handler);

    // Init shell state
    state.job_number = 1;

    // Main loop of SHrimp
    while(1) {
        // Reset for new loop iteration
        commands.command_amt = 0;
        reset_vars(&cmd, &pipeline);
        
        // Obtain user input
        input = get_input(display);
        if(input == NULL){
            if(errno == EINTR) {
                display = 0;
                clearerr(stdin);
                continue;
            }
            break;
        }

        // Parse raw input for semi-colons to determine if there are multiple commands to execute
        parsecode = parse_commands(input, &commands);
        
        display = 1;

        // Parse, check redirection and piping, and then execute each command in commands
        for(int i = 0; i < commands.command_amt; i++) {
            reset_vars(&cmd, &pipeline);

            // Parse user input
            parsecode = parse_input(commands.commands[i], &cmd);

            // Continue if the user pressed enter or if the command is delayed
            if(cmd.args[0] == NULL)
                continue; 

            // Error handling for certain parsecode values
            switch(parsecode) {
                case PARSE_INVALID_DELAY:
                    fprintf(stderr, RED_TEXT "Error: invalid delay amount. Must be a positive integer\n" RESET_COLOR);
                    continue;
                case PARSE_DELAY_OUT_OF_RANGE:
                    fprintf(stderr, RED_TEXT "Error: delay amount out of range.\n" RESET_COLOR);
                    continue;
                case PARSE_NEGATIVE_DELAY:
                    fprintf(stderr, RED_TEXT "Error: delay amount cannot be less than 0.\n" RESET_COLOR);
                    continue;
                default:
                    break;
            }

            // Scan cmd before it is passed off to pipeline for built-in commands
            for(int j = 0; cmd.args[j] != NULL; j++) {
                if(strcmp(cmd.args[j], "cd") == 0 || strcmp(cmd.args[j], "exit") == 0) {
                    if(j != 0) { // short-circuit to the next loop iteration if built-in not in the first index
                        fprintf(stderr, RED_TEXT "Error: the built-in commands 'cd' and 'exit' must be the first token of a given command.\n" RESET_COLOR);
                        continue;
                    }
                    cmd.has_builtin = 1; // true
                }
            }

            // Parse cmd for pipes
            parsecode = check_piping(&cmd, &pipeline);

            if(parsecode == PARSE_INVALID_PIPE) {
                fprintf(stderr, RED_TEXT "Pipe error: A pipe cannot begin or end a line\n" RESET_COLOR);
                continue;
            } else if(parsecode == PARSE_CMD_OUT_OF_RANGE) {
                // Should probably be removed and replaced with dynamic buffer size using realloc()
                fprintf(stderr, RED_TEXT "Error: Too many commands\n" RESET_COLOR);
            }

            // Check for redirection for each command in the pipeline
            check_redirection(&pipeline);

            // Execute the built-in commands cd or exit here if there are no pipes or redirection characters
            // If there are pipes or redirection, print an error message and continue to the next loop iteration
            if(pipeline.has_builtin == 1) {
                if(pipeline.has_pipe || pipeline.has_redirect) {
                    fprintf(stderr, RED_TEXT "Error: cannot contain pipes or redirection alongside a built-in command\n" RESET_COLOR);
                    continue;
                }

                // Check if the built-in is cd and call cd() if so. Otherwise exit
                if(strcmp(pipeline.commands[0]->args[0], "cd") == 0) {
                    cd(pipeline.commands[0]->args);
                    continue;
                } else {
                    exit(0); // will exit if a line is "exit 1 2 3", needs addressed
                }
            }
            
            // Execute the full command pipeline
            exec_pipeline(&pipeline, &state); 
        }
    }
    
    // Free allocated heap memory
    free(cmd.args);
    for(int i = 0; i < MAX_ARGS; i++) {
        free(pipeline.commands[i]);
    }

    return 0;
}

//======================================================================================

/**
 * @brief Resets the values of all variables when starting a new iteration in the main
 * shell loop.
 *
 * @param cmd Pointer to the SHrimpCommand struct variable to reset named cmd.
 */
void reset_vars(SHrimpCommand *cmd, Pipeline *pipeline) {
    cmd->index = -1;
    cmd->outdex = -1;
    cmd->appenddex = -1;
    cmd->background = 0;
    cmd->input_redirect = 0;
    cmd->output_redirect = 0;
    cmd->append_redirect = 0;
    cmd->has_builtin = 0;
    errno = 0;

    for(int i = 0; i < MAX_ARGS; i++) {
        cmd->args[i] = NULL;
    }

    pipeline->has_builtin = 0;
    pipeline->background = 0;
    pipeline->has_redirect = 0;
    pipeline->has_pipe = 0;
    pipeline->command_amt = 0;
}

//======================================================================================

/**
 * @brief Prevents zombie process buildup.
 *
 * @param signo SIGCHLD
 */
void sig_handler(int signo) {
    signal(SIGCHLD, sig_handler);

    // Suppress compilation warning message
    (void)signo; 

    // Prevent zombie process buildup
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

//======================================================================================