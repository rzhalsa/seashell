/* main.c
 *
 * SHrimp is a Linux shell which supports the following features:
 *
 *     1. The built-in commands cd and exit.
 *
 *     2. All simple UNIX commands.
 *
 *     3. Commands running in the background using &. The shell
 *        will display the job number and pid of the background
 *        process.
 *
 *     4. Input redirection with < and output redirection with 
 *        either > or >>. Input and output redirection can be
 *        specified within the same command in either order.
 *
 *     5. Commands with a single pipe.
 *
 *     6. Delayed commands using the prefix "delay" with a number of seconds.
 *
 * The basic structure of the shell is an infinite while loop that:
 * 
 *     1. Displays a shell prompt.
 *
 *     2. Waits for the user to enter input. 
 * 
 *     3. Reads the command line.
 *
 *     4. Parses the command line.
 *
 *     5. Takes the appropriate action.
 *
 * Credits and Acknowledgements:
 *
 *     1. Many of the error messages in SHrimp are copies of the
 *        same messages in Bash.
 *
 *     2. The idea to use void(signo) to suppress the compilation warning message
 *        concerning signo being an unused variable came from Stack Overflow.
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
 * Last Modified: February 1, 2026
 */

#include <pthread.h>
#include <sys/wait.h>      // waitpid(), WNOHANG
#include <stdlib.h>        // malloc(), free()
#include <stdio.h>         // printf(), clearerr(), stdin
#include <signal.h>        // SIGCHLD, signal()
#include <errno.h>         // errno, EINTR
#include <time.h>
#include <unistd.h>
#include "types/types.h"   // SHrimpCommand, DelayedCommand, ThreadQueue, SHrimpState, PollArgs
#include "exec/pipe.h"     // check_piping()
#include "exec/redirect.h" // check_redirection()
#include "exec/exec.h"     // execute_command()
#include "parse/parse.h"   // get_input(), parse_input()
#include "delay/delay.h"   // poll()

// function prototypes
void reset_vars(SHrimpCommand *, DelayedCommand *);
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
 * After exiting the main loop of the shell, memory is freed and the background thread is
 * joined.
 */
int main() {
    // Vars
    int display = 1;              // flag to display the SHrimp prompt or not
    char *input;                  // string to store CLI input
    SHrimpCommand cmd = {0};      // shell command
    Commands commands = {0};      // list of commands in a line of input
    DelayedCommand del_cmd = {0}; // command to be delayed
    ThreadQueue queue = {0};      // queue for holding delayed commands
    SHrimpState state = {0};      // shell state

    // Allocate memory for cmd and del_cmd args
    cmd.args = malloc(MAX_ARGS * sizeof(char *));
    del_cmd.args = malloc(MAX_ARGS * sizeof(char *));

    // Set up handler to catch child processes in order to prevent zombies
    signal(SIGCHLD, sig_handler);

    // Init shell state
    state.job_number = 1;
    pthread_mutex_init(&state.mutex, NULL);

    // Init poll args
    PollArgs *p_args = malloc(sizeof(PollArgs)); // args for poll() function
    p_args->queue = &queue;
    p_args->state = &state;

    // Init background thread running the function poll(). Detach p1 as poll() is an infinite while loop.
    pthread_t p1;
    pthread_create(&p1, NULL, poll, p_args);
    pthread_detach(p1);

    // Main loop of the shell
    while(1) {
        commands.command_amt = 0;
        reset_vars(&cmd, &del_cmd);
        
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
        parse_commands(input, &commands);

        display = 1;

        // Parse, check redirection and piping, and then execute each command in commands
        for(int i = 0; i < commands.command_amt; i++) {
            reset_vars(&cmd, &del_cmd);

            // Parse user input
            parse_input(commands.commands[i], &cmd, &del_cmd, &queue, &state);
            if(cmd.args[0] == NULL || cmd.delay == 1)
                continue; 
            if(cmd.delay == -1) {
                printf(RED_TEXT "delay: provide delay amount in seconds" RESET_COLOR "\n");
                continue;
            }  

            // Check for redirection and piping in the command
            check_redirection(&cmd);
            check_piping(&cmd);

            // Execute the command
            execute_command(&cmd, &state); 
        }
    }
    
    // Cleanup
    free(cmd.args);
    free(del_cmd.args);

    return 0;
}

//======================================================================================

/**
 * @brief Resets the values of all variables when starting a new iteration in the main
 * shell loop.
 *
 * @param cmd Pointer to the SHrimpCommand struct variable to reset named cmd.
 * @param del_cmd Pointer to the DelayedCommand struct variable to reset named del_cmd.
 */
void reset_vars(SHrimpCommand *cmd, DelayedCommand *del_cmd) {
    cmd->index = -1;
    cmd->outdex = -1;
    cmd->appenddex = -1;
    cmd->background = 0;
    cmd->input_redirect = 0;
    cmd->output_redirect = 0;
    cmd->append_redirect = 0;
    cmd->pipe_flag = 0;
    errno = 0;
    cmd->delay = 0;

    // cmd vars
    del_cmd->delay_amt = 0;
    del_cmd->index = -1;
    del_cmd->outdex = -1;
    del_cmd->appenddex = -1;
    del_cmd->background = 0;
    del_cmd->input_redirect = 0;
    del_cmd->output_redirect = 0;
    del_cmd->append_redirect = 0;
    del_cmd->pipe_flag = 0;

    for(int i = 0; i < MAX_ARGS; i++) {
        cmd->args[i] = NULL;
        del_cmd->args[i] = NULL;
    }
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