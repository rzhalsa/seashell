/* seashell.c
 *
 * SeaShell is a UNIX shell which supports the following features:
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
 *     1. Lines 136, 272, 306, 313, 381, 394, 404, 424, 489, 498, 507, 568, and 587
 *        were written with the help of ChatGPT.
 *
 *            Specifically:
 *
 *                1. The strtok() function and the delimeters for it in parse_input.
 *
 *                2. The isatty() function in main and get_input.
 *
 *                3. The dup2() function in pipe_command and redirect.
 *
 *                4. The strdup() function in enqueue.
 *
 *     2. Many of the error messages in SeaShell are copies of the
 *        same messages in Bash.
 *
 *     3. The ASCII art shown on startup was obtained from patorjk.com.
 *
 *     4. The idea to use void(signo) to suppress the compilation warning message
 *        concerning signo being an unused variable came from Stack Overflow.
 *
 * Known Issues:
 *     
 *    No known issues as of 4/29/25, though I will admit I have not bug tested the new delay
 *    feature to the same extent I bug tested the original shell creation. Please let me know
 *    if you find any bugs during testing, as I plan to continue developing this shell even after
 *    our OS class ends.
 *
 * Other Notes:
 *
 *     1. During development, seashell.c was compiled using the following line:
 *        
 *        gcc -std=c11 -o seashell seashell.c -Wall -Wextra -lphread
 *
 *        No compilation errors or warnings were observed as of April 29th, 2025.
 *
 * Author: Ryan McHenry
 * Date: March 21, 2025
 */

#define MAX_ARGS 64
#include <sys/types.h>   // pid_t, size_t, ssize_t
#include <sys/wait.h>    // waitpid(), WNOHANG
#include <unistd.h>      // fork(), execvp(), pipe(), isatty(), dup2(), ...
#include <stdlib.h>      // exit(), free(), getenv(), malloc()
#include <string.h>      // strtok(), strcmp(), strdup()
#include <stdio.h>       // printf(), perror(), getline(), fflush(), ...
#include <fcntl.h>       // O_RDONLY, O_CREAT, O_WRONLY, O_TRUNC, O_APPEND
#include <signal.h>      // SIGCHLD, signal()
#include <errno.h>       // errno, EINTR
#include <pthread.h>     // pthread_create(), pthread_join(), pthread_exit()
#include <time.h>        // time(), time_t

// structs for handling delayed commands
struct DelayedCommand {
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
};
struct ThreadQueue {
    struct DelayedCommand commands[MAX_ARGS];
    int command_amt;
};

// function prototypes
void startup();
void reset_vars(int *, int *, int *, int *, int *, int *, int *, int *, char **, int *,
                struct DelayedCommand *);
char *get_input(int);
ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream);
char *strdup(const char *);
void parse_input(char *, char **, int *, int *, struct DelayedCommand *, struct ThreadQueue *);
void init_queue(struct ThreadQueue *);
void enqueue(struct DelayedCommand *, struct ThreadQueue *, int);
void *poll(void *);
void check_redirection(char **, int *, int *, int *, int *,int *, int *);
void redirect(char **, int, int, int, int, int, int);
void check_piping(char **, int *, int *);
void pipe_command(char **, int, int, int, int,  int, int,  int, int);
void execute_command(char **, int, int, int, int, int, int, int, int, int);
int cd(char **);
int exec_unix_command(char **, int, int, int, int, int , int , int, int, int);
void sig_handler(int);

// globals
int job_number = 1;           // global job number counter
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 


//======================================================================================


