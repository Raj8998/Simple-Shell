# Custom Shell Program

## Overview

This custom shell program, written in C, provides a basic command-line interface with several advanced features. It allows users to run standard shell commands, manage background processes, handle signal interruptions, and ensures that all commands are run as child processes of the shell.

## Features

### 1. Running Shell Commands

- The shell supports all standard shell commands.
- Commands can be executed with basic functionalities excpet, input/output redirection, piping, etc.

### 2. Background Command Execution

- Commands can be executed in the background by appending an `&` at the end of the command.
- Example:
  ```bash
  $ ls -l &
  ```

### 3. Background Command Queue

- The shell supports simultaneous execution of background commands.
- By default, up to 64 commands can be run in the background at the same time.
- This limit can be adjusted via command-line argument "--max-background-processes" if needed.

### 4. Parent-Child Command Execution Model

- The shell acts as the parent process for all commands.
- Each command is executed as a child process, ensuring isolation and better process management.
- Example:
    ```bash
    $ grep "search_term" file.txt
    ```
    In this example, grep runs as a child process of the shell.
### 5. Signal Handling for Long-running Commands
- The shell handles the CTRL+C shortcut to kill long-running commands.
- Signal handling is implemented in C to gracefully terminate the process.
- This prevents the shell itself from being terminated and allows users to stop only the current running command.

## Usage
1. Run the shell program:
    ```bash
    $ ./shell
    ```

2. List command-line argument and get help:
    ```bash
    $ ./shell -h/--help
    ```

3. Change default "max-background-processes" queue size:
    ```bash
    $ ./shell --max-background-processes [INT]
    ```

4. Execute a command:

    ```bash
    $ ls -l
    ```

5. Execute a command in the background:

    ```bash
    $ sleep 30 &
    ```

6. Terminate a long-running command with CTRL+C.

## Source Code Structure
- **src/shell.c**: The main source file containing the shell implementation.
- **src/run_shell_commands.h**: Header file for command execution functions.
- **src/tokenizer.h**: Header file for command parsing and tokenization functions.

## Compilation
- To compile the shell program, you can use the Makefile available in the "src" folder as follows:
    1. **Compile the program**
        ```bash
        $ make
        ```
    2. **Clean compiled files**
        ```bash
        $ make clean
        ```