/* delay.c
 *
 * Contains the logic for delaying commands with the "delay" prefix.
 *
 * Author: Ryan McHenry
 * Created: January 23, 2026
 * Last Modified: January 23, 2026
 */

#include <unistd.h>        // sleep()
#include <stdlib.h>        // malloc(), free()
#include <pthread.h>       // pthread_mutex_lock, pthread_mutex_unlock()
#include "types/types.h"   // DelayedCommand, ThreadQueue, PollArgs
#include "delay/delay.h"
#include "exec/exec.h"     // execute_delayed_command()
#include "exec/redirect.h" // check_delayed_redirection()
#include "exec/pipe.h"     // check_delayed_piping()

char *strdup(const char *);

//======================================================================================

/**
 * @brief Enqueues the DelayedCommand cmd to the ThreadQueue queue.
 *
 * @param cmd Pointer to the DelayedCommand struct variable cmd, which contains the delayed
 * command and all its corresponding data.
 * @param queue Pointer to the ThreadQueue struct variable queue which contains an array
 * of DelayedCommand struct variables.
 * @param token_counter The number of tokens in cmd.
 *
 * @details Enqueues cmd to queue. If queue is empty, the function enqueues cmd to 
 * queue->commands[0]. Otherwise, the function iterates through a for-loop and compares
 * the delay_amt value of cmd and queue->commands[i]. If the delay_amt of cmd is lower and
 * the queue is not full, the function inserts cmd into the queue. Otherwise, the function
 * enqueues cmd into the end of queue.
 */
void enqueue(DelayedCommand *cmd, ThreadQueue *queue, int token_counter) {
    int inserted = 0; // flag used so no command is inserted more than once

    /* the queue to hold delayed commands is ordered by the time when the delayed commands
     * will execute, that is, the command with the shortest remaining delay time will be
     * at the front of the queue, and the command with the longest remaining time until
     * execution will be at the end of the queue
     */

    // insert command into the queue if queue is empty
    if(queue->command_amt == 0) {
        queue->commands[0] = *cmd;
        queue->commands[0].args = malloc(MAX_ARGS * sizeof(char *));
        for(int i = 0; i < token_counter; i++) {
            queue->commands[0].args[i] = strdup(cmd->args[i]);
        }
        queue->command_amt++;
    } else {
        // Insert command into queue, insert location determined by delay amount
        for(int i = 0; i < queue->command_amt; i++) {
            if(cmd->delay_amt < queue->commands[i].delay_amt && queue->command_amt < MAX_ARGS) {
                inserted = 1; 
                // move all commands with longer delay amount one index back in queue
                for(int j = queue->command_amt - 1; j >= i; j--) {
                    queue->commands[j + 1] = queue->commands[j];
                }
                // insert new delayed command
                queue->commands[i] = *cmd;
                queue->commands[i].args = malloc(MAX_ARGS * sizeof(char *));
                for(int k = 0; k < token_counter; k++) {
                    queue->commands[i].args[k] = strdup(cmd->args[k]);
                }
                queue->command_amt++;
                break;
            }
        }
        // insert command into end of queue
        if(inserted == 0 && queue->command_amt < MAX_ARGS) {
            queue->commands[queue->command_amt] = *cmd;
            queue->commands[queue->command_amt].args = malloc(MAX_ARGS * sizeof(char *));
            for(int i = 0; i < token_counter; i++) {
                queue->commands[queue->command_amt].args[i] = strdup(cmd->args[i]);
            }
            queue->command_amt++;
        }
    }
}

//======================================================================================

/**
 * @brief Polls the ThreadQueue struct variable queue and executes delayed commands if
 * their delay time amount has passed.
 *
 * @param t Generic pointer passed from pthread_create which contains the ThreadQueue
 * struct variable queue.
 *
 * @details Obtains the queue by casting t with (struct ThreadQueue *). The function uses
 * a simple polling method where the function sleeps for 1 second each iteration through
 * the while loop. After, if the command at the front of the queue's delay time amount has
 * passed, the function executes the command using check_redirection(), check_piping(), and
 * finally execute_command().
 */
void *poll(void *t) {
    PollArgs *p_args = ((PollArgs *) t); 

    ThreadQueue *queue = p_args->queue;
    SHrimpState *state = p_args->state;

    while(1) {
        // Simple polling, sleep 1 second between each queue poll
        sleep(1);

        pthread_mutex_lock(&state->mutex);
        if(queue->command_amt > 0 && queue->commands[0].args != NULL) {
            // Execute command at front of queue if the delay time amount has passed
            if(queue->commands[0].delay_amt < time(NULL)) {
                check_delayed_redirection(&queue->commands[0]);
                check_delayed_piping(&queue->commands[0]);
                execute_delayed_command(&queue->commands[0], state);

                // Free memory addresses and also shift queue
                for(int i = 0; queue->commands[0].args[i] != NULL; i++) {
                    free(queue->commands[0].args[i]);
                }
                free(queue->commands[0].args);
                queue->commands[0].args = NULL;
                for(int i = 0; i < queue->command_amt - 1; i++) {
                    queue->commands[i] = queue->commands[i + 1];
                }
                queue->command_amt--;              
            }
        }
        pthread_mutex_unlock(&state->mutex);
    }

    free(p_args);
}

//======================================================================================