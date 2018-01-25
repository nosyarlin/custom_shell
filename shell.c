int main(int argc, char **argv){
	// Run command loop
	cmd_loop();

	// Perform shutdown cleanup

	return EXIT_SUCCESS;
}

void cmd_loop(void){
	char *line;
	char **args;
	int status;

	do {
		printf("> ");
		line = cmd_read_line();
		args = cmd_split_line(line);
		status = cmd_execute(args);

		free(line);
		free(args);
	} while(status);
}

#define DEFAULT_BUFSIZE = 1024;
char *cmd_read_line(void){
	int bufsize = DEFAULT_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;

	// if buffer does not get allocated, print error and exit
	if(!buffer){
		fprintf(stderr, "cmd: allocation error\n");
		exit(EXIT_FAILURE);
	}

	// Main reading character loop
	while(true){
		c = getChar();
		if(c == EOF || c == '\n'){
			buffer[position] = '\0';
			return buffer;
		} else {
			buffer[position] = c;
		}
		position++;

		// If we exceed buffer, allocate more space
		if (position >= bufsize){
			bufsize += DEFAULT_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			// Again, is allocation fails
			if (!buffer){
				fprintf(stderr, "cmd: allocation error\n"
				exit(EXIT_FAILURE);
			}
		}
	}
}

