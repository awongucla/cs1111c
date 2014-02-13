// UCLA CS 111 Lab 1 command execution
//ALBERT WONG
#include "command.h"
#include "command-internals.h"


#include <error.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>

int g_num_depend = 0;

enum redir_type
{
	INPUT,
	OUTPUT,
};

struct file_redir
{
	int num;
	char* fileName;
	enum redir_type redir;
};

struct threadArgs
{
	int id;
	command_t c;
};

void getDependencies(struct file_redir** file_depend_list, command_t* depend_list, int index);
void zeroOutList(struct file_redir** fileList);
int** create_dependency_matrix(struct file_redir** file_depend_list);
int get_str_len(char* word);
void print_word( char* c);
void print_words(char** words, int numWords);
void print_command_list(command_t* commList, int length);
void print_enum(command_t c);
void print_dependencies(struct file_redir** file_depend_list);
void zero_adj_matrix(int** adj_matrix, int length);
void print_adj_matrix(int** adj_matrix, int length);
int str_equal(char* c1, char* c2);
int** create_dependency_matrix(struct file_redir** file_depend_list);
void handle_dependencies(command_t root);
void* paraFunc(void* args);
void getDependencies(struct file_redir** file_depend_list, command_t* depend_list, int index);
int command_status (command_t c);
int isExec(char* c);
void execSimpleCommand(command_t c);
void execPipe(command_t c);
void printCommType(enum command_type command);
void execOperatorCommand(command_t c);
void execute_command (command_t c, int time_travel);


int get_str_len(char* word)
{
	int counter = 0;
	//printf("counter: %d\n", counter);
	while(word[counter] != '\0')
	{
		//printf("counter: %d\n", counter);
		counter++;
	}
	return counter;
}

void print_word( char* c)
{
	int length = get_str_len(c);
	int i = 0;
	for(; i < length; i++)
	{
		char ch = c[i];
			
			printf("%c", ch);
	}
}

void print_words(char** words, int numWords)
{
	//printf("numWords: %d\n", numWords);
	int i;
	for(i = 0; i < numWords; i++)
	{
		char* word = words[i];
		int length = get_str_len(word);
		//printf("printWord1");
		//printWord(word);
		//printf("printWord2");
		//int length = strlen(word);
		int j;
		//printf("length: %d\n", length);
		for(j = 0; j < length; j++)
		{
			char c = word[j];
			
			printf("%c", c);
		}
		printf(" ");
	}
	printf("\n");
}

void print_command_list(command_t* commList, int length)
{

	int i;
	printf("printCommandList\n");
	printf("length: %d\n", length);
	//int k = commList[0].type == SIMPLE_COMMAND;
	//printf("k: %d\n", k);
	for(i = 0; i < length; i++)
	{
		printf("%d ", i);
		switch(commList[i]->type)
		{
			case AND_COMMAND: printf("AND_COMMAND\n"); break;
			case SEQUENCE_COMMAND: printf("SEQUENCE_COMMAND\n"); break;
			case OR_COMMAND: printf("OR_COMMAND\n"); break;
			case PIPE_COMMAND: printf("PIPE_COMMAND\n"); break;
			case SIMPLE_COMMAND: printf("SIMPLE_COMMAND\n"); //printf("commList[i].numWords: %d\n", commList[i]->numWords);
									 print_words(commList[i]->u.word, commList[i]->numWords); break;
				break;
			case OPEN_PAREN: printf("OPEN_PAREN\n"); break;
			case CLOSED_PAREN: printf("CLOSED_PAREN\n"); break;
			case LEFT_REDIRECT: printf("LEFT_REDIRECT\n"); break;
			case RIGHT_REDIRECT: printf("RIGHT_REDIRECT\n"); break;
			case NEW_LINE: printf("NEW_LINE\n"); break;
			case IGNORE_COMMAND: printf("IGNORE_COMMAND\n"); break;
			default: break;
		}

	}
	printf("\n");
}


