#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <malloc.h>

#define LINE_BUF_LEN 1000
#define COMMAND_OPTION_LEN 100
#define MAX_CMD 10

char split(char *line, char **cmd, char *array)
{
	int letter_count = 0, index = 0, offset = 0, ret_value = 0;
	char c, flag_a = 0, flag_b = 0;
	while((c = line[letter_count++]) != '\n') {
		if((c == '>') || (c == '<') || (c == '|') || (c == ' ' )) {
			cmd[index][offset] = 0;
			offset = 0;
			if(c != ' ') {
				array[1] = cmd[(array[0] = (index = (index + 1 - flag_a)))][0] = c;
				cmd[index++][1] = 0;
				flag_a = 0;
				flag_b = 1;
			} else {
				index = index + 1 - flag_b;
				flag_b = 0;
				flag_a = 1;
			}
		} else {
			flag_a = 0;
			flag_b = 0;
			cmd[index][offset++] = c;
		}
	}
	cmd[index][offset] = 0;
	return array[1];
}

void flush (char **cmd, int start, int end)
{
	do {
		end--;
		cmd[end][0] = 0;
	} while(end != start);
}

void exec(char **cmd, int start, int stop)
{
	int j = 0;
	char* CMD[MAX_CMD], i = 0;
	for(i = 0; i < MAX_CMD; i++)
		CMD[i] = 0;
	for(i = start; cmd[i][0] != 0 && i <= stop; i++,j++)
		CMD[j] = cmd[i];
	execlp(CMD[0],CMD[0],CMD[1],CMD[2],CMD[3],CMD[4],CMD[5],CMD[6],CMD[7],CMD[8],CMD[9], NULL);
}

flush_array(char *array, int size)
{
	int i = 0;
	for(;i<size; i++)
		array[i] = 0;
}

into_stdin(char **cmd, char c)
{
	int fd = 0;
	fd = open(cmd[c+1],O_RDONLY);
	close(0);
	dup(fd);
	exec(cmd, 0, c-1);

}

into_file(char **cmd, char c)
{
	int fd = 0;
	fd = open(cmd[c+1],O_WRONLY|O_CREAT|O_TRUNC, 0644);
	close(1);
	dup(fd);
	exec(cmd, 0, c-1);
}

piping(char **cmd, char c)
{
	int FD[2];
	pipe(FD);
	int pid1;
	if((pid1 = fork()) == 0) {
		close(1);
		dup(FD[1]);
		exec(cmd, 0, c-1);
		exit(0);
	} else {
		waitpid(pid1, NULL,0);
		close(FD[1]);	
		close(0);
		dup(FD[0]);
		exec(cmd,c+1,10);
	}
}

main()
{
	char line[LINE_BUF_LEN];
	char *cmd[MAX_CMD], array[10];
	int i = 0, ret = 0, pid = 0;
	for(;i < 10; i++)
		cmd[i] = malloc(COMMAND_OPTION_LEN);	
	while(1) {
		printf("\nMyShell~$ ");
		flush(cmd, 0,sizeof(*cmd));
		flush_array(array, sizeof(array));
		fgets(line,sizeof(line),stdin);
		if(!strcmp(line,"exit"))
			break;
		if (split(line, cmd, array) == 0) {
			if((pid = fork()) == 0)
				exec(cmd, 0, 10);
		} else if(array[1] == '<') {
			if((pid = fork()) == 0) 
				into_stdin(cmd, array[0]);
		} else if(array[1] == '>') {
			if((pid = fork()) == 0)
				into_file(cmd, array[0]);
		} else if(array[1] == '|') {
			if((pid = fork()) == 0)
				piping(cmd, array[0]);
		}	
		waitpid(pid,NULL,0);
	}
}

