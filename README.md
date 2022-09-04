# CS350_Userspace

These are various userspace applications created over the course of CS 350 - Compilers

Make_Sort_Number
- This folder has two simple programs; one that generates random numbers, and another that sorts them.

MyShell
- This is a a shell created in C. The shell operates in the following way: when you type in a command, the shell creates a child process that executes the command you entered and then prompts for more user input when it has finished. The shell parses user input and executes the requested command using various system calls. While some commands are built-in, many more functions are supported via the execvp system call. Te shell also supports background jobs and batch mode. In batch mode, the shell is started by specifying a batch file on its command line; the batch file contains the same list of commands as you would have typed in the interactive mode. The shell executes this batch of commands together, as they would have been executed in interactive mode.

SimpleFS
- tes
