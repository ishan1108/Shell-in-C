// Modify this file for your assignment
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_BUFFER_SIZE 80

int background = 0;
int getinput = 0;
int writeoutput = 0;
char* outputto;
char* inputfrom;

void clear_screen()
{
	printf("\e[1;1H\e[2J");
}

void kill_program()
{
	long pid;
	scanf("%ld", &pid);
	pid_t pid1 = (pid_t) pid;
	kill(pid1, SIGKILL);
	printf("Killed the program \n");
	return;
}

void initialize()
{
	clear_screen();
	printf("************************************************** \n");
	printf("________________WELCOME TO MY SHELL_______________ \n");
	printf("************************************************** \n");
	printf("\n");
	char* username = getenv("USER");
	printf("Username : %s \n", username);
}

void getWorkingDirectory()
{
	char directory[1024];
	getcwd(directory, sizeof(directory));
	printf("%s :", directory);
}

int getUserCommand(char* input)
{
	background = 0;
	printf(" mini-shell >> ");
	char buffer[MAX_BUFFER_SIZE];
	if(!fgets(buffer, MAX_BUFFER_SIZE, stdin))
		return -1;

	if(strlen(buffer) !=0)
	{
		strcpy(input, buffer);
		return 0;
	}
	else
		return 1;
}

int checkForPipe(char* input, char** commands)
{
	commands[0] = strtok(input, "|");
	commands[1] = strtok(NULL, "|");

	if(commands[1] == NULL)
		return 0; // No pipe found
	else
		return 1; //Found pipe

}

char* getOutputFile(char* input, char* outputto, int num)
{
	strtok(input, ">");
	outputto = strtok(NULL, " ");
	if(outputto == NULL)
		return NULL;
	writeoutput = 1;
	if(num == 2)
		writeoutput = 2;
	return outputto;
}

char* getInputFile(char* input, char* inputfrom, int num)
{
	strtok(input, "<");
	inputfrom = strtok(NULL, " ");
	if(inputfrom == NULL)
		return NULL;
	getinput = num;
	return inputfrom;
}

void parseCommand(char* input, char** command, int num)
{
	int i = 0;
	if(input[strlen(input) - 1] == '&')
	{
		input[strlen(input)-1] = '\0';
		background = 1;
	}

	inputfrom = getInputFile(input, inputfrom, num);
	outputto = getOutputFile(input, outputto, num);
	for(i = 0; i < MAX_BUFFER_SIZE; ++i)
	{
		command[i] = strsep(&input, " ");
		if(command[i] == NULL)
			break;
		if(strlen(command[i]) == 0)
			i--;
	}
	return;
}

int parseInput(char* input, char** before, char** after)
{
	int isPiped;
	char* commands[2];

	isPiped = checkForPipe(input, commands);
	int num = 0;
	if(isPiped == 0)
	{
		parseCommand(input, before, num);
	}
	else if(isPiped == 1)
	{
		parseCommand(commands[0], before, 0);
		parseCommand(commands[1], after, 1);
	}

	return isPiped;
}

void changeDir(char** before)
{
	int ret = chdir(before[1]);
	if(ret != 0)
		printf("Failed to Changed the directory \n");
	return;
}

void help()
{
	printf("Welcome to Help option \n");
	printf("Copyright @IshanPatel \n");
	printf("List of in-built Commands \n");
	printf("cd -> CHANGE DIRECTORY \n");
	printf("help -> HELP MENU \n");
	printf("exit -> EXIT THE PROGRAM \n");
	printf("kill -> KILL A PROGRAM \n");
	return;
}

void exit_shell()
{
	exit(0);
}

int checkForInBuiltCommand(char** before)
{
	int _switch;
	char* commands[4];

	commands[0] = "cd";
	commands[1] = "help";
	commands[2] = "exit";
	commands[3] = "kill";

	int i;
	for(i = 0; i < 4; ++i)
	{
		//printf("%s \n", commands [i]);
		//printf("%s \n", before[0]);
		if(strcmp(before[0], commands[i]) == 0)
		{
			_switch = i;
			break;
		}
		else
			_switch = 5;
	}

	switch(_switch)
	{
		case 0:
			changeDir(before);
			return 1;
		case 1:
			help();
			return 1;
		case 2:
			exit_shell();
			return 1;
		case 3:
			kill_program();
			return 1;
		default:
			break;
	}
	return 0;
}


