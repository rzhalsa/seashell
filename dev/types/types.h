/* types.h
 *
 * Contains all type declarations used by the shell in one convenient location.
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

#ifndef TYPES_H
#define TYPES_H

#include "config/macros.h" // MAX_ARGS
#include <pthread.h>       // pthread_mutex_t

// Enums for function return codes, codes are handled in the main SHrimp loop
typedef enum {
    PARSE_OK,
    PARSE_MALLOC_FAIL,
    PARSE_INVALID_CMD,
    PARSE_INVALID_PIPE,
    PARSE_INVALID_DELAY,
    PARSE_NEGATIVE_DELAY,
    PARSE_DELAY_OUT_OF_RANGE,
    PARSE_CMD_OUT_OF_RANGE
} ParseCode;

// struct for handling a shell command
typedef struct {
    char **args;               // array to store parsed tokens
    int background;            // flag for if this command runs in the background
    int input_redirect;        // flag for if this command uses input redirection 
    int output_redirect;       // flag for if this command uses output redirection 
    int append_redirect;       // flag for if this command uses append redirection  
    int has_builtin;           // flag for if this command has a built-in command
    int index;                 // index of < token in args if it is present
    int outdex;                // index of > token in args if it is present
    int appenddex;             // index of >> token in args if it is present
} SHrimpCommand; 

// struct for holding all shell commands in a line of input, separated by semi colons
typedef struct {
    char *commands[MAX_COMMANDS];   // array of all commands in a line of input
    int command_amt;                // amount of commands in a line of input
} Commands;

// struct for holding the parsed command pipeline to execute
typedef struct {    
    SHrimpCommand *commands[MAX_COMMANDS];  // array of SHrimpCommand objects to execute sequentially
    int command_amt;                        // amount of commands in this pipeline
    int background;                         // flag for if this pipeline runs in the background
    int has_pipe;                           // flag for if this pipeline has at least one pipe
    int has_redirect;                       // flag for if this pipeline has at least one redirect token
    int has_builtin;                        // flag for if this pipeline has a built-in command
} Pipeline;

// struct to hold the current state of the shell
typedef struct {
    int job_number;  // job number counter
} SHrimpState;

#endif