void print_enum(command_t c)
{
		//printf("%d ", i);
		switch(c->type)
		{
			case AND_COMMAND: printf("AND_COMMAND\n"); break;
			case SEQUENCE_COMMAND: printf("SEQUENCE_COMMAND\n"); break;
			case OR_COMMAND: printf("OR_COMMAND\n"); break;
			case PIPE_COMMAND: printf("PIPE_COMMAND\n"); break;
			case SIMPLE_COMMAND: printf("SIMPLE_COMMAND\n"); //printf("commList[i].numWords: %d\n", commList[i].numWords);
									// printWords(commList[i].u.word, commList[i].numWords); break;
				break;
			case OPEN_PAREN: printf("OPEN_PAREN\n"); break;
			case CLOSED_PAREN: printf("CLOSED_PAREN\n"); break;
			case LEFT_REDIRECT: printf("LEFT_REDIRECT\n"); break;
			case RIGHT_REDIRECT: printf("RIGHT_REDIRECT\n"); break;
			case NEW_LINE: printf("NEW_LINE\n"); break;
			case IGNORE_COMMAND: printf("IGNORE_COMMAND\n"); break;
			default: break;
		}

	
	printf("\n");
}


void print_dependencies(struct file_redir** file_depend_list)
{
	int index;
	for(index = 0; index < g_num_depend; index++)
	{
		printf("%d ", index);
		int num = file_depend_list[index][0].num;
		int i;
		
		if(num == 0)
		{
			printf("no files\n");
			continue;
		}
		/*else
		{
			printf("num: %d\n", num);
		}*/
		for(i = 1; i < num+1; i++)
		{
			/*char* c = file_depend_list[index][i].fileName;

			if(c == 0)
			{
				printf("NULL: %d\n", index);
			}*/

			int length = get_str_len(file_depend_list[index][i].fileName);
		//	printf("length: %d\n", length);
			int j;
			for(j = 0; j < length; j++)
			{
				char c = file_depend_list[index][i].fileName[j];
				printf("%c", c);
			}
			if(file_depend_list[index][i].redir == INPUT)
			{
				printf(", INPUT\n");
			}
			else
			{
				printf(", OUTPUT\n");
			}
			
		}
	}
}


void zero_adj_matrix(int** adj_matrix, int length)
{
	int i = 0; 
	for(; i < length; i++)
	{
		int j = 0;
		for(; j < length+1; j++)
		{
			adj_matrix[i][j] = 0;
		}
	}
}

void print_adj_matrix(int** adj_matrix, int length)
{
	int i = 0;
	printf("   ");
	for(; i < length; i++)
	{
		printf("%d  ", i);
	}
	printf("\n");
	int j = 0;
	for(; j < length; j++)
	{
		int k = 0;
		printf("%d  ", j); 
		for(; k < length+1; k++)
		{
			printf("%d  ", adj_matrix[j][k]);
		}
		printf("\n");
	}
}

int str_equal(char* c1, char* c2)
{
	int i = 0;
	//printf("cmp "); print_word(c1); printf(" with "); print_word(c2);printf("\n");
	while(1)
	{
		//printf("%d and %d\n", c1[i], c2[i]);
		if(c1[i] != c2[i])
		{
			return 0;
		}
		if(c1[i] == '\0' && c2[i] == '\0')
		{
			return 1;
		}
		i++;
	}
}

int** create_dependency_matrix(struct file_redir** file_depend_list)
{
	int** adjGraph = malloc(sizeof(int*)*g_num_depend);
	int i = 0;


	for(; i < g_num_depend; i++)
	{
		adjGraph[i] = malloc(sizeof(int)*g_num_depend+1);
	}

	//print_adj_matrix(adjGraph, g_num_depend);
	
	zero_adj_matrix(adjGraph, g_num_depend);

	//print_adj_matrix(adjGraph, g_num_depend);

	int j = 0;
	for(; j < g_num_depend-1; j++)
	{
		
		int a = 1;
		for(; a< file_depend_list[j][0].num+1; a++)
		{
			struct file_redir file1 = file_depend_list[j][a];
			int k = j+1;
			for(; k < g_num_depend; k++)
			{
				int b = 1;
				for(; b < file_depend_list[k][0].num+1; b++)
				{
					struct file_redir file2 = file_depend_list[k][b];
					if(str_equal(file1.fileName, file2.fileName) && (file1.redir != INPUT || file2.redir != INPUT))
					{
						//printf("%d depends on %d\n", k, j);
						adjGraph[k][j] = 1;
						adjGraph[k][g_num_depend]++;
					}
				}
				
			}
		}
	}
	return adjGraph;
}