void executeWithoutPipe(char** command)
{
	int child_status;
	pid_t pid = fork();
	int fd;
	int fd1;

	if(pid ==  0)
	{
        	if(background == 1)
        	{
                	setpgid(pid,0);
        	}

        	if(writeoutput == 1)
        	{
                	fd1 = open(outputto, O_CREAT | O_WRONLY | O_TRUNC,0644);
			perror("open");
			dup2(fd1, STDOUT_FILENO);
			perror("dup2");
			close(fd1);
		}

        	if(getinput == 1)
        	{
                	fd = open(inputfrom, O_RDONLY,0644);
			perror("open");
                	dup2(fd,STDIN_FILENO);
			perror("dup2");
                	close(fd);
        	}

		int exe = execvp(command[0], command);
		if (exe < 0)
		{
			printf("Could not execute the command. Try again\n");
			if(background == 0)
				exit(0);
		}
		close(fd1);
	}

	else if(pid < 0)
	{
		printf("Could not create a child process. Try again\n");
	}
	if(background == 0)
	{
		wait(&child_status);
	}
	return;
}


void executeWithPipe(char** command1, char** command2)
{
	int piped[2];
	pid_t pid1;
	pid_t pid2;
	int run_pipe;
	int fd, fd1;
	run_pipe = pipe(piped);
	if(run_pipe < 0)
	{
		printf("Failed to run pipe. Try again. \n");
		return;
	}

	pid1 = fork();

	if(pid1 == 0)
	{
		if(background == 1)
		{
			setpgid(pid1,0);
		}
	}

	if (pid1 < 0)
	{
		printf("Failed to create child process. Try again \n");
		return;
	}
	if( pid1 == 0)
	{
		if(writeoutput == 1)
		{
			open(outputto, O_CREAT | O_WRONLY | O_TRUNC, 0644);
			dup2(fd1,STDOUT_FILENO);
			close(fd1);
		}
		else
		{
			close(piped[0]);
			dup2(piped[1], STDOUT_FILENO);
			close(piped[1]);
		}

		if(getinput == 1)
		{
			fd = open(inputfrom, O_RDONLY, 0644);
			dup2(fd,STDIN_FILENO);
			close(fd);
		}

		if (execvp(command1[0], command1) < 0)
		{
			printf("Could not execute command 1. Try again \n");
			exit(0);
		}
		else
		{
			exit(0);
		}
	}
	else
	{
		pid2 = fork();
		if(pid2 < 0)
		{
			printf("Could not create second fork. Try again. \n");
			return;
		}

		if(pid2 == 0)
		{

			if(writeoutput == 2)
			{
				open(outputto, O_CREAT | O_WRONLY | O_TRUNC, 0644);
				dup2(fd1,STDOUT_FILENO);
				close(fd1);
			}


			if(getinput == 2)
			{
				fd = open(inputfrom, O_RDONLY, 0644);
				dup2(fd,STDIN_FILENO);
				close(fd);
			}
			else
			{
				close(piped[1]);
				close(0);
				dup2(piped[0], STDIN_FILENO);
				close(piped[0]);
			}
			int exe2 = execvp(command2[0], command2);
			if (exe2 < 0)
			{
				printf("Could not execute command 2. Try again \n");
				exit(1);
			}
			exit(0);
		}
		else
		{
			close(piped[0]);
			close(piped[1]);
			close(fd);
			close(fd1);
			if(background  == 0)
			{
				waitpid(pid1, NULL,0);
				waitpid(pid2, NULL,0);
			}
		}
	}
	if(getinput == 1)
                close(fd);
        if(writeoutput == 1)
                close(fd1);  
	return;
}

int shell_loop()
{
	char input[MAX_BUFFER_SIZE];
	char* command_before_pipe[MAX_BUFFER_SIZE];
	char* command_after_pipe[MAX_BUFFER_SIZE];

	int isParsed = 0;
	int isBuiltIn;

	while(1)
	{
		getWorkingDirectory();

		if(getUserCommand(input))
			continue;
		if(input[strlen(input) - 1] == '\n' && strlen(input) != 1)
		{
			input[strlen(input) -1] = '\0';
		}
		isParsed = parseInput(input, command_before_pipe, command_after_pipe);
		isBuiltIn = checkForInBuiltCommand(command_before_pipe);

		if(isBuiltIn == 0 && isParsed == 0)
		{
			executeWithoutPipe(command_before_pipe);
		}

		else if (isBuiltIn == 0 && isParsed == 1)
		{
			executeWithPipe(command_before_pipe, command_after_pipe);
		}
		getinput = 0;
		writeoutput = 0;
		background = 0;
		outputto = "\0";
		inputfrom = "\0";
	}

	return 0;
}


void sigint_handler(int sig)
{
	printf("\nMini-shell Terminated \n");
	kill(0,SIGINT);
	exit(0);
}


int main(){

	signal(SIGINT,sigint_handler);
	initialize();
	shell_loop();
	return 0;
}