int main() {
    if(isatty(STDIN_FILENO))
        startup();

    // Vars
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
    int display = 1;           // flag to display the SeaShell prompt or not
    int delay;                 // flag for if a command is delayed
    struct DelayedCommand cmd; // command to be delayed
    struct ThreadQueue queue;  // queue for holding delayed commands

    // Allocate memory for args
    args = malloc(MAX_ARGS * sizeof(char *));
    cmd.args = malloc(MAX_ARGS * sizeof(char *));

    // Set up handler to catch child processes in order to prevent zombies
    signal(SIGCHLD, sig_handler);

    // Initiate queue
    init_queue(&queue);

    // Init background thread running the function poll()
    pthread_t p1;
    pthread_create(&p1, NULL, poll, &queue);

    // Main loop of the shell
    while(1) {
        reset_vars(&index, &outdex, &appenddex, &background,
                   &input_redirect, &output_redirect, &append_redirect,
                   &pipe_flag, args, &delay, &cmd);
        
        input = get_input(display);
        if(input == NULL){
            if(errno == EINTR) {
                display = 0;
                clearerr(stdin);
                continue;
            }
            break;
        }

        display = 1;

        parse_input(input, args, &background, &delay, &cmd, &queue);
        if(args[0] == NULL || delay == 1)
            continue; 
        if(delay == -1) {
            printf("delay: provide delay amount in seconds\n");
            continue;
        }  

        check_redirection(args, &input_redirect, &output_redirect,
                          &append_redirect, &index, &outdex, &appenddex);

        check_piping(args,  &pipe_flag, &pipedex);

        execute_command(args,  background, input_redirect, output_redirect, append_redirect,
                    index, outdex, appenddex, pipe_flag, pipedex);        
    }
    
    free(args);
    free(cmd.args);
    pthread_join(p1, NULL);

    return 0;
}


//======================================================================================


void startup() {
    printf("\n Welcome to\n");

    printf(
        "  ____  ____   __   ____  _  _  ____  __    __  \n "
        "/ ___)(  __) /__\\ / ___)/ )( \\(  __)(  )  (  )  \n"
        " \\___ \\ ) _) /    \\___  \\) __ ( )__) / (_/\\/ (_/\\ \n"
        " (____/(____)\\_/\\_/(____/\\_)(_/(____)\\____/\\____/ \n\n");

    printf(" Made by Ryan McHenry\n\n");
}


//======================================================================================


void reset_vars(int *index, int *outdex, int *appenddex, int *background, int *input_redirect,
                int *output_redirect, int *append_redirect, int *pipe_flag, char **args,
                int *delay, struct DelayedCommand *cmd) {
    *index = -1;
    *outdex = -1;
    *appenddex = -1;
    *background = 0;
    *input_redirect = 0;
    *output_redirect = 0;
    *append_redirect = 0;
    *pipe_flag = 0;
    errno = 0;
    *delay = 0;

    // cmd vars
    cmd->delay_amt = 0;
    cmd->index = -1;
    cmd->outdex = -1;
    cmd->appenddex = -1;
    cmd->background = 0;
    cmd->input_redirect = 0;
    cmd->output_redirect = 0;
    cmd->append_redirect = 0;
    cmd->pipe_flag = 0;

    for(int i = 0; i < MAX_ARGS; i++) {
        args[i] = NULL;
        cmd->args[i] = NULL;
    }
}


//======================================================================================


