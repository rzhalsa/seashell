# SHrimp

SHrimp is a lightweight Linux shell written in C.

SHrimp currently supports the following features:

- The built-in commands cd and exit.
  
- All simple UNIX commands.
 
- Commands running in the background using &. (e.g. echo one two three &)
  
- Input redirection with < and output redirection with either > or >>. Input and output redirection can be specified within the same command in either order.
 
- Commands with an arbitrary amount of pipes. (e.g. echo one two three | grep one | wc -w)

- Running multiple commands in a single line separated by semicolons. (e.g. echo one; echo two; echo three)  

---

### Installation

Download the archive for the most recent release of SHrimp (currently **v0.5.2**) and extract it to your location of choice. To build the shell executable, navigate to the 
root directory (where you extracted SHrimp) and enter the following command in the terminal:

`make`

Once the executable binary is built, you can install it to your local bin directory by running:

`sudo make install`

If you would like to uninstall SHrimp from your machine, you can navigate to the SHrimp root directory and enter the following command:

`sudo make uninstall`

Once SHrimp is installed on your machine, you can run it from anywhere in the terminal by running:

`shrimp`  

Alternatively, if you would like to run SHrimp without installing it to your machine, after running `make` you will find the executable binary for the shell at /build/shrimp and can simply execute that.

---

### Future Planned Additions
- Command history
- Autocomplete by pressing the TAB key
- More built-in commands to add fun and unique quirks and/or capabilities to SHrimp  

---

### License
SHrimp is licensed under the [GPL-3.0](https://www.gnu.org/licenses/gpl-3.0.html) license.
