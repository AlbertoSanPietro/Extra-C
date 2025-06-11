#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>


#define SIZE 1024
#define MAX_ARGS 25

void reopen_stdin();
void skip_whitespace(char **);
void parse(char *);


int builtin_cd(char **);
int builtin_exit(char **);
int builtin_pwd(char **);
int builtin_echo(char **);


void handle_commands(char **);


extern char **environ;
typedef int (*builtin_func)(char ** argv); //takes argv returns exit status

struct BuiltinCommand{
	const char *name;
	builtin_func func;
};

struct BuiltinCommand builtins[] = {
	{"cd", builtin_cd},
	{"exit", builtin_exit},
	{"pwd", builtin_pwd},
	{"echo", builtin_echo},
	{NULL, NULL} //marks the end of the array
};


int builtin_exit(char **input) {
	if (input[1] == NULL) {
		exit(0);
	}
	char *end;

	int e_code = strtol(input[1], &end, 10);
	if(end == input[1] || *end !='\0') {perror("strtol error"); exit(EXIT_FAILURE);}
	exit(e_code);
} 


int builtin_cd(char **input) {
    static char *prev_dir = NULL;
    char *target_dir = input[1];
    char cwd[PATH_MAX];
		
			if (target_dir == NULL) {
        target_dir = getenv("HOME");
        if (target_dir == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }

	}	else if (strcmp(target_dir, "-") == 0) {
        if (prev_dir == NULL) {
            fprintf(stderr, "cd: no previous directory\n");
            return 1;
        }
				target_dir = prev_dir;
	}


	if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("cd");
        return 1;
    }
		char *swap=cwd;

	if (chdir(target_dir) != 0) {
        perror("cd");
        return 1;
    }
	free(prev_dir);

	prev_dir = strdup(swap);

	if (prev_dir == NULL) {
        perror("cd: strdup");
        return 1;
    }
	return 0;
}


int builtin_pwd(char **input) {
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd))==NULL){
		return 1;
	}
	else {
		printf("%s\n", cwd);
	}

return 0;
}

int builtin_echo(char **input) {
	if (input[1]== NULL) {
		printf("\n");
		return 0;
	}
	printf("%s",input[1]);
	size_t i=2;
	while(input[i]!=NULL) {
		printf(" %s", input[i]);
		i++;
	}
	printf("\n");

	return 0;
}



void reopen_stdin() {
	fclose(stdin);
	stdin=fopen("/dev/tty", "r");
}

void skip_whitespace(char **buffer_ptr) {
	while(isspace(**buffer_ptr)) {
		(**buffer_ptr)++;
	}
}

int is_empty_or_whitespace(const char *str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return 0; // Not empty or whitespace
        }
        str++;
    }
    return 1; // String is empty or contains only whitespace
}




void parse(char *input_string){
	input_string[strcspn(input_string, "\n")] = 0;
	
	if (strlen(input_string) > 0) {
	char *current_buffer=input_string;
	char *argv[MAX_ARGS];
	
	size_t i=0;
	char *token =	strtok(current_buffer, " \t\n");

	while(token!=NULL && i< MAX_ARGS -1) {
		argv[i++]= token;	
		token = strtok(NULL, " \t\n");

	}
	argv[i]=NULL;
	
	if (argv[0] == NULL) {return;}

	int command_status=0;
	for (size_t j=0; builtins[j].name != NULL; j++) {
		
		if (strcmp(argv[0], builtins[j].name) == 0) {
				command_status = builtins[j].func(argv);

				if (command_status == -1) {
					exit(EXIT_FAILURE);
				}
				return;
		}

	}
	handle_commands(argv);


	} else {
		return;
	}
}

void handle_commands(char **argv) {
	pid_t pid;
	char *path=getenv("PATH");
	if (path == NULL) {
		puts("Global variable PATH not set. exiting...");
		exit(EXIT_FAILURE);
	}
		
	char *path_copy=(strdup(path));
	if(path_copy == NULL) {perror("strdup failure"); exit(EXIT_FAILURE);}
	
	char full_path[PATH_MAX];
	int found =0;

	char *dir_token = strtok(path_copy, ":");
	//snprintf(full_command_path, sizeof(full_command_path), "%s/%s", dir_token, argv[0]);
	while (dir_token != NULL) {

		snprintf(full_path, sizeof(full_path), "%s/%s", dir_token, argv[0]);
	
	if (access(full_path, F_OK | X_OK) == 0) {
		found = 1;
		break;
	}
//free(path_copy)
//I wanna check if this leaks...

	dir_token=strtok(NULL, ":");
	}
	if (!found) {puts("Command not found"); return;}

	pid = fork();
	
	if (pid == -1) {
		//continue execution, but return. 
		return;
	} else if (pid==0) { //child process!
		
		execve(full_path, argv, environ);	
		perror("execve failed");
    exit(EXIT_FAILURE);	

	} else { //parent process
		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {printf("\nchild exited with status:%d\n",	WEXITSTATUS(status));}
		else if (WIFSIGNALED(status)) {printf("\nchild was killed by signal:%d\n", WTERMSIG(status));}

	}
}




int main() {
	char buff[SIZE];
	for(;;) {
		printf("MyShell>\t");
		fflush(stdout);
		char *input = fgets(buff, SIZE, stdin);
		

		if(input == NULL) {
			if(feof(stdin)) {puts("EOF detected"); reopen_stdin();}
			else {perror("fgets error"); exit(EXIT_FAILURE);}
		} else {
				char *buff_copy = buff;
				if (is_empty_or_whitespace(buff_copy)) {continue;}
					parse(buff);
			}
	
	}	


	return 0;
}