char *get_input(int display) {
    size_t buf_size = 0;    // size of buffer (dynamically resized)
    char *buffer = NULL;    // buffer to store read values and cwd

    // Display user prompt
    if(isatty(STDIN_FILENO) && display) {
        printf("SeaShell:%s> ", getcwd(buffer, buf_size));
        //printf("SeaShell> ");
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


void parse_input(char *input, char **args, int *background, int *delay, 
                 struct DelayedCommand *cmd, struct ThreadQueue *queue) {
    char *token = NULL;      // string to store tokens
    int token_counter = 0;   // indexer for args

    // Parse the command as args[0]
    token = strtok(input, " \t\n");
    if(token == NULL)
        return;
    args[token_counter++] = token;

    // Parse arguments if they exist
    while(token != NULL) {
        token = strtok(NULL, " \t\n");
        args[token_counter++] = token;
    }     

    // Remove & char from args and set command to run in background
    if(strcmp(args[token_counter - 2], "&") == 0) {
        *background = 1;
        cmd->background = 1;
        args[token_counter - 2] = NULL;
        token_counter--;
    } 

    // Check if command is delayed and extract delay amount if it is
    if(strcmp(args[0], "delay") == 0) {
        if(args[1] == NULL) {
            *delay = -1;
            return;
        }

        int delay_time = atoi(args[1]);

        if(delay_time <= 0) {
            *delay = -1;
        } else {
            *delay = 1;
            cmd->delay_amt = delay_time + time(NULL);

            if(cmd->delay_amt == time(NULL))
                *delay = -1;

            for(int i = 2; args[i] != NULL; i++)
                args[i - 2] = args[i];

            token_counter -= 2;

            args[token_counter] = NULL;
            args[token_counter - 1] = NULL;

            token_counter--;

            cmd->args = args;

            pthread_mutex_lock(&mutex);
            enqueue(cmd, queue, token_counter);
            pthread_mutex_unlock(&mutex);
        }     
    }
}


//======================================================================================


void init_queue(struct ThreadQueue *queue) {
    queue->command_amt = 0;
}


//======================================================================================


void enqueue(struct DelayedCommand *cmd, struct ThreadQueue *queue, int token_counter) {
    int inserted = 0;

    if(queue->command_amt == 0) {
        queue->commands[0] = *cmd;
        queue->commands[0].args = malloc(MAX_ARGS * sizeof(char *));
        for(int i = 0; i < token_counter; i++) {
            queue->commands[0].args[i] = strdup(cmd->args[i]);
        }
        queue->command_amt++;
    } else {
        for(int i = 0; i < queue->command_amt; i++) {
            if(cmd->delay_amt < queue->commands[i].delay_amt && queue->command_amt < MAX_ARGS) {
                inserted = 1;
                for(int j = queue->command_amt - 1; j >= i; j--) {
                    queue->commands[j + 1] = queue->commands[j];
                }
                queue->commands[i] = *cmd;
                queue->commands[i].args = malloc(MAX_ARGS * sizeof(char *));
                for(int k = 0; k < token_counter; k++) {
                    queue->commands[i].args[k] = strdup(cmd->args[k]);
                }
                queue->command_amt++;
                break;
            }
        }
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


void *poll(void *t) {
    struct ThreadQueue *queue = ((struct ThreadQueue *) t);

    while(1) {
        sleep(1);

        pthread_mutex_lock(&mutex);
        if(queue->commands[0].args != NULL) {
            if(queue->commands[0].delay_amt < time(NULL)) {
                check_redirection(queue->commands[0].args, &queue->commands[0].input_redirect,
                                  &queue->commands[0].output_redirect, &queue->commands[0].append_redirect,
                                  &queue->commands[0].index, &queue->commands[0].outdex,
                                  &queue->commands[0].appenddex);
                
                check_piping(queue->commands[0].args, &queue->commands[0].pipe_flag,
                             &queue->commands[0].pipedex);

                execute_command(queue->commands[0].args, queue->commands[0].background,
                                queue->commands[0].input_redirect, queue->commands[0].output_redirect,
                                queue->commands[0].append_redirect, queue->commands[0].index,
                                queue->commands[0].outdex, queue->commands[0].appenddex,
                                queue->commands[0].pipe_flag, queue->commands[0].pipedex);

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
        pthread_mutex_unlock(&mutex);
    }
}


//======================================================================================


void check_redirection(char **args, int *input_redirect, int *output_redirect,
                       int *append_redirect, int *index,int *outdex, int *appenddex) {
    for(int i = 0; i < MAX_ARGS; i++) {
        if(args[i] == NULL)
            return;
        if(strcmp(args[i], "<") == 0) {
            *input_redirect = 1;
            *index = i;
        } 
        if(strcmp(args[i], ">") == 0) {
            *output_redirect = 1;
            *outdex = i;
        } 
        if(strcmp(args[i], ">>") == 0) {
            *append_redirect = 1;
            *appenddex = i;
        }
    }
}


//======================================================================================


void redirect(char **args, int input_redirect, int output_redirect, int append_redirect,
                int index, int outdex, int appenddex) {
    if(input_redirect == 1) {
        int in = open(args[index + 1], O_RDONLY, 0666);
        close(STDIN_FILENO);
        dup2(in, STDIN_FILENO);
        close(in);
        args[index] = NULL;
        args[index + 1] = NULL;
    }

    if(output_redirect == 1) {
        int out = open(args[outdex + 1], O_CREAT | O_WRONLY | O_TRUNC, 0666);  
        close(STDOUT_FILENO);
        dup2(out, STDOUT_FILENO);
        close(out);
        args[outdex] = NULL;
        args[outdex + 1] = NULL;    
    }

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


void check_piping(char **args, int *pipe_flag, int *pipedex) {
    for(int i = 0; i < MAX_ARGS; i++) {
        if(args[i] == NULL)
            return;
        if(strcmp(args[i], "|") == 0) {
            *pipe_flag = 1;
            *pipedex = i;
            return;
        }
    }
}

//======================================================================================


void pipe_command(char **args, int pipedex, int background, int input_redirect, int output_redirect,
                  int append_redirect, int index,  int outdex, int appenddex) {
    int fd[2];                      // file descriptors
    int offset = (pipedex + 1);     // offset for indexing redirection after pipe
    char **command_1;               // command before pipe
    char **command_2;               // command after pipe
    int command_2_counter = 0;      // counter to index command_2 properly

    command_1 = malloc(MAX_ARGS * sizeof(char *));
    command_2 = malloc(MAX_ARGS * sizeof(char *));

    for(int i = 0; i < pipedex; i++) {
        command_1[i] = args[i];
    }

    for(int j = offset; j < MAX_ARGS; j++) {
        if(args[j] == NULL)
            break;
        command_2[command_2_counter++] = args[j];
    }

    command_1[pipedex] = NULL;
    command_2[command_2_counter] = NULL;

    if(pipe(fd) < 0) {
        perror("pipe failed");
        exit(1);
    }

    pid_t pid1 = fork();
    if(pid1 < 0) {
        perror("first fork failed");
        exit(1);
    } else if(pid1 == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        if(input_redirect == 1 && index < pipedex) {
            redirect(command_1, input_redirect, output_redirect, append_redirect, 
                index, outdex, appenddex);
        } 

        execvp(command_1[0], command_1);
        printf("%s: command not found\n", command_1[0]); 
        exit(1);
    } 

    pid_t pid2 = fork();
    if(pid2 < 0) {
        perror("second fork failed");
        exit(1);
    } else if(pid2 == 0) {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

        if(output_redirect == 1 && outdex > pipedex) {
            redirect(command_2, input_redirect, output_redirect, append_redirect, 
                        index, outdex - offset, appenddex);
        }

        if(append_redirect == 1 && appenddex > pipedex) {
            redirect(command_2, input_redirect, output_redirect, append_redirect, 
                        index, outdex, appenddex - offset);
        }

        execvp(command_2[0], command_2);
        printf("%s: command not found\n", command_2[0]); 
        exit(1);
    } 

    // Parent
    close(fd[0]);
    close(fd[1]);
    if(background == 0) {
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    } else {
        printf("[%d] %d\n", job_number++, pid1);
        printf("[%d] %d\n", job_number++, pid2);
    }

    free(command_1);
    free(command_2);
}


//======================================================================================


void execute_command(char **args, int background, int input_redirect, int output_redirect,
                     int append_redirect, int index, int outdex, int appenddex, int pipe_flag,
                     int pipedex) {
    if(strcmp(args[0], "cd") == 0) {
        cd(args);
    } else if(strcmp(args[0], "exit") == 0) {
        exit(0);
    } else {
        exec_unix_command(args, background, input_redirect, output_redirect, 
            append_redirect, index, outdex, appenddex, pipe_flag, pipedex);
    }
}


//======================================================================================


int cd(char **args) {
    if(args[2] != NULL) {
        printf("cd: too many arguments\n");
        return 1;        
    } 

    // cd to home dir
    if(args[1] == NULL || strcmp(args[1], "~") == 0) {
        char *home = getenv("HOME");
        if(home != NULL) {
            if(chdir(home) == -1) {
                printf("seashell: cd home: No home directory found\n");
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
        printf("seashell: cd: %s: No such file or directory\n", args[1]);
        return 1;
    }

    return 0;
}


//======================================================================================


int exec_unix_command(char **args, int background, int input_redirect, int output_redirect,
                        int append_redirect, int index, int outdex, int appenddex, 
                        int pipe_flag, int pipedex) {
    // Pipe if applicable
    if(pipe_flag == 1) {
        pipe_command(args, pipedex, background, input_redirect, output_redirect, append_redirect,
                    index, outdex, appenddex);
        return 0;
    }

    pid_t pid = fork();
    if(pid == -1) {
        perror("fork: error while forking");
        exit(1);     
    } else if(pid == 0) {
        // Redirect if applicable
        if(input_redirect == 1 || output_redirect == 1 || append_redirect == 1) {
            redirect(args, input_redirect, output_redirect, append_redirect, 
                index, outdex, appenddex);
        } 

        // Execute args[0] using any arguments passed by user input
        execvp(args[0], args);
        printf("%s: command not found\n", args[0]); 
        exit(1); 
    } else {
        if(background == 0)
            waitpid(pid, NULL, 0);
        else { 
            printf("[%d] %d\n", job_number++, pid);
        }
    }

    return 0;
}


//======================================================================================


void sig_handler(int signo) {
    signal(SIGCHLD, sig_handler);

    // Suppress compilation warning message
    (void)signo; 

    // Prevent zombie process buildup
    while(waitpid(-1, NULL, WNOHANG) > 0);
}