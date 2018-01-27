#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>

// defining global variables
#define MAX_INPUT 8192
#define DELIMITER " \t\n\a\r"
static char *homedir;
static char **history;
static int c_counter;

// Parse a command line into tokens
char **parse_command(char *commandLine){
	int position = 0;
	char *c;
	char **command = (char **) malloc(MAX_INPUT);
	char temp[MAX_INPUT];

	// parse the input into tokens
	strcpy(temp, commandLine);
	c = strtok(temp, DELIMITER);
	while (c != NULL){
		command[position] = c;
		position++;
		c = strtok(NULL, DELIMITER);
	}
	// set last position as null
	command[position] = NULL;
	return command;
}

// This function calls execvp to execute non-builtin commands
int exec_non_builtin(char **args){
	pid_t pid, wpid;
	int status;
	pid = fork();	// -1 means forking failed
					// 0 means forking success
					// + means returned to parent or caller, contains child id
	if (pid == 0) {
		// execute child process
		if (execvp(args[0], args) == -1) {
			perror("csh");
		}
		exit(EXIT_FAILURE);

	} else if (pid < 0) { //error forking 
		perror("csh");

	} else { // parent process
		do { // wait for child process to be done before resuming
      		wpid = waitpid(pid, &status, WUNTRACED);
    	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

// List of built in commands
// I use this to check if a command is a builtin
char *builtin_str[] = {
  "cd",
  "exit",
  "history",
  "!!"
};

// Returns number of built in commands
int num_builtin(){
	return sizeof(builtin_str)/sizeof(char *);
}

// Allows changing of directory
int csh_cd(char **arg){
	struct passwd *pw;
	char *temp = malloc(MAX_INPUT);

	// setting up homedir for later use
	if (homedir == NULL){
		pw = getpwuid(getuid());
		homedir = pw->pw_dir;
	}
	strcpy(temp, homedir); // Copying homedir so that i won't accidentally modify it
	if (arg[1] == NULL){
		printf("csh:expected input after cd\n");
	} else {
		if (arg[1][0] == '~'){
			arg[1]++; // removing the ~		
			strcat(temp, arg[1]); // adding home directory to path
			arg[1] = temp;
		}
		if (chdir(arg[1]) != 0){	// if changing dir is unsuccessful
			perror("csh");
		}
	}
	free(temp);
	return 1; 
}

// Allows you to exit the shell
int csh_exit(char **arg){
	return 0; // return 0 basically just stops the program
}

// Allows you to view past 10 commands
int csh_history(char **arg){
	for (int i = c_counter-1; i > -1; --i){
		printf("%d %s \n", c_counter-i, history[i]);
	}
	return 1;
}

// Allows calling of last command
int csh_call_last(char **arg){
	char **command;
	int status;
	command = parse_command(history[c_counter-1]); // grab the latest command in history and parse it
	status = exec_command(command);
	return status;
}

// Allows calling of a command from history list
// arg[0] must be an int within range of 1 to c_counter
int csh_call_listed_command(char **arg){
	char **command;
	int status;
	int index;
	index = atoi(arg[0]);
	if (index > 0 && index <= c_counter){		
		command = parse_command(history[c_counter-index]);
		status = exec_command(command);
	} else {	// if there is no command corresponding to index
		printf("%s %d\n", "csh: index needs to be between 1 and", c_counter);
		status = 1;
	}
	
	return status;
}

// List of references to built in functions
int (*builtin_func[]) (char **) = {
	&csh_cd,
	&csh_exit,
	&csh_history,
	&csh_call_last
};

// Executes either using builtin function or using exec_non_builtin 
// This is what you actually call to run a command
int exec_command(char **command){
	int status;
	if (atoi(command[0]) != 0){
		status = csh_call_listed_command(command);
	} else {
		for (int i = 0; i < num_builtin(); ++i){ 			// check if it is a builtin command
			if (strcmp(command[0], builtin_str[i]) == 0){
				status = (builtin_func[i])(command);		// calls builtin command using function pointer
				break;
			} else if (i == num_builtin()-1){				// if not we use exec_non_builtin
				status = exec_non_builtin(command);
			}
		}
	}
	return status;
}

int main(int argc, char **argv){//start main
	char commandLine[MAX_INPUT];//to store users command
	char **command;
	int status = 1;
	history = (char **) malloc(10*sizeof(commandLine));
	c_counter = 0;

	// print shell details
	printf("%s\n", "Custom Shell version 0.0.1");
	printf("%s\n", "Copyright (C) 2018 Rayson Lim (1002026)");

	//while loop to keep asking user for more inputs
	do {
		printf("csh>");
		fgets(commandLine,MAX_INPUT,stdin);//take input from user

		// parse the input into tokens
		command = parse_command(commandLine);

		// add command to history if it is not "history" or "!!" or integer
		if (strcmp(command[0], "history") != 0 && strcmp(command[0], "!!") != 0 && atoi(command[0]) == 0){
			if (c_counter < 10){
				history[c_counter] = malloc(MAX_INPUT);
				strcpy(history[c_counter], commandLine);
				c_counter++;
			} else {
				free(history[0]); 	// freeing the memory that we don't need anymore
				history++; 			// removing first entry from history
				history[c_counter-1] = malloc(MAX_INPUT);
				strcpy(history[c_counter-1], commandLine); // adding new entry
			}
		}

		// execute command
		status = exec_command(command);

	} while (status != 0);

	// free mem
	for (int i = 0; i < c_counter; ++i){
		free(history[i]);
	}
	free(command);
}// end main