void handle_dependencies(command_t root)
{
	int depend_list_size = 10;
	int DFS_stack_size = 10;
	command_t* depend_list = malloc(sizeof(command_t)*depend_list_size);
	command_t* DFS_stack = malloc(sizeof(command_t)*DFS_stack_size);

	//int num_dependencies = 0;
	//print_enum(root);
	if(root->type == SEQUENCE_COMMAND)
	{
		DFS_stack[0] = root;
	}
	else
	{
		execute_command(root, 0);
		return;
		//printf("wtf");
		//return;
	}
	int stack_bot = 0; // bot of stack
	int stack_top = 1; // top of stack (first empty spot)


	while(stack_top-stack_bot > 0)
	{
		//print_command_list(depend_list, g_num_depend);
		if(stack_top >= DFS_stack_size-3)
		{
			DFS_stack_size *= 2;
			DFS_stack = realloc(DFS_stack, sizeof(command_t)*DFS_stack_size);
		}
		if(g_num_depend >= depend_list_size-3)
		{
			depend_list_size *= 2;
			depend_list = realloc(depend_list, sizeof(command_t)*depend_list_size);
		}

		if(DFS_stack[stack_top-1]->type != SEQUENCE_COMMAND)
		{
			depend_list[g_num_depend] = DFS_stack[stack_top-1];
			g_num_depend++;
			stack_top--;
		}
		else
		{
			command_t child1 = DFS_stack[stack_top-1]->u.command[1];
			command_t child2 = DFS_stack[stack_top-1]->u.command[0];
			stack_top--;
			if(child1->type != SEQUENCE_COMMAND && child2->type != SEQUENCE_COMMAND)
			{
				//printf("sequence command has two non sequence command children\n");
				depend_list[g_num_depend] = child2;
				g_num_depend++;
				depend_list[g_num_depend] = child1;
				g_num_depend++;
				//stack_top--;
				continue;
			}

			else if(child1->type == SEQUENCE_COMMAND && child2->type != SEQUENCE_COMMAND)
			{
				DFS_stack[stack_top] = child2;
				stack_top++;
				DFS_stack[stack_top] = child1;
				stack_top++;
			}
			else if(child1->type != SEQUENCE_COMMAND && child2->type == SEQUENCE_COMMAND)
			{
				DFS_stack[stack_top] = child1;
				stack_top++;
				DFS_stack[stack_top] = child2;
				stack_top++;
			}
			else
			{
				DFS_stack[stack_top] = child1;
				stack_top++;
				DFS_stack[stack_top] = child2;
				stack_top++;
			}
		}

		//stack_bot++;
	}
	
	//printf("g_num_depend: %d\n", g_num_depend);
	//print_command_list(depend_list, g_num_depend);
	struct file_redir** file_depend_list = malloc(sizeof(struct file_redir*)*g_num_depend);

	
	
	int index;
	for(index = 0; index < g_num_depend; index++)
	{
		//printf("index: %d\n", index);
		getDependencies(file_depend_list, depend_list, index);
	}
	//print_dependencies(file_depend_list);
	int** depend_matrix = create_dependency_matrix(file_depend_list);
	
	//print_adj_matrix(depend_matrix, g_num_depend);

	int threadsLeft = g_num_depend;

	int* independentList = malloc(sizeof(int)*g_num_depend);
	
	while(threadsLeft > 0)
	{
		int indeIndex = 0;
		
		int i = 0;
		for(; i < g_num_depend; i++)
		{
			// get indexes with no dependencies
			if(depend_matrix[i][g_num_depend] == 0)
			{
				independentList[indeIndex] = i;
				depend_matrix[i][g_num_depend]--;
			//	printf("i: %d\n", i);
				indeIndex++;
				threadsLeft--;
			}

		}
		pthread_t* threads = malloc(indeIndex*sizeof(pthread_t));
		struct threadArgs* args = malloc(indeIndex*sizeof(struct threadArgs));

		int a = 0;
		for(; a < indeIndex; a++)
		{
			args[a].id = a;
			args[a].c = depend_list[independentList[a]];
		}
		int z = 0;
		for(; z < indeIndex; z++)
		{
			//printf("executing command: %d\n", independentList[z]);
		}
		//printf("\n");
		int b = 0;
		for(; b < indeIndex; b++)
		{
			pthread_create(&threads[b], NULL, &paraFunc, &args[b]);
		}

		int c = 0;
		for(; c < indeIndex; c++)
		{
			pthread_join(threads[c], NULL);
		}


		// remove dependencies after executing commands
	//	printf("indeIndex: %d\n", indeIndex);
		int j = 0;
		for(; j < indeIndex; j++)
		{
			int col = independentList[j];

			int k = 0;
			for(; k < g_num_depend; k++)
			{
				if(depend_matrix[k][col] == 1)
				{
			//		printf("col: %d\n", col);
					depend_matrix[k][col] = 0;
					depend_matrix[k][g_num_depend]--;
				}
			}
		}
		//print_adj_matrix(depend_matrix, g_num_depend);


		free(threads);
		free(args);
	}

	
	free(DFS_stack);
	free(depend_list);
	free(independentList);

	int aa = 0;
	for(; aa < g_num_depend; aa++)
	{
		free(file_depend_list[aa]);
		free(depend_matrix[aa]);
	}
	free(depend_matrix[aa]);
	free(depend_matrix);
	free(file_depend_list);
	g_num_depend = 0;

}

