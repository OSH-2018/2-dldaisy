
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>

extern char ** environ;

struct cmd_line {
	char command[256];
	char *args[128];
	char type;
};

char * cur[128];
int savein,saveout;


int main() {

	while (1) {

		struct cmd_line cmd;
		printf("# ");
		fflush(stdin);
		
		fgets(cmd.command, 256, stdin);

		int i;
		for (i = 0; cmd.command[i] != '\n'; i++)
			;
		cmd.command[i] = '\0';

		cmd.args[0] = cmd.command;
		int pipe_count = 0;//记录管道数目

		/*处理空格，同时记录cmd中管道的数目*/
		for (i = 0; *cmd.args[i]; i++) {
			if (*cmd.args[i] == '|') pipe_count++;
			for (cmd.args[i + 1] = cmd.args[i] + 1; *cmd.args[i + 1]; cmd.args[i + 1]++)
				if (*cmd.args[i + 1] == ' ') {
					*cmd.args[i + 1] = '\0';
					cmd.args[i + 1]++;
					break;
				}
		}
		cmd.args[i] = NULL;
		
		savein=dup(STDIN_FILENO);
		saveout=dup(STDOUT_FILENO);
		/*while 管道剩余，处理下一个子进程*/
		pid_t cpid;
		int pipefd[2];
		pipe(pipefd);	
		i=-1;
		while (pipe_count > 0) {

			cpid = fork();

			pipe_count--;

			//cur = SeekNext(cmd, cur);//寻找下一条指令的开头和结尾，存入指针数组cur
			int j = 0;
			i++;
			while (cmd.args[i] != NULL && *cmd.args[i] != '|') {
				cur[j] = cmd.args[i];
				i++;
				j++;
			}
			cur[j] = NULL;
			//
			if (cpid < 0) {
				perror("error");
				exit(1);
			}

			else if (cpid == 0) {
				//child
				close(pipefd[0]);
				dup2(pipefd[1], STDOUT_FILENO);
				close(pipefd[1]);

				if (!cur[0])
					exit(0);


				if (strcmp(cur[0], "cd") == 0) {
					if (cur[1])
						chdir(cur[1]);
					exit(0);
				}
				if (strcmp(cur[0], "pwd") == 0) {
					char wd[4096];
					puts(getcwd(wd, 4096));
					exit(0);
				}
				if (strcmp(cur[0], "exit") == 0)
					return 0;



				if (strcmp(cur[0], "env") == 0) {
					for (int k = 0; environ[k] != NULL; k++) {
						puts(environ[k]);
					}

					exit(0);

				}
				if (strcmp(cmd.args[0],"export") == 0) {
					char env_cmd[256];
					char * envp;
					envp = env_cmd;
					strcpy(env_cmd,cmd.args[1]);
		
					char name[256];
					char value[256];
		
					for(int k = 0; (*envp) != '='; k++) {
						name[k] = *envp;
						envp++;
					}
		
					envp++;
		
					for( int k= 0; (*envp) != '\0'; k++) {
						value[k] = *envp;
						envp++;
					}
		
					setenv(name,value,1);
					exit(0);
				//有待完善：用户输错的error处理
				}

				pid_t pid = fork();
				if (pid == 0) {

					execvp(cur[0], cur);

					return 255;
				}
				wait(NULL);
				exit(0);
			}
			else {
				//father
				close(pipefd[1]);
				dup2(pipefd[0], STDIN_FILENO);
				close(pipefd[0]);
				wait(NULL);
			}
		}

		// the last
		dup2(saveout, STDOUT_FILENO);
		close(saveout);

		int j = 0;
		i++;
		
		//seeknext
		while (cmd.args[i] != NULL && *cmd.args[i] != '|') {
			cur[j] = cmd.args[i];
			i++;
			j++;
		}
		cur[j] = NULL;


		if (!cur[0])
			continue;

		if (strcmp(cur[0], "cd") == 0) {
			if (cur[1])
				chdir(cur[1]);
			continue;
		}
		if (strcmp(cur[0], "pwd") == 0) {
			char wd[4096];
			puts(getcwd(wd, 4096));
			continue;
		}
		if (strcmp(cur[0], "exit") == 0)
			return 0;



		if (strcmp(cur[0], "env") == 0) {
			for (int k = 0; environ[k] != NULL; k++) {
				puts(environ[k]);
			}
			continue;
		}
		if (strcmp(cmd.args[0],"export") == 0) {
			char env_cmd[256];
			char * envp;
			envp = env_cmd;
			strcpy(env_cmd,cmd.args[1]);
		
			char name[256];
			char value[256];
		
			for(int k = 0; (*envp) != '='; k++) {
				name[k] = *envp;
				envp++;
			}
		
			envp++;
		
			for( int k= 0; (*envp) != '\0'; k++) {
				value[k] = *envp;
				envp++;
			}
		
			setenv(name,value,1);
			continue;
				//有待完善：用户输错的error处理
		}
		pid_t pid = fork();
		if (pid == 0) {

			execvp(cur[0], cur);

			return 255;
		}
		wait(NULL);

		dup2(savein, STDIN_FILENO);
		close(savein);
		continue;
}
}


