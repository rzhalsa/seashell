/* parse.c
 *
 * Contains the logic for parsing user input on the CLI.
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

#include <sys/types.h>     // ssize_t, size_t
#include <stdio.h>         // printf(), fflush(), feof(), perror()
#include <string.h>        // strtok(), strcmp()
#include <stdlib.h>        // atoi()
#include <unistd.h>        // isatty(), getcwd()
#include <pthread.h>       // pthread_mutex_lock(), pthread_mutex_unlock()
#include <errno.h>         // errno, EINTR
#include <time.h>          // time()
#include "config/macros.h" // ORANGE_TEXT, BLUE_TEXT, RED_TEXT, RESET_COLOR
#include "types/types.h"   // SHrimpCommand, SHrimpState
#include "utils/utils.h"
#include "parse/parse.h"

ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream);

//======================================================================================

/**
 * @brief Gets and returns the user input.
 *
 * @param display Integer flag to determine whether the SHrimp prompt is displayed or not.
 *
 * @return A pointer to a char array consisting of the user input.
 *
 * @details Displays the user prompt for the shell if display is set to 1. The function then
 * reads the user input using getline(), and then trims off the newline character if present
 * by setting it to the null character.
 */
char *get_input(int display) {
    size_t buf_size = 0;    // size of buffer (dynamically resized)
    ssize_t nread;          // number of bytes read by getline()
    char *buffer = NULL;    // buffer to store read values and cwd

    // Display user prompt
    if(isatty(STDIN_FILENO) && display) {
        char *usr_home = getenv("HOME");
        char cwd_sub[MAX_ARGS] = "";
        cwd_sub[0] = '\0';
        char *cwd = getcwd(buffer, buf_size);
        strncat(cwd_sub, cwd, strlen(usr_home));

        // See if the user's cwd starts with $HOME
        if(strcmp(usr_home, cwd_sub) == 0) { // user's cwd is within $HOME, replace $HOME with ~
            char dir[MAX_ARGS] = "~";
            int home_len = strlen(usr_home);
            strncat(dir, cwd + home_len, strlen(cwd) - home_len);
            printf(ORANGE_TEXT "SHrimp" RESET_COLOR ":" BLUE_TEXT "%s" RESET_COLOR "> ", dir);
        } else { // print whole cwd
            printf( ORANGE_TEXT "SHrimp" RESET_COLOR ":" BLUE_TEXT "%s" RESET_COLOR "> ", getcwd(buffer, buf_size));
        }
        
        fflush(stdout);
    }

    // Read the user input and trim the newline off if present
    nread = getline(&buffer, &buf_size,stdin);
    if(nread == -1) {
        if(feof(stdin)) {
            return NULL; 
        } else if(errno == EINTR) {
            return NULL;
        } else {
            perror(RED_TEXT "getline returned a value of -1" RESET_COLOR);
            return NULL;
        }
    } 

    if(nread > 0 && buffer[nread - 1] == '\n')
        buffer[nread - 1] = '\0';

    return buffer;
}

//======================================================================================

/**
 * @brief Splits up the raw input obtained in get_input() into separate commands, delimited by ;
 *
 * @param input the raw text input obtained in get_input()
 * @param cmds Commands object used to store the list of commands
 *
 * @details Splits up the raw input obtained in get_input() into separate commands, delimited by ;.
 * If a line of input does not posses any semi-colons, Commands only stores a single command to
 * execute.
 */
ParseCode parse_commands(char *input, Commands *cmds) {
    char *command;  // string to store commands

    command = strtok(input, ";");

    while(command != NULL) {
        cmds->commands[cmds->command_amt++] = command;
        command = strtok(NULL, ";");
    }    

    return PARSE_OK;
}

//======================================================================================

/**
 * @brief Parses the raw user input obtained in get_input() and stores it in a SHrimpCommand
 * object.
 *
 * @param input raw input string of the entered user command.
 * @param cmd SHrimpCommand object used to pass the obtained user input, and then
 * save the args of the command, whether the command runs in the background, and whether the
 * command is delayed.
 *
 * @details Parses the raw user input using strtok() and strcmp(). The function first
 * parses the command itself as args[0], and then parses all arguments if they exist.
 *
 * The function then checks if the user designates the command to run in the 
 * background using &. If yes, remove the & character from args, set the value of
 * the integer flag named background to be 1, and subtract 1 from token_counter.
 */
ParseCode parse_input(char *input, SHrimpCommand *cmd) {
    char *token = NULL;      // string to store tokens
    int token_counter = 0;   // indexer for args

    // Parse the command as args[0]
    token = strtok(input, " \t\n");
    if(token == NULL)
        return PARSE_INVALID_CMD;
    cmd->args[token_counter++] = token;

    // Parse arguments if they exist
    while(token != NULL) {
        token = strtok(NULL, " \t\n");
        cmd->args[token_counter++] = token;
    }     

    // Remove & char from args and set command to run in background
    if(strcmp(cmd->args[token_counter - 2], "&") == 0) {
        cmd->background = 1;
        cmd->args[token_counter - 2] = NULL;
        token_counter--;
    } 

    return PARSE_OK;
}

//======================================================================================