/* exec.c
 *
 * Contains the logic for executing a command
 *
 * Author: Ryan McHenry
 * Created: January 23, 2026
 * Last Modified: January 23, 2026
 */

#include <sys/types.h>     // pid_t
#include <sys/wait.h>      // waitpid()
#include <string.h>        // strcmp()
#include <stdio.h>         // printf(), perror()
#include <stdlib.h>        // exit()
#include <unistd.h>        // chdir(), execvp()
#include "types/types.h"   // SHrimpCommand, DelayedCommand, SHrimpState
#include "exec/exec.h" 
#include "exec/pipe.h"     // pipe_command(), pipe_delayed_command()
#include "exec/redirect.h" // redirect()

//======================================================================================

/**
 * @brief Executes the built-in Linux command cd using chdir().
 *
 * @param args 2D char array containing the command and all its arguments.
 *
 * @return 0 to denote a successful directory change, 1 to denote an insuccessful
 * directory change.
 */
static int cd(char **args) {
    if(args[2] != NULL) {
        printf("cd: too many arguments\n");
        return 1;        
    } 

    // cd to home dir
    if(args[1] == NULL || strcmp(args[1], "~") == 0) {
        char *home = getenv("HOME");
        if(home != NULL) {
            if(chdir(home) == -1) {
                printf("shrimp: cd home: No home directory found\n");
                return 1;
            }
        } else {
            printf("cd: error finding home directory\n");
            return 1;
        }
        return 0;
    }

    // cd to dir passed as args[1]
    if(chdir(args[1]) == -1) {
        printf("shrimp: cd: %s: No such file or directory\n", args[1]);
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
static int exec_unix_command(SHrimpCommand *cmd, SHrimpState *state) {
    // Pipe if applicable
    if(cmd->pipe_flag == 1) {
        pipe_command(cmd, state);
        return 0;
    }

    pid_t pid = fork();
    if(pid == -1) {
        perror("fork: error while forking");
        exit(1);     
    } else if(pid == 0) {
        // Redirect if applicable
        if(cmd->input_redirect == 1 || cmd->output_redirect == 1 || cmd->append_redirect == 1) {
            redirect(cmd->args, cmd->input_redirect, cmd->output_redirect, cmd->append_redirect, 
                cmd->index, cmd->outdex, cmd->appenddex);
        } 

        // Execute args[0] using any arguments passed by user input
        execvp(cmd->args[0], cmd->args);
        printf("%s: command not found\n", cmd->args[0]); 
        exit(1); 
    } else {
        if(cmd->background == 0)
            waitpid(pid, NULL, 0);
        else { 
            printf("[%d] %d\n", state->job_number++, pid);
        }
    }

    return 0;
}

//======================================================================================

/**
 * @brief Executes the user delayed command.
 *
 * @param del_cmd DelayedCommand object containing all needed values to execute a unix command.
 * @param state SHrimpState object allowing access to the shell's job_number variable.
 *
 * @return 0 if command successfully execited, 1 if execution was insuccessful.
 *
 * @details Executes the user command. The function first pipes the command if applicable. Then,
 * the process is forked and on the child process the command is redirected if applicable, and 
 * then executed. If the process is not run in the background, the parent process waits for the
 * child process to finish executing the command.
 */
static int exec_delayed_unix_command(DelayedCommand *del_cmd, SHrimpState *state) {
    // Pipe if applicable
    if(del_cmd->pipe_flag == 1) {
        pipe_delayed_command(del_cmd, state);
        return 0;
    }

    pid_t pid = fork();
    if(pid == -1) {
        perror("fork: error while forking");
        exit(1);     
    } else if(pid == 0) {
        // Redirect if applicable
        if(del_cmd->input_redirect == 1 || del_cmd->output_redirect == 1 || del_cmd->append_redirect == 1) {
            redirect(del_cmd->args, del_cmd->input_redirect, del_cmd->output_redirect, del_cmd->append_redirect, 
                del_cmd->index, del_cmd->outdex, del_cmd->appenddex);
        } 

        // Execute args[0] using any arguments passed by user input
        execvp(del_cmd->args[0], del_cmd->args);
        printf("%s: command not found\n", del_cmd->args[0]); 
        exit(1); 
    } else {
        if(del_cmd->background == 0)
            waitpid(pid, NULL, 0);
        else { 
            printf("[%d] %d\n", state->job_number++, pid);
        }
    }

    return 0;
}

//======================================================================================

/**
 * @brief Executes the user command via cd() or exit() for the built-in Linux commands
 * cd and exit, or via exec_unix_command() for any other command.
 *
 * @param cmd SHrimpCommand object containing all needed values to execute a command.
 * @param state SHrimpState object allowing access to the shell's job_number variable.
 */
void execute_command(SHrimpCommand *cmd, SHrimpState *state) {
    if(strcmp(cmd->args[0], "cd") == 0) {
        cd(cmd->args);
    } else if(strcmp(cmd->args[0], "exit") == 0) {
        exit(0);
    } else {
        exec_unix_command(cmd, state);
    }
}

//======================================================================================

/**
 * @brief Executes the user command via cd() or exit() for the built-in Linux commands
 * cd and exit, or via exec_unix_command() for any other command.
 *
 * @param del_cmd DelayedCommand object containing all needed values to execute a command.
 * @param state SHrimpState object allowing access to the shell's job_number variable.
 */
void execute_delayed_command(DelayedCommand *del_cmd, SHrimpState *state) {
    if(strcmp(del_cmd->args[0], "cd") == 0) {
        cd(del_cmd->args);
    } else if(strcmp(del_cmd->args[0], "exit") == 0) {
        exit(0);
    } else {
        exec_delayed_unix_command(del_cmd, state);
    }
}

//======================================================================================