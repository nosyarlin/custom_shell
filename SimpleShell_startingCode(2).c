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
static char **history;
static int c_counter;

// This function calls execvp to execute commands
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

// Allows changing of directory
int csh_cd(char **arg){
	// setting up homedir for later use
	struct passwd *pw = getpwuid(getuid());
	char *homedir = pw->pw_dir;

	if (arg[1] == NULL){
		printf("csh:expected input after cd\n");
	} else {
		if (arg[1][0] == '~'){
			arg[1]++; // removing the ~		
			strcat(homedir, arg[1]); // adding home directory to path
			arg[1] = homedir;
		}
		if (chdir(arg[1]) != 0){
			perror("csh");
		}
	}
	return 1; 
}

// Allows you to exit the shell
int csh_exit(char **arg){
	return 0;
}

// Allows you to view past 10 commands
int csh_history(char **arg){
	for (int i = 0; i < c_counter; ++i){
		printf("%d %s \n", i+1, history[i]);
	}
	return 1;
}

// Calls the last command
int csh_call_last(char **arg){
	return 1;
}

// List of built in commands
char *builtin_str[] = {
  "cd",
  "exit",
  "history",
  "!!"
};

// List of references to built in functions
int (*builtin_func[]) (char **) = {
	&csh_cd,
	&csh_exit,
	&csh_history,
	&csh_call_last
};

int num_builtin(){
	return sizeof(builtin_str)/sizeof(char *);
}

int main(int argc, char **argv){//start main
	char commandLine[MAX_INPUT];//to store users command
	char **command = (char **) malloc(MAX_INPUT);
	char *c;
	int status = 1;
	int position;
	history = malloc(10*sizeof(commandLine));
	c_counter = 0;
	//while loop to keep asking user for more inputs
	do {
		printf("csh>");
		fgets(commandLine,MAX_INPUT,stdin);//take input from user

		// add command to history
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

		// parse the input into tokens
		position = 0;
		c = strtok(commandLine, DELIMITER);
		while (c != NULL){
			command[position] = c;
			position++;
			c = strtok(NULL, DELIMITER);
		}
		command[position] = NULL;

		// execute command
		for (int i = 0; i < num_builtin(); ++i){
			if (strcmp(command[0], builtin_str[i]) == 0){
				status = (builtin_func[i])(command);
				break;
			} else if (i == num_builtin()-1){
				status = exec_non_builtin(command);
			}
		}

////--------------Case3, History-------------------------
//check if user enteres history option

	} while (status != 0);
	// free mem
	free(history);
	free(command);
}// end main