void* paraFunc(void* args)
{
	struct threadArgs* thread_info = (struct threadArgs*) args;
	execute_command(thread_info->c, 0);
	return NULL;
}

/*void zeroOutList(struct file_redir** fileList)
{
	int index;
	for(index = 0; index < g_num_depend; index++)
	{
		file_depend_list[index][0].num = 0;
	}
}*/

//get_depend_simple(file

void getDependencies(struct file_redir** file_depend_list, command_t* depend_list, int index)
{
	command_t root = depend_list[index];

	int max_files = 10;
	int BFS_queue_size = 10;
	struct file_redir* file_list = malloc(sizeof(struct file_redir)*max_files);
	command_t* BFS_queue = malloc(sizeof(command_t)*BFS_queue_size);

	file_list[0].num = 0; 

	int queue_head = 0;
	int queue_tail = 1;
	while(root->type == SUBSHELL_COMMAND)
	{
		root = root->u.subshell_command;
	}

	if(root->type != SIMPLE_COMMAND)
	{
		BFS_queue[0] = root;
	}
	else
	{
		if(root->input != 0)
			{
				file_list[file_list[0].num+1].fileName = root->input;
				file_list[file_list[0].num+1].redir = INPUT;
				file_list[0].num++;
			}
		if(root->output != 0)
		{
			file_list[file_list[0].num+1].fileName = root->output;
			file_list[file_list[0].num+1].redir = OUTPUT;
			file_list[0].num++;
		}
		file_depend_list[index] = file_list;
		return;
	}

	while(queue_tail-queue_head > 0)
	{
	//	printf("more work: %d\n", index);
		if(queue_tail >= BFS_queue_size-2)
		{
			BFS_queue_size *= 2;
			BFS_queue = realloc(BFS_queue, sizeof(command_t)*BFS_queue_size);
		}
		if(file_list[0].num >= max_files-5)
		{
			max_files *= 2;
			file_list = realloc(file_list, sizeof(command_t)*max_files);
		}

		command_t child1 = BFS_queue[queue_head]->u.command[0];
		if(child1->type != SIMPLE_COMMAND)
		{
			BFS_queue[queue_tail] = child1;
			queue_tail++;
		}
		else
		{
			if(child1->input != 0)
			{
				//printf("in1\n");
				file_list[file_list[0].num+1].fileName = child1->input;
				file_list[file_list[0].num+1].redir = INPUT;
				file_list[0].num++;
			}
			if(child1->output != 0)
			{
				//printf("out1\n");
				file_list[file_list[0].num+1].fileName = child1->output;
				file_list[file_list[0].num+1].redir = OUTPUT;
				file_list[0].num++;
			}
		}

		command_t child2 = BFS_queue[queue_head]->u.command[1];
		if(child2->type != SIMPLE_COMMAND)
		{
			BFS_queue[queue_tail] = child2;
			queue_tail++;
		}
		else
		{
			/*depend_list[file_list[0].num+1] = child2;
			file_list[0].num++;*/
			if(child2->input != 0)
			{
				//printf("in2\n");
				file_list[file_list[0].num+1].fileName = child2->input;
				file_list[file_list[0].num+1].redir= INPUT;
				file_list[0].num++;
			}
			if(child2->output != 0)
			{
				//printf("out2\n");
				file_list[file_list[0].num+1].fileName = child2->output;
				file_list[file_list[0].num+1].redir = OUTPUT;
				file_list[0].num++;
			}
		}

		queue_head++;
	}

	file_depend_list[index] = file_list;
	//printf("num: %d\n", file_depend_list[index][0].num);
	//printf("gd index: %d\n", index);
	free(BFS_queue);
	return;

}

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
/*
void  printStr(char* c)
{
	int length = strlen(c);
	int i;
	for(i = 0; i < length; i++)
	{
		printf("%c", c[i]);
	}
	printf("\nlength: %d\n", length);
	printf("end printString\n");
}*/

