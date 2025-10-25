# SeaShell

SeaShell is a custom Linux shell written in C.

SeaShell supports the following features:

  1. The built-in commands cd and exit.
  
  2. All simple UNIX commands.
 
  3. Commands running in the background using &. The shell
     will display the job number and pid of the background
     process.
  
  4. Input redirection with < and output redirection with 
     either > or >>. Input and output redirection can be
     specified within the same command in either order.
 
  5. Commands with a single pipe.

  6. Delayed commands using the prefix "delay" with a number of seconds.

The basic structure of the shell is an infinite while loop that:
  
  1. Displays a shell prompt.
 
  2. Waits for the user to enter input. 
  
  3. Reads the command line.
 
  4. Parses the command line.
 
  5. Takes the appropriate action.

## Installation

Download the source C file and compile it using your C compiler of choice.
