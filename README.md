# CS350_Userspace

These are various userspace applications created over the course of CS 350 - Compilers

Make_Sort_Number
- This folder has two simple programs; one that generates random numbers, and another that sorts them.

MyShell
- This is a a shell created in C. The shell operates in the following way: when you type in a command, the shell creates a child process that executes the command you entered and then prompts for more user input when it has finished. The shell parses user input and executes the requested command using various system calls. While some commands are built-in, many more functions are supported via the execvp system call. Te shell also supports background jobs and batch mode. In batch mode, the shell is started by specifying a batch file on its command line; the batch file contains the same list of commands as you would have typed in the interactive mode. The shell executes this batch of commands together, as they would have been executed in interactive mode. The executable to run the shell is ./myshell

SimpleFS
- This is a simplified version of the Unix File System. There are three components to the program: The Shell, File System, and Disk Emulator. The Shell and Disk Emulator were provided to us, and we were required to implement the File System. This system is centered around dividing the disk into fixed size (4KB) blocks. Some of the supported operations by the File SYstem include mounting and formating a disk, creating and removing files, and writing to and reading from files.
- The SimpleFS/data folder has a few disk images to run the file system on.
- For example, to use the disk image image.5, we can run the following command from the SimpleFS/src folder:
- ./sfssh ../data/image.5 5
- Or to start with a fresh new disk image