int
command_status (command_t c)
{
  return c->status;
}
/*
pid_t forkWrap()
{
	pid_t pid = fork();
	
	if(pid < 0)
	{
		fprintf(stderr, "Fork failed\n");
		//exit(1);
	}

	return pid;
}

int execvpWrap(const char* file, char *const argv[])
{
	int suc = execvp(file, argv);
	if(suc < 0)
	{
		fprintf(stderr, "execution failed\n");
		//exit(1);
	}
	return suc;
}
*/


int isExec(char* c)
{
	if(c[0] == 'e' && c[1] == 'x' && c[2] == 'e' && c[3] == 'c')
	{
		return 1;
	}
	return 0;
}



void execSimpleCommand(command_t c)
{
	//printf("exec simple command\n");
	//print_words(c->u.word, c->numWords);
	pid_t pid = fork();
	if(pid < 0)
	{
		//c->status = 1;
		error(1, 0, "error when forking\n");
		return;
	}
	if(pid == 0) // child process
	{
		//printf("simple child\n");
		if(c->output != NULL)
		{
			//printf("output not null\n");
			char* output = c->output;
			int fout = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
			if(fout < 0)
			{
				error(1, 0, "error when opening file for output\n");
			}
			else
			{
				//printf("open file successful\n");
				if(dup2(fout, 1) < 0)
				{
					error(1, 0, "error in dup2\n");
				}
				
			}
			close(fout);
		}
		
		if(c->input != NULL)
		{
			char* input = c->input;
			int fin = open(input, O_RDONLY, 0666);
			if(fin < 0)
			{
				error(1, 0, "error when opening file for input\n");
			}
			else
			{
				if(dup2(fin, 0) < 0)
				{
					error(1, 0, "error in dup2\n");
				}
				close(fin);
			}
			
		}
		//printf("WTF");
		//printf("a\n");
		char*arg1;
		char** arg2;
		
		  if(!isExec(c->u.word[0]))
        {
            arg1 = c->u.word[0];
            arg2 = c->u.word;   
        }
        else
        {
            arg1 = c->u.word[1];
            arg2 = c->u.word + 1;
        }
		
		/*	arg1 = c->u.word[0];
			arg2 = c->u.word;	*/
			//printStr(arg1);
			//printf("numWords: %d\n", c->numWords);
			//printf("before exec\n");
		if(execvp(arg1, arg2) < 0)
		{
		//	c->status = 
			error(1, 0, "Error in execution of simple command\n");
			//return;
		}
		//printf("after exec\n");
		//exit(0);

		 /* if(execvp(c->u.word[0],c->u.word)<0)
			  printf("error");
                exit(c->status);*/
	}
	else // parent process
	{
		
		int status;
		waitpid(pid, &status, 0);
		//printf("simple parent\n");

		c->status = WEXITSTATUS(status);
		//printf("status: %d\n", c->status);
	}
	//printf("end exec simple command\n");
}

/*if(!isExec(c->u.word[0]))
		{
			arg1 = c->u.word[0];
			arg2 = c->u.word;	
		}
		else
		{
			arg1 = c->u.word[1];
			arg2 = c->u.word + 1;
		}*/
/*
void executeSubshellCommand(command_t c)
{

}*/


