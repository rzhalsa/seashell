/* types.h
 *
 * Contains all type declarations used by the shell in one convenient location.
 *
 * Author: Ryan McHenry
 * Created: January 23, 2026
 * Last Modified: January 23, 2026
 */

#ifndef TYPES_H
#define TYPES_H

#include "config/macros.h" // MAX_ARGS
#include <time.h>          // time_t
#include <pthread.h>       // pthread_mutex_t

// struct for handling shell commands
typedef struct {
    char *input;               // string to store CLI input
    char **args;               // array to store parsed tokens
    int background;            // flag for if a command runs in background
    int input_redirect;        // flag for if a command uses input redirection 
    int output_redirect;       // flag for if a command uses output redirection 
    int append_redirect;       // flag for if a command uses append redirection  
    int index;                 // index of < token in args if it is present
    int outdex;                // index of > token in args if it is present
    int appenddex;             // index of >> token in args if it is present
    int pipe_flag;             // flag for if a command uses a pipe
    int pipedex;               // index of | token in args if it is present
    int delay;                 // flag for if a command is delayed
} SHrimpCommand; 

// structs for handling delayed commands
typedef struct {
    char **args;
    time_t delay_amt;
    int background;          
    int input_redirect;       
    int output_redirect;      
    int append_redirect;       
    int index;                
    int outdex;               
    int appenddex;          
    int pipe_flag;             
    int pipedex;               
} DelayedCommand;

typedef struct {
    DelayedCommand commands[MAX_DELAYED_COMMANDS];
    int command_amt;
} ThreadQueue;

// struct to hold the current state of the shell
typedef struct {
    int job_number;           // job number counter
    pthread_mutex_t mutex;    // mutex lock
} SHrimpState;

// struct to pack together a ThreadQueue and SHrimpState object for the poll() function
typedef struct {
    ThreadQueue *queue;
    SHrimpState *state;
} PollArgs;

#endif