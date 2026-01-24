/* parse.c
 *
 * Contains the logic for parsing user input on the CLI.
 *
 * Author: Ryan McHenry
 * Created: January 23, 2026
 * Last Modified: January 23, 2026
 */

#include <sys/types.h>   // ssize_t, size_t
#include <stdio.h>       // printf(), fflush(), feof(), perror()
#include <string.h>      // strtok(), strcmp()
#include <stdlib.h>      // atoi()
#include <unistd.h>      // isatty(), getcwd()
#include <pthread.h>     // pthread_mutex_lock(), pthread_mutex_unlock()
#include <errno.h>       // errno, EINTR
#include <time.h>        // time()
#include "types/types.h" // SHrimpCommand, DelayedCommand, ThreadQueue, SHrimpState
#include "delay/delay.h" // enqueue()
#include "parse/parse.h"

ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream);

//======================================================================================

/**
 * @brief Gets and returns the user input.
 *
 * @param display Integer flag to determine whether the shell prompt is displayed or not.
 *
 * @return A pointer to a char array consisting of the user input.
 *
 * @details Displays the user prompt for the shell if display is set to 1. The function then
 * reads the user input using getline(), and then trims off the newline character if present
 * by setting it to the null character.
 */
char *get_input(int display) {
    size_t buf_size = 0;    // size of buffer (dynamically resized)
    char *buffer = NULL;    // buffer to store read values and cwd

    // Display user prompt
    if(isatty(STDIN_FILENO) && display) {
        printf("SHrimp:%s> ", getcwd(buffer, buf_size));
        fflush(stdout);
    }

    // Read the user input and trim the newline off if present
    if(getline(&buffer, &buf_size,stdin) == -1) {
        if(feof(stdin)) {
            return NULL; 
        } else if(errno == EINTR) {
            return NULL;
        } else {
            perror("getline returned a value of -1");
            return NULL;
        }
    } 

    if(buffer[buf_size - 1] == '\n')
        buffer[buf_size - 1] = '\0';

    return buffer;
}

//======================================================================================

/**
 * @brief Parses the user input obtained in get_input().
 *
 * @param cmd SHrimpCommand object used to pass the obtained user input, and then
 * save the args of the command, whether the command runs in the background, and whether the
 * command is delayed.
 * @param del_cmd DelayedCommand object used to store the command if it is delayed.
 * @param queue ThreadQueue object used to enqueue the command stored to cmd if
 * the command is delayed.
 * @param state SHrimpState object used to access the shell's mutex lock 
 *
 * @details Parses the user input using strtok() and strcmp(). The function first
 * parses the command itself as args[0], and then parses all arguments if they exist.
 *
 * The function then checks if the user designates the command to run in the 
 * background using &. If yes, remove the & character from args, set the value of
 * the integer flag named background to be 1, and subtract 1 from token_counter.
 *
 * The function then checks if args[0] is "delay", which designates that the command
 * is delayed. If so, extract the delay amount using atoi(args[1]), calculate the
 * time the delayed command will run at as the sum of delay_time + time(NULL), then
 * remove "delay" and the delay time from args and subtract 2 from token_counter.
 * Finally, use enqueue() to add the DelayedCommand cmd to the ThreadQueue queue.
 */
void parse_input(SHrimpCommand *cmd, DelayedCommand *del_cmd, ThreadQueue *queue, SHrimpState *state) {
    char *token = NULL;      // string to store tokens
    int token_counter = 0;   // indexer for args

    // Parse the command as args[0]
    token = strtok(cmd->input, " \t\n");
    if(token == NULL)
        return;
    cmd->args[token_counter++] = token;

    // Parse arguments if they exist
    while(token != NULL) {
        token = strtok(NULL, " \t\n");
        cmd->args[token_counter++] = token;
    }     

    // Remove & char from args and set command to run in background
    if(strcmp(cmd->args[token_counter - 2], "&") == 0) {
        cmd->background = 1;
        del_cmd->background = 1;
        cmd->args[token_counter - 2] = NULL;
        token_counter--;
    } 

    // Check if command is delayed and extract delay amount if it is
    if(strcmp(cmd->args[0], "delay") == 0) {
        if(cmd->args[1] == NULL) {
            cmd->delay = -1;
            return;
        }

        int delay_time = atoi(cmd->args[1]);

        if(delay_time < 0) {
            cmd->delay = -1;
        } else {
            cmd->delay = 1;
            del_cmd->delay_amt = delay_time + time(NULL);

            // Remove "delay" and delay time from args
            for(int i = 2; cmd->args[i] != NULL; i++)
                cmd->args[i - 2] = cmd->args[i];

            token_counter -= 2;

            cmd->args[token_counter] = NULL;
            cmd->args[token_counter - 1] = NULL;

            token_counter--;

            del_cmd->args = cmd->args;

            // Enqueue cmd to queue
            pthread_mutex_lock(&state->mutex);
            enqueue(del_cmd, queue, token_counter);
            pthread_mutex_unlock(&state->mutex);
        }     
    }
}

//======================================================================================