void execPipe(command_t c)
{
	int pipefd[2];
	command_t left = c->u.command[0];
	command_t right = c->u.command[1];

	if(pipe(pipefd) < 0)
	{
		error(1, 0, "error when trying to pipe\n");
		return;
	}

	pid_t pid1 = fork();
	
	if(pid1 < 0) // fork error
	{
		error(1, 0, "error when forking\n");
	}

	else if(pid1 == 0)
	{
		//printf("child1\n");
		
		close(pipefd[0]);
		if(dup2(pipefd[1], 1) < 0)
		{
			error(1, 0, "error in pipe  dup2\n");
			exit(1);
				// error
		}

		execute_command(left, 0);
		if(left->status != 0)
		{
			error(1, 0, "error in execution of left pipe");
		}
		exit(0);
		
	}
	

	pid_t pid2 = fork();

	if(pid2 < 0) // fork error
	{
		error(1, 0, "error when forking\n");
		// error
	}
	else if(pid2 == 0) // child process 
		// execute right side of pipe
	{
		// wait for child to finish
			
		//check exit status of left pipe command
		close(pipefd[1]);
		if(dup2(pipefd[0], 0) < 0)
		{
			error(1, 0, "error in pipe dup2\n");
			exit(1);
			// error
		}
		//close(pipefd[0]);


	//	printf("child2\n");
		
		execute_command(right, 0);
		if(right->status != 0)
		{
			error(1, 0, "error in execution of right pipe");

		}
		exit(0);
	}
	
	int status1, status2;
		
	close(pipefd[0]);
	close(pipefd[1]);
	waitpid(pid1, &status1, 0);
	waitpid(pid2, &status2, 0);
	//printf("status1: %d\n", WEXITSTATUS(status1));
	//printf("status2: %d\n", WEXITSTATUS(status2));
	c->status = WEXITSTATUS(status2);
	if(WEXITSTATUS(status2) == 0 &&  WEXITSTATUS(status1) != 0)
	{
		c->status = WEXITSTATUS(status1);
	}
	//printf("c->status: %d\n", c->status);

	

}
/*else
	{
		int status;
		close(pipefd[0]);
		close(pipefd[1]);
		waitpid(pid2, &status, 0); 
		printf("status: %d" , status);
		c->status = status;
		return;
	}*/



void printCommType(enum command_type command)
{
	switch(command)
		{
			case AND_COMMAND: printf("AND_COMMAND\n"); break;
			case SEQUENCE_COMMAND: printf("SEQUENCE_COMMAND\n"); break;
			case OR_COMMAND: printf("OR_COMMAND\n"); break;
			case PIPE_COMMAND: printf("PIPE_COMMAND\n"); break;
			case SIMPLE_COMMAND: printf("SIMPLE_COMMAND\n"); //printf("commList[i].numWords: %d\n", commList[i].numWords);
									 //printWords(commList[i].u.word, commList[i].numWords); break;
				break;
			case OPEN_PAREN: printf("OPEN_PAREN\n"); break;
			case CLOSED_PAREN: printf("CLOSED_PAREN\n"); break;
			case LEFT_REDIRECT: printf("LEFT_REDIRECT\n"); break;
			case RIGHT_REDIRECT: printf("RIGHT_REDIRECT\n"); break;
			case NEW_LINE: printf("NEW_LINE\n"); break;
			default: break;
		}
}

void execOperatorCommand(command_t c)
{
	command_t left = c->u.command[0];
	command_t right = c->u.command[1];
	switch(c->type)
	{
	case AND_COMMAND:
		
		execute_command(left, 0);
		c->status = left->status;
		if(left->status == 0) // if left command successful, execute right command and save status of right command
		{
			execute_command(right, 0);
			c->status = right->status;
		}
		else // dont execute right command
		{
			c->status = left->status;
		}

		break;
	case OR_COMMAND:
		execute_command(left, 0);
		c->status = left->status;
		if(left->status != 0) // if left command is not successful, execute right command and save status of right command
		{
			execute_command(right, 0);
			c->status = right->status;
		}
		else
		{
			c->status = left->status;
		}
		break;
	case PIPE_COMMAND:
		execPipe(c);
		break;
	case SEQUENCE_COMMAND:
		execute_command(left, 0);
		execute_command(right, 0);
		break;
	default:
		fprintf(stderr, "ERROR: INVALID COMMAND");
		//printCommType(c->type);
	}
}

	void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
	//printf("time_travel: %d\n", time_travel);

	if(time_travel == 1)
	{
		handle_dependencies(c);
	}
	else
	{
		//printf("else ");
		switch(c->type)
		{
		case SIMPLE_COMMAND:
			execSimpleCommand(c);
			break;
		case SUBSHELL_COMMAND:
			execute_command(c->u.subshell_command, 0);
			c->status = c->u.subshell_command->status;
			break;
		default:
			execOperatorCommand(c);
		}
	}
/*
	switch(c->type)
		{
		case SIMPLE_COMMAND:
			execSimpleCommand(c);
			break;
		case SUBSHELL_COMMAND:
			execute_command(c->u.subshell_command, 0);
			c->status = c->u.subshell_command->status;
			break;
		default:
			execOperatorCommand(c);
		}
	*/


 // error (1, 0, "command execution not yet implemented");
	
}