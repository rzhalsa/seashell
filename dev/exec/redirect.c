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
 * Last Modified: February 10, 2026
 */

#include <string.h>        // strcmp()
#include <unistd.h>        // STDIN_FILENO, STDOUT_FILENO, close(), dup2()
#include <fcntl.h>         // O_RDONLY, O_CREAT, O_WRONLY, O_TRUNC, O_APPEND, open()
#include "types/types.h"   // SHrimpCommand, DelayedCommand
#include "exec/redirect.h"

//======================================================================================

/**
 * @brief Parses args within the command pipeline to check for the redirection
 * tokens <, >, or >>.
 *
 * @param pipeline Pipeline object to check redirection for.
 */
void check_redirection(Pipeline *pipeline) {
    // O(n^2), should probably be streamlined in the future if possible
    for(int i = 0; i < pipeline->command_amt; i++) {
        for(int j = 0; pipeline->commands[i]->args[j] != NULL; j++) {
            // Redirect input token found
            if(strcmp(pipeline->commands[i]->args[j], "<") == 0) {
                pipeline->commands[i]->input_redirect = 1;
                pipeline->commands[i]->index = j;
                pipeline->has_redirect = 1; // true
            }

            // Redirect output token found
            if(strcmp(pipeline->commands[i]->args[j], ">") == 0) {
                pipeline->commands[i]->output_redirect = 1;
                pipeline->commands[i]->outdex = j;
                pipeline->has_redirect = 1; // true
            }

            // Append token found
            if(strcmp(pipeline->commands[i]->args[j], ">>") == 0) {
                pipeline->commands[i]->append_redirect = 1;
                pipeline->commands[i]->appenddex = j;
                pipeline->has_redirect = 1; // true
            }
        }
    }
}

//======================================================================================

/**
 * @brief Redirects the input/output of the command.
 *
 * @param cmd SHrimpCommand object to redirect the input and/or output of.
 */
void redirect(SHrimpCommand *cmd) {
    // Redirect the command's input
    if(cmd->input_redirect == 1) {
        int in = open(cmd->args[cmd->index + 1], O_RDONLY, 0666);
        close(STDIN_FILENO);
        dup2(in, STDIN_FILENO);
        close(in);
        cmd->args[cmd->index] = NULL;
        cmd->args[cmd->index + 1] = NULL;
    }

    // Redirect the command's output 
    if(cmd->output_redirect == 1) {
        int out = open(cmd->args[cmd->outdex + 1], O_CREAT | O_WRONLY | O_TRUNC, 0666);  
        close(STDOUT_FILENO);
        dup2(out, STDOUT_FILENO);
        close(out);
        cmd->args[cmd->outdex] = NULL;
        cmd->args[cmd->outdex + 1] = NULL;    
    }

    // Redirect the command's output and append it to the provided file
    if(cmd->append_redirect == 1) {
        int out = open(cmd->args[cmd->appenddex + 1], O_CREAT | O_WRONLY | O_APPEND, 0666);
        close(STDOUT_FILENO);
        dup2(out, STDOUT_FILENO);
        close(out);
        cmd->args[cmd->appenddex] = NULL;
        cmd->args[cmd->appenddex + 1] = NULL;
    }
}

//======================================================================================