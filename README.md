# SHrimp

SHrimp is a modular Linux shell written in C.

SHrimp currently supports the following features:

  1. The built-in commands cd and exit.
  
  2. All simple UNIX commands.
 
  3. Commands running in the background using &. The shell
     will display the job number and pid of the background
     process.
  
  4. Input redirection with < and output redirection with 
     either > or >>. Input and output redirection can be
     specified within the same command in either order.
 
  5. Commands with a single pipe.

  6. Delayed commands using the prefix "delay" with a number of seconds. (e.g. delay 15 echo hi)

  7. Running multiple commands in a single line separated by semicolons. (e.g. echo one; echo two; echo three)

The basic structure of the shell is an infinite while loop that:
  
  1. Displays a shell prompt.
 
  2. Waits for the user to enter input. 
  
  3. Reads the command line.
 
  4. Parses the command line.
 
  5. Takes the appropriate action.

## Installation

Download the archive for the most recent release of SHrimp (currently **v0.4.0**) and extract it to your location of choice. To build the shell executable, navigate to the 
root directory (where you extracted SHrimp) and enter the following command in the terminal:

`make`

Once the executable binary is built, you can install it to your local bin directory by running:

`make install`

If you would like to uninstall SHrimp from your machine, you can navigate to the SHrimp root directory and enter the following command:

`make uninstall`

Once SHrimp is installed on your machine, you can run it from anywhere in the terminal by running:

`shrimp`

## Future Planned Additions
- Command history
- Support for multiple pipes
- Autocomplete by pressing the TAB key

## License
SHrimp is licensed under the [GPL-3.0](https://www.gnu.org/licenses/gpl-3.0.html) license.
