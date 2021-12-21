# TerminalBasedFileExplorer

A File explorer application which will run on terminal with simple cursor based UI (which can be controlled by keyboard)

This application will work in two modes :

    Normal Mode : used to explore the current directory and navigate the filesystem.

    Command Mode : used to enter and execute shell commands.

The root of the application is the directory where the application will start.

# How to execute

Open terminal and enter following commands :

    For Compilation : g++ main.cpp
    For Execute : ./a.out

# How to switch between Two Modes

Press : to switch from Normal Mode to Command Mode. Press Esc to switch from Command Mode to Normal Mode
Command Support

    Copy - copy <source_file(s)> <destination_directory>

    Move - move <source_file(s)> <destination_directory>

    Rename - rename <old_filename> <new_filename>

    Create File - create_file <file_name> <destination_path>

    Create Directory - create_dir <dir_name> <destination_path>

    Delete File - delete_file <file_path>

    Delete Directory - delete_dir <dir_path>

    Goto - goto

    Search - search <file_name> or search <directory_name>

# Assumptions

    Absolute path wrt application root will be given
    Terminal Resizing is not handeled so please do not resize the terminal after code execution (Full window size is recommended)
