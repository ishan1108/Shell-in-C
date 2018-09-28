#include <stdio.h>
#include <string.h>
int main ()
{
	int MAXSIZE=80;	
	char input[MAXSIZE];
	fgets(input,MAXSIZE,stdin);
	char* SPACE= " ";
	char* PIPE= "|";
	char* token;
	token=strtok(input, SPACE);
	while(token!=NULL)
	{
		if(strcmp(token, PIPE) == 0)
		{
			token = strtok(NULL,SPACE);
		}
		else
		{
			printf("%s \n", token);
			token = strtok(NULL,SPACE);
		}
	}
	return 0;
}
