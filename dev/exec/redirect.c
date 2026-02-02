/* redirect.c
 *
 * Contains the logic for redirecting a command.
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

#include <string.h>        // strcmp()
#include <unistd.h>        // STDIN_FILENO, STDOUT_FILENO, close(), dup2()
#include <fcntl.h>         // O_RDONLY, O_CREAT, O_WRONLY, O_TRUNC, O_APPEND, open()
#include "types/types.h"   // SHrimpCommand, DelayedCommand
#include "exec/redirect.h"

//======================================================================================

/**
 * @brief Parses args to check for the redirection tokens <, >, or >>.
 *
 * @param cmd SHrimpCommand object which contains all relevant vars for checking command redirection.
 */
void check_redirection(SHrimpCommand *cmd) {
    for(int i = 0; i < MAX_ARGS; i++) {
        if(cmd->args[i] == NULL)
            return;
        if(strcmp(cmd->args[i], "<") == 0) {
            cmd->input_redirect = 1;
            cmd->index = i;
        } 
        if(strcmp(cmd->args[i], ">") == 0) {
            cmd->output_redirect = 1;
            cmd->outdex = i;
        } 
        if(strcmp(cmd->args[i], ">>") == 0) {
            cmd->append_redirect = 1;
            cmd->appenddex = i;
        }
    }
}

//======================================================================================

/**
 * @brief Parses args to check for the redirection tokens <, >, or >>.
 *
 * @param del_cmd DelayedCommand object which contains all relevant vars for checking command redirection.
 */
void check_delayed_redirection(DelayedCommand *del_cmd) {
    for(int i = 0; i < MAX_ARGS; i++) {
        if(del_cmd->args[i] == NULL)
            return;
        if(strcmp(del_cmd->args[i], "<") == 0) {
            del_cmd->input_redirect = 1;
            del_cmd->index = i;
        } 
        if(strcmp(del_cmd->args[i], ">") == 0) {
            del_cmd->output_redirect = 1;
            del_cmd->outdex = i;
        } 
        if(strcmp(del_cmd->args[i], ">>") == 0) {
            del_cmd->append_redirect = 1;
            del_cmd->appenddex = i;
        }
    }
}

//======================================================================================

/**
 * @brief Redirects the input/output of the command.
 *
 * @param args 2D char array containing the command and all its arguments.
 * @param input_redirect Integer flag to denote if the command contains input redirection.
 * @param output_redirect Integer flag to denote if the command contains output redirection.
 * @param append_redirect Integer flag to denote if the command contains append redirection.
 * @param index Integer value to denote the index of the < token.
 * @param outdex Integer value to denote the index of the > token.
 * @param appenddex Integer value to denote the index of the >> token.
 */
void redirect(char **args, int input_redirect, int output_redirect, int append_redirect,
                int index, int outdex, int appenddex) {
    // Redirect the command's input
    if(input_redirect == 1) {
        int in = open(args[index + 1], O_RDONLY, 0666);
        close(STDIN_FILENO);
        dup2(in, STDIN_FILENO);
        close(in);
        args[index] = NULL;
        args[index + 1] = NULL;
    }

    // Redirect the command's output 
    if(output_redirect == 1) {
        int out = open(args[outdex + 1], O_CREAT | O_WRONLY | O_TRUNC, 0666);  
        close(STDOUT_FILENO);
        dup2(out, STDOUT_FILENO);
        close(out);
        args[outdex] = NULL;
        args[outdex + 1] = NULL;    
    }

    // Redirect the command's output and append it to the provided file
    if(append_redirect == 1) {
        int out = open(args[appenddex + 1], O_CREAT | O_WRONLY | O_APPEND, 0666);
        close(STDOUT_FILENO);
        dup2(out, STDOUT_FILENO);
        close(out);
        args[appenddex] = NULL;
        args[appenddex + 1] = NULL;
    }
}

//======================================================================================