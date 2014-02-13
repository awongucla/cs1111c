// UCLA CS 111 Lab 1 command reading
// REGULAR

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>


/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

struct command_stream
{
	command_t* treeHead;
	int size;
};

int g_commandListLength = 0;

int isToken(char c)
{
	if(c == ';' || c == '|' || c == '&' || c == '(' || c == ')' || c == '<' || c == '>' || (int)c == 10)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int isValidCharForWord(char c)
{
	if(c == '!' || c == '%' || ((int) c >= 43 && (int) c <= 58) 
		|| c == '@' || ((int) c >=65 && (int) c <= 90) || c == '^' 
		|| c == '_' || ((int) c >= 97 && (int) c <= 122))
	{
		return 1;
	}
	else
	{
		return 0;
	}

}


void  printString(char* c)
{
	int length = strlen(c);
	int i;
	for(i = 0; i < length; i++)
	{
		printf("%c", c[i]);
	}
	printf("\nlength: %d\n", length);
	printf("end printString\n");
}

char* getTextFromFile(int (*get_next_byte) (void *), void *get_next_byte_argument)
{ // processes file and adds chars to a text file, compresses excess whitespace (spaces or tabs)
	// checks for characters that are not for words or special tokens
	int maxLetters = 2;

	char* fileString = malloc(sizeof(char)*maxLetters);
	char prevChar;
	int index = 0;
	int prevIsWhiteSpace = 0;
	int prevIsNewLine = 0;
	int lineNumber = 1;
	while(1)
	{
	//	printf("index: %d\n", index);
		
		int c = get_next_byte(get_next_byte_argument);
		//printf("char: %c\n", c);
		//printf("lineNumber: %d\n", lineNumber);
	//	printf("prevIsWhiteSpace: %d\n", prevIsWhiteSpace);
		if(c == EOF) // end of file?
		{
			break;
		}
		else if(index == maxLetters) // need to reallocate
		{
			maxLetters *= 100 ;
			fileString = realloc(fileString, maxLetters*sizeof(char));
		}

		if(c == '\n')
		{
			lineNumber++;
		}

		if(c == '#')
		{
			if(index != 0)
			{
				if( prevChar != ' ' && prevChar != '\n')
				{
					fprintf(stderr, "%d: COMMENT SYNTAX ERROR", lineNumber);
					exit(1);
				}
			}
			c = get_next_byte(get_next_byte_argument);
			while(c != EOF)
			{
				if(c == '\n')
				{
					lineNumber++;
					break;
				}
				c = get_next_byte(get_next_byte_argument);
			}
			prevChar = c;
			continue;
		}

		if(c == '\t') // is a tab
		{
			if(prevIsWhiteSpace)
			{
				//index++;
				continue;
			}
			else
			{
				prevIsWhiteSpace = 1;
				fileString[index] = (char) 32;
				prevChar = ' ';
				index++;
				continue;
			}
		}
		else if(c == ' ') // is a space
		{
			if(prevIsWhiteSpace)
			{
				//index++;
				prevChar = ' ';
				continue;
			}
			else
			{
				prevIsWhiteSpace = 1;
				fileString[index] = ' ';
				prevChar = ' ';
				index++;
				continue;
			}
		}
		else if(c == '\n')
		{
			if(prevIsNewLine)
			{
				//index++;
				prevChar = '\n';
				continue;
			}
			else
			{
				prevIsNewLine = 1;
				fileString[index] = '\n';
				prevChar = '\n';
				index++;
				continue;
			}
		}
		
		else
		{
			prevIsNewLine = 0;
			prevIsWhiteSpace = 0;
			//printf("index in else: %d\n", index);

			if(!isToken(c) && !isValidCharForWord(c))
			{
				//printf("syntax error invalid char: %d \n", c);
				fprintf(stderr, "%d: syntax error invalid char", lineNumber);
				exit(1);
				break;
			}

			fileString[index] = (char) c;
			prevChar = c;
			index++;
			continue;
		}
	}
	fileString[index] = '\0';
	return fileString;

}
/*
void checkParen(char* fileString) 
	// checks same # of open and close parentheses
{
	int firstOpenParen = -1;
	int lastOpenParen = -1;
	int firstCloseParen = -1;

	int numOpenParen = 0;
	int numCloseParen = 0;
	int parenScore = 0; // score should never be negative, score should be 0 when loops terminates

	int index;
	int length = strlen(fileString);
	for(index = 0; index < length; index++)
	{
		char c = fileString[index];
		if(c == '(')
		{
			
			lastOpenParen = index;
			parenScore++;
			numOpenParen++;
		}
		if(c == ')')
		{
			if(firstCloseParen == -1)
			{
				firstCloseParen = index;
			}
			if(parenScore == 0)
			{
				printf("syntax error1: too many closed parentheses\n");
			}
			parenScore--;
			numCloseParen++;
		}
	}

	
	if(parenScore > 0)
	{
		printf("syntax error3: too many open parentheses\n");
		// syntax error at last parentheses
	}
	printf("end of check paren\n");
}*/

void checkParen(command_t commList) 
	// checks same # of open and close parentheses
{
	int firstOpenParen = -1;
	int lastOpenParen = -1;
	int firstCloseParen = -1;

	int numOpenParen = 0;
	int numCloseParen = 0;
	int parenScore = 0; // score should never be negative, score should be 0 when loops terminates or when reach SEQUENCE_COMMAND

	int index;
	//int length = strlen(fileString);
	int lineNumber = 1;
	for(index = 0; index < g_commandListLength; index++)
	{
		enum command_type commType = commList[index].type;
		if(commType == IGNORE_COMMAND || commType == NEW_LINE)
		{
			lineNumber++;
		}
		if(commType == OPEN_PAREN)
		{
			lastOpenParen = index;
			parenScore++;
			numOpenParen++;
		}
		if(commType == CLOSED_PAREN)
		{
			if(firstCloseParen == -1)
			{
				firstCloseParen = index;
			}
			if(parenScore == 0)
			{
				//printf("syntax error1: too many closed parentheses\n");
				fprintf(stderr, "%d: too many closed parentheses", lineNumber);
				exit(1);
			}
			parenScore--;
			numCloseParen++;
		}
		if(commType == NEW_LINE && parenScore != 0)
		{
			//printf("syntax error paren: too many open paren in complete command");
			fprintf(stderr, "%d: syntax error paren: too many open paren in complete command", lineNumber);
			exit(1);
		}
	}

	
	if(parenScore > 0)
	{
		//printf("syntax error3: too many open parentheses\n");
		fprintf(stderr, "%d: syntax error3: too many open parentheses", lineNumber);
		exit(1);
		// syntax error at last parentheses
	}
	//printf("end of check paren\n");

}
/*void isValidWord(char* fileString, int start, int end) 
	// start and end arguments are inclusive
{

}
*/

void printWord(char* word)
{
	int i = 0;
	while(word[i] != '\0')
	{
		printf("%c", word[i]);
		i++;
	}
	printf(" ");
}

int getStrLen(char* word)
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


void printWords(char** words, int numWords)
{
	//printf("numWords: %d\n", numWords);
	int i;
	for(i = 0; i < numWords; i++)
	{
		char* word = words[i];
		int length = getStrLen(word);
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

void printCommandList(command_t commList, int length)
{

	int i;
	printf("printCommandList\n");
	printf("length: %d\n", length);
	int k = commList[0].type == SIMPLE_COMMAND;
	printf("k: %d\n", k);
	for(i = 0; i < length; i++)
	{
		printf("%d ", i);
		switch(commList[i].type)
		{
			case AND_COMMAND: printf("AND_COMMAND\n"); break;
			case SEQUENCE_COMMAND: printf("SEQUENCE_COMMAND\n"); break;
			case OR_COMMAND: printf("OR_COMMAND\n"); break;
			case PIPE_COMMAND: printf("PIPE_COMMAND\n"); break;
			case SIMPLE_COMMAND: printf("SIMPLE_COMMAND\n"); //printf("commList[i].numWords: %d\n", commList[i].numWords);
									 printWords(commList[i].u.word, commList[i].numWords); break;
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


command_t zeroIO(command_t commandList, int listSize)
{
	int i;
	for(i = 0; i < listSize; i++)
	{
		commandList[i].input = 0;
		commandList[i].output = 0;
	}
	return commandList;
}

command_t makeCommandList(char* fileString)
{
	int maxListSize = 1000;
	int listSize = 0;
	//printf("before malloc");
	command_t commandList = malloc(sizeof(struct command)*maxListSize);
	
	//checkParen(fileString);
//	printf("before strlen");
	int length = strlen(fileString);
	//printf("length: %d\n", length);
	int index;

	/*command_t comm = malloc(sizeof(command_t));
	comm->type = AND_COMMAND;*/
	int lineNumber = 1;
	//printf("begin forloop\n");
	for(index = 0; index < length; index++)
	{
		//printf("index: %d\n", index);
		char c = fileString[index];
		
		//printf("makeCommandList: c: %c %d\n", c, c);
		//printf("listSize: %d\n", listSize);
		if(listSize == maxListSize)
		{
			//printf("realloc commandList");
			maxListSize *= 2;
			commandList = realloc(commandList, sizeof(struct command)*maxListSize);
		}

		if(c == '&')
		{
			if(fileString[index+1] == '&')
			{
				commandList[listSize].type  = AND_COMMAND;
				//commandList[listSize].status = 1;
				listSize++;
				index++;
				continue;
			}
			else
			{
				fprintf(stderr, "%d: invalid syntax: only one & sign", lineNumber);
				exit(1);
			}
		}

		else if(c == '|')
		{
			//printf("OR");
			if(fileString[index+1] == '|')
			{
				// add PIPE_COMMAND to list
				commandList[listSize].type  = OR_COMMAND;
				listSize++;
				index++;
				continue;
			}
			else
			{
				// add OR_COMMAND to list
				commandList[listSize].type  = PIPE_COMMAND;
				listSize++;
				continue;
			}
		}

		else if(c == ';')
		{
			// add SEQUENCE_COMMAND to list
			commandList[listSize].type  = SEQUENCE_COMMAND;
			listSize++;
			continue;
		}

		else if(c == '\n')
		{
			commandList[listSize].type  = NEW_LINE;
			lineNumber++;
			listSize++;
			continue;
		}

		else if(c == ' ')
		{
			continue;
		}

		else if(c == '>')
		{
			commandList[listSize].type = RIGHT_REDIRECT;
			listSize++;
			continue;
		}

		else if(c == '<')
		{
			commandList[listSize].type = LEFT_REDIRECT;
			listSize++;
			continue;
		}

		
		else if(c == ')')
		{
			commandList[listSize].type = CLOSED_PAREN;
			listSize++;
			continue;
		}

		else if( c == '(')
		{
			commandList[listSize].type = OPEN_PAREN;
			listSize++;
			continue;
		}

		else
		{
			commandList[listSize].type = SIMPLE_COMMAND;
			
			//index++;
			c = fileString[index];

			int maxWords = 10;
			int maxChars = 10;
			int wordsIndex = 0;
			char** words = malloc(maxWords*sizeof(char*));
			

			while(1)
			{
				int breakAll = 0;
				if(wordsIndex == maxWords)
				{
					maxWords *= 2;
					words = realloc(words, maxWords*sizeof(char*));
				}
				int letterIndex = 0;
				char* letters = malloc(maxChars*sizeof(char));
				c = fileString[index];
				
				if(isToken(c))
				{
				//		words[wordsIndex] = letters;
					/*printf("index: %d\n", index);
					printf("token c: %c %d\n", c, c);
					printf("isToken\n");*/
						wordsIndex++;
						index--;
						break;
				}

				while(c != ' ' && c != '\0' && c != EOF)
				{
					//printf("word c%d: %c %d\n",index, c, c);
					//printf("index: %d\n", index);
					if(isToken(c))
					{
						breakAll = 1;
						//printf("letterIndex: %d\n", letterIndex);
						if(letterIndex > 0)
						{
							letters[letterIndex] = '\0';
							words[wordsIndex] = letters;
							wordsIndex++;
						}
						index--;
						commandList[listSize].numWords =  wordsIndex;
						break;
					}
					if(letterIndex == maxChars)
					{
						maxChars *= 2;
						letters = realloc(letters, maxChars*sizeof(char));
					}
					letters[letterIndex] = c;
					letterIndex++;
					index++;
					if(index < length)
					{
						c = fileString[index];
					}
					else
					{
						letters[letterIndex] = '\0';
						words[wordsIndex] = letters;
						breakAll = 1;
						break;
					}
					
				}
				if(breakAll)
				{
					break;
				}
				//printf("PRINT_WORD");
			//	printWord(letters);
				letters[letterIndex] = '\0';
				words[wordsIndex] = letters;
				wordsIndex++;
				commandList[listSize].numWords =  wordsIndex;
				
				index++;
				//printWords(commandList[0].u.word, commandList[0].numWords);
			}
			commandList[listSize].u.word = words;
			
			//printWords(commandList[listSize].u.word, commandList[listSize].numWords);
			//printWords(commandList[0].u.word, commandList[0].numWords);
			listSize++;
		}
	}
	//printf("listSize: %d\n", listSize);
	g_commandListLength = listSize;
	 //printWords(commandList[0].u.word, commandList[0].numWords);
	//printCommandList(commandList, listSize);
	return zeroIO(commandList, g_commandListLength);
}



void checkCommandList(command_t commList)
{
	int i;
	int length = g_commandListLength;
	int lineNumber = 1;
	//int iter;
		for(i = 0; i < length; i++)
		{
			enum command_type commType = commList[i].type;
		
			if(i == 0) // beginning of command list
				// only possible command types new_line, simple_command, open_paren
			{
				if(commType == NEW_LINE) 
				{
					if(commList[i+1].type != OPEN_PAREN && commList[i+1].type != CLOSED_PAREN && commList[i+1].type != SIMPLE_COMMAND && commList[i+1].type != NEW_LINE)
					{ // invalid command following new line
						//printf("syn err 1 line: %d\n", lineNumber);
						//printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err 1", lineNumber);
						exit(1);

					}
					else
					{ 
						commList[i].type = IGNORE_COMMAND;
						lineNumber++;
						continue;
					}
				}
				else if(commType != SIMPLE_COMMAND && commType != OPEN_PAREN)
				{ 
					//printf("syn err 2 line: %d\n", lineNumber);
					//printf("err index: %d\n", i);
					fprintf(stderr, "%d: syn err 2", lineNumber);
					exit(1);
					// syntax error at line 1
				}
				else
				{
					continue;
				}
			}
			if(i == length -1) // end of command list
			{
				if(commType != NEW_LINE && commType != CLOSED_PAREN && commType != SEQUENCE_COMMAND && commType != SIMPLE_COMMAND && commType != IGNORE_COMMAND)
				{
					//printf("syn err 3 line: %d\n", lineNumber);
					//printf("err index: %d\n", i);
					fprintf(stderr, "%d: syn err 3", lineNumber);
					exit(1);
					//syntax error
				}
				if(commType == NEW_LINE && (commList[i-1].type == NEW_LINE || commList[i-1].type ==  IGNORE_COMMAND))
				{
					commList[i].type = IGNORE_COMMAND;
					continue;
				}
				
				else
				{
					continue;
				}
			}
			if(commType == PIPE_COMMAND)
			{
				int lowerInd = i-1;
				while(commList[lowerInd].type != SIMPLE_COMMAND)
				{
					if(commList[lowerInd].type != CLOSED_PAREN && commList[lowerInd].type != NEW_LINE)
					{
						//printf("syn err 4 line: %d\n", lineNumber);
						//printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err 4", lineNumber);
						exit(1);
						break;
						 // syntax error
					}
					lowerInd--;
					if(lowerInd < 0)
					{
						//printf("syn err PIPE1: %d\n", lineNumber);
						//printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err PIPE1", lineNumber);
						exit(1);
						break;
					}

				}
				int higherInd = i+1;
				while(commList[higherInd].type != SIMPLE_COMMAND)
				{

					if(commList[higherInd].type != OPEN_PAREN && commList[higherInd].type != NEW_LINE)
					{
						//printf("syn err 5 line: %d\n", lineNumber);
						//printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err 5", lineNumber);
						exit(1);
						break;
						 // syntax error
					}
					higherInd++;
					if(higherInd >= length)
					{
						//printf("syn err PIPE2: %d\n", lineNumber);
						//printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err PIPE2", lineNumber);
						exit(1);
						break;
					}

				}
				continue;
				/*else if(commList[i+1].type != SIMPLE_COMMAND && commList[i+1].type != NEW_LINE && commList[i+1].type != OPEN_PAREN)
				{
					printf("syn err 5 line: %d\n", lineNumber);
					printf("err index: %d\n", i);
					// syntax error
				}*/
			}
		
			else if(commType == AND_COMMAND)
			{
				int lowerInd = i-1;
				while(commList[lowerInd].type != SIMPLE_COMMAND)
				{
					if(commList[lowerInd].type != CLOSED_PAREN && commList[lowerInd].type != NEW_LINE)
					{
					//	printf("syn err 6 line: %d\n", lineNumber);
					//	printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err 6", lineNumber);
						exit(1);
						break;
						 // syntax error
					}
					lowerInd--;
					if(lowerInd < 0)
					{
					//	printf("syn err AND1: %d\n", lineNumber);
					//	printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err AND1", lineNumber);
						exit(1);
						break;
					}

				}
				

				int higherInd = i+1;
				while(commList[higherInd].type != SIMPLE_COMMAND)
				{
					if(commList[higherInd].type != OPEN_PAREN && commList[higherInd].type != NEW_LINE)
					{
					//	printf("syn err 7 line: %d\n", lineNumber);
					//	printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err 7", lineNumber);
						exit(1);
						break;
						 // syntax error
					}
					higherInd++;
					if(higherInd >= length)
					{
					//	printf("syn err AND2: %d\n", lineNumber);
					//	printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err AND2", lineNumber);
						exit(1);
						break;
					}

				}


/*
				if(commList[i-1].type != SIMPLE_COMMAND)
				{
					printf("syn err 6 line: %d\n", lineNumber);
					printf("err index: %d\n", i);
					 // syntax error
				}*/
				/*else if(commList[i+1].type != SIMPLE_COMMAND  && commList[i+1].type != NEW_LINE && commList[i+1].type != OPEN_PAREN)
				{
					printf("syn err 7 line: %d\n", lineNumber);
					printf("err index: %d\n", i);
					// syntax error
				}*/
				/*else
				{
					continue;
				}*/
				continue;
			}

			else if(commType == OR_COMMAND)
			{
				int lowerInd = i-1;
				while(commList[lowerInd].type != SIMPLE_COMMAND)
				{
					if(commList[lowerInd].type != CLOSED_PAREN && commList[lowerInd].type != NEW_LINE)
					{
					//	printf("syn err 8 line: %d\n", lineNumber);
					//	printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err 8", lineNumber);
						exit(1);
						break;
						 // syntax error
					}
					lowerInd--;
					if(lowerInd < 0)
					{
					//	printf("syn err OR1: %d\n", lineNumber);
					//	printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err OR1", lineNumber);
						exit(1);
						break;
					}

				}
				

				int higherInd = i+1;
				while(commList[higherInd].type != SIMPLE_COMMAND)
				{
					if(commList[higherInd].type != OPEN_PAREN && commList[higherInd].type != NEW_LINE)
					{
					//	printf("syn err 9 line: %d\n", lineNumber);
					//	printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err 9", lineNumber);
						exit(1);
						break;
						 // syntax error
					}
					higherInd++;
					if(higherInd >= length)
					{
					//	printf("syn err OR2: %d\n", lineNumber);
					//	printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err OR2", lineNumber);
						exit(1);
						break;
					}
				}
				/*if(commList[i-1].type != SIMPLE_COMMAND)
				{
					printf("syn err 8 line: %d\n", lineNumber);
					printf("err index: %d\n", i);
					 // syntax error
				}

				else if(commList[i+1].type != SIMPLE_COMMAND  && commList[i+1].type != NEW_LINE && commList[i+1].type != OPEN_PAREN)
				{
					printf("syn err 9 line: %d\n", lineNumber);
					printf("err index: %d\n", i);
					// syntax error
				}
				else
				{
					continue;
				}*/
			}
			else if(commType == NEW_LINE)
			{
				//printf("NEW_LINE comm#: %d\n", i);
				if(commList[i+1].type != OPEN_PAREN && commList[i+1].type != CLOSED_PAREN && commList[i+1].type != SIMPLE_COMMAND && commList[i+1].type != NEW_LINE)
				{
				//	printf("syn err 10 line: %d\n", lineNumber);
					lineNumber++;
				//printf("lineNumber: %d"
				//	printf("err index: %d\n", i);
					fprintf(stderr, "%d: syn err 10", lineNumber);
					exit(1);
					// syntax error 
				}
				else if(commList[i-1].type == LEFT_REDIRECT || commList[i-1].type == RIGHT_REDIRECT)
				{
				//	printf("syn err 11 line: %d\n", lineNumber);
					lineNumber++;
				//	printf("err index: %d\n", i);
					fprintf(stderr, "%d: syn err 11", lineNumber);
					exit(1);
					// syntax error
				}
			    else if((commList[i-1].type == SIMPLE_COMMAND || commList[i-1].type == CLOSED_PAREN || commList[i-1].type == SEQUENCE_COMMAND) && commList[i+1].type != NEW_LINE)
				{
					//commList[i].type = SEQUENCE_COMMAND;
					lineNumber++;
					continue;
				}
				//else if(commist[i-1].type == 
				else
				{
					commList[i].type = IGNORE_COMMAND;
					lineNumber++;
					continue;
				}
				//else if(commList[i-1].type == 
			}

			else if(commType == SEQUENCE_COMMAND)
			{
				if(commList[i-1].type != SIMPLE_COMMAND && commList[i-1].type != CLOSED_PAREN)
				{
					fprintf(stderr, "%d: SEQUENCE COMMAND SYNTAX ERROR", lineNumber);
					exit(1);
				}
				if(commList[i+1].type == NEW_LINE)
				{
					commList[i].type = NEW_LINE;
					commList[i+1].type = IGNORE_COMMAND;
				}
				continue;
			}

			else if(commType == LEFT_REDIRECT)
			{
				int lowerInd = i-1;
				while(commList[lowerInd].type != SIMPLE_COMMAND)
				{
					if(commList[lowerInd].type != CLOSED_PAREN && commList[lowerInd].type != NEW_LINE)
					{
					//	printf("syn err 8 line: %d\n", lineNumber);
					//	printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err 12", lineNumber);
						exit(1);
						break;
						 // syntax error
					}
					lowerInd--;
					if(lowerInd < 0)
					{
					//	printf("syn err OR1: %d\n", lineNumber);
					//	printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err L_REDIRECT", lineNumber);
						exit(1);
						break;
					}
				}

				/*if(commList[i-1].type != SIMPLE_COMMAND)
				{
				//	printf("syn err 12 line: %d\n", lineNumber);
				//	printf("err index: %d\n", i);
					fprintf(stderr, "%d: syn err 12", lineNumber);
					exit(1);
					 // syntax error
				}*/
				if(commList[i+1].type != SIMPLE_COMMAND || commList[i+1].numWords > 1)
				{
				//	printf("syn err 13 line: %d\n", lineNumber);
				//	printf("err index: %d\n", i);
					fprintf(stderr, "%d: syn err 13", lineNumber);
					exit(1);
					// syntax error
				}
				else
				{
					continue;
				}
			}

			else if(commType == RIGHT_REDIRECT)
			{
				int lowerInd = i-1;
				while(commList[lowerInd].type != SIMPLE_COMMAND)
				{
					if(commList[lowerInd].type != CLOSED_PAREN && commList[lowerInd].type != NEW_LINE)
					{
					//	printf("syn err 8 line: %d\n", lineNumber);
					//	printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err 12", lineNumber);
						exit(1);
						break;
						 // syntax error
					}
					lowerInd--;
					if(lowerInd < 0)
					{
					//	printf("syn err OR1: %d\n", lineNumber);
					//	printf("err index: %d\n", i);
						fprintf(stderr, "%d: syn err L_REDIRECT", lineNumber);
						exit(1);
						break;
					}
				}
				/*if(commList[i-1].type != SIMPLE_COMMAND || commList[i+1].numWords > 1)
				{
				//	printf("syn err 14 line: %d\n", lineNumber);
				//	printf("err index: %d\n", i);
					fprintf(stderr, "%d: syn err 14", lineNumber);
					exit(1);
					 // syntax error
				}*/
				if(commList[i+1].type != SIMPLE_COMMAND)
				{
				//	printf("syn err 15 line: %d\n", lineNumber);
				//	printf("err index: %d\n", i);
					fprintf(stderr, "%d: syn err 15", lineNumber);
					exit(1);
					// syntax error
				}
				else
				{
					continue;
				}
			}
		}
	
//	printf("end check");
}

int checkPrecedence(enum command_type input, enum command_type top) // compares current command "input" with command "top" at top of stack
	// returns 0 if you add input to stack, return 1 if you pop off operator stack
{
	if(input == SEQUENCE_COMMAND)
	{
		if(top != OPEN_PAREN)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	if(input == AND_COMMAND || input == OR_COMMAND)
	{
		if(top != OPEN_PAREN && top != SEQUENCE_COMMAND)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	if(input == PIPE_COMMAND)
	{
		if(top == LEFT_REDIRECT || top == RIGHT_REDIRECT || top == PIPE_COMMAND)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	if(input == LEFT_REDIRECT || input == RIGHT_REDIRECT)
	{
		if(top == LEFT_REDIRECT || top == RIGHT_REDIRECT)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;

}

void popOperator(int operatorIndex, int operandIndex, int* operatorList, int* operandList, command_t commList) // indexes are the last OCCUPIED spot
{
	//printf("POP\n");
	//printf("pp..operatorIndex: %d\n", operatorIndex);
	//printf("pp..operandIndex: %d\n", operandIndex);
	int topOperatorIndex = operatorList[operatorIndex];
	int topOperandIndex1 = operandList[operandIndex];
	int topOperandIndex2 = operandList[operandIndex-1];

	enum command_type commandType = commList[topOperatorIndex].type;
	if(commandType == LEFT_REDIRECT)
	{
		commList[topOperandIndex2].input = commList[topOperandIndex1].u.word[0];
		//print_command(&commList[topOperandIndex2]);
		//printf("\n");
	}
	else if(commandType == RIGHT_REDIRECT)
	{
		commList[topOperandIndex2].output = commList[topOperandIndex1].u.word[0];
		//print_command(&commList[topOperandIndex2]);
		//printf("\n");
	}
	else
	{
		commList[topOperatorIndex].u.command[0] = &commList[topOperandIndex2];
		commList[topOperatorIndex].u.command[1] = &commList[topOperandIndex1];

		operandList[operandIndex] = -1;
		operandList[operandIndex-1] = topOperatorIndex;

		//print_command(&commList[operandList[operandIndex-1]]);
		//printf("\n");
	}
}

void printCommandType(enum command_type command)
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


void printStack(int operatorIndex, int operandIndex, int* operatorList, int* operandList) //
{
	// prints two columns, operators on the left and operands on the right
	/*printf("p..operatorIndex: %d\n", operatorIndex);
	printf("p..operandIndex: %d\n", operandIndex);
	printf("\nBOTTOM OF STACK\n");*/
		int index = 0;
	while(1)
	{
		int endOfStack1 = 0;
		int endOfStack2 = 0;
		if(index > operatorIndex)
		{
			endOfStack1 = 1;
			printf("            ");
		}
		else
		{
			printf("%d          ", operatorList[index]);
		}
		if(index > operandIndex)
		{
			printf("\n");
			endOfStack2 = 1;
		}
		else
		{
			printf("%d\n", operandList[index]);
		}
		if(endOfStack1 && endOfStack2)
		{
			break;
		}
		index++;
	}
	printf("\nTOP OF STACK\n");
}

void setSubShell(int subShellIndex, int operandIndex, int* operandList, command_t commList)
	// subShellIndex index of OPEN_PAREN command that will be changed to SUBSHELL head
	// operandIndex last occupied index of operandList
{

	/*printf("ss..subShellIndex: %d\n", subShellIndex);
	printf("ss..operandIndex: %d\n", operandIndex);*/
	commList[subShellIndex].type = SUBSHELL_COMMAND;
	int rootIndex = operandList[operandIndex];
	commList[subShellIndex].u.subshell_command = &commList[rootIndex];

	operandList[operandIndex] = subShellIndex;
}

int buildTree(command_t commList, int i, int rootListIndex, int* rootList)
{
	int maxOperandSize = 100;
	int maxOperatorSize = 100;
	int operandIndex = 0; // first empty index in operand stack
	int operatorIndex = 0; // first empty index in operator stack
	//printf("rootListIndex: %d\n", rootListIndex);

	int* operatorList = malloc(sizeof(int)*maxOperatorSize);
	int* operandList = malloc(sizeof(int)*maxOperandSize);
	int returnIndex = -1;
	//int i;
	//printf("# of commands: %d\n", g_commandListLength);
	for(; i < g_commandListLength; i++)
	{
	//	printf("ii: %d\n", i);
		//printf("p..operatorIndex: %d\n", operatorIndex);
		//printf("p..operandIndex: %d\n", operandIndex);
		if(operandIndex == maxOperandSize)
		{
			maxOperandSize *= 2;
			operandList = realloc(operandList, sizeof(int)*maxOperandSize);
		}
		else if(operatorIndex == maxOperatorSize)
		{
			maxOperatorSize *= 2;
			operatorList = realloc(operatorList, sizeof(int)*maxOperatorSize);
		}

		else if(commList[i].type == SIMPLE_COMMAND)
		{
			//printf("simple command\n");
			//printf("operandIndex: %d\n", operandIndex);
			operandList[operandIndex] = i;
			operandIndex++;
		//	printStack(operatorIndex-1, operandIndex-1, operatorList, operandList);
			
			continue;
		}

		else if(commList[i].type == OPEN_PAREN)
		{
			// push onto operator stack
			operatorList[operatorIndex] = i;
			operatorIndex++;
			continue;
		}
		else if(commList[i].type == CLOSED_PAREN)
		{
			// pop off until reach open paren
			int topIndex = operatorList[operatorIndex-1];
			enum command_type top = commList[topIndex].type-1;
			while(top != OPEN_PAREN)
			{
				//printf("pop til open paren \n");
				popOperator(operatorIndex-1, operandIndex-1, operatorList, operandList, commList);
				operandIndex--;
				operatorIndex--;
				//printStack(operatorIndex-1, operandIndex-1, operatorList, operandList);
				// print stack for debugging
				//printf("topIndex: %d\n", topIndex);
				topIndex = operatorList[operatorIndex-1];
				top = commList[topIndex].type;
			}
			//end loop: top = OPEN_PAREN
			//printf("end paren popping\n");
			int subShellIndex = operatorList[operatorIndex-1];
			setSubShell(subShellIndex, operandIndex-1, operandList, commList);
			operatorIndex--;
			//printCommandList(commList, g_commandListLength);
			//printStack(operatorIndex-1, operandIndex-1, operatorList, operandList);

			continue;
		}
		else if(commList[i].type == NEW_LINE)
		{
			returnIndex = i;
			if(i+1 == g_commandListLength)
			{
			//	printf("return\n");
				returnIndex = -1;
			}
			break;
		}
		else if(commList[i].type == IGNORE_COMMAND)
		{
			continue;
		}
		/*else if(commList[i].type == SEQUENCE_COMMAND)
		{
			//printf("i: %d\n", i);
			returnIndex = i;
			if(i == g_commandListLength-2 && commList[i+1].type == NEW_LINE)
			{
				returnIndex = -1;
			}
			break;
		}*/
		else
		{
			enum command_type input = commList[i].type;
			if(operatorIndex == 0)
			{
				operatorList[0] = i;
				operatorIndex++;
				//printStack(operatorIndex-1, operandIndex-1, operatorList, operandList);
				continue;
			}
			
			int topIndex = operatorIndex-1;
			enum command_type top = commList[operatorList[topIndex]].type;

		/*	printf("topIndex: %d\n", topIndex);
			printf("top: ");
			printCommandType(top);
			printf("input: ");
			printCommandType(input);*/

			while(checkPrecedence(input, top) && operatorIndex > 0)
			{
			//	printf("PRECEDENCE\n");
			//	printf("stack1:\n");
				//printStack(operatorIndex-1, operandIndex-1, operatorList, operandList);
				popOperator(operatorIndex-1, operandIndex-1, operatorList, operandList, commList);
				operandIndex--;
				operatorIndex--;
			//	printf("stack2:\n");
				//printStack(operatorIndex-1, operandIndex-1, operatorList, operandList);

				//printf("op index: %d\n", operatorIndex);
				// print stack for debugging
				topIndex = operatorList[operatorIndex-1];
				top = commList[topIndex].type;
			/*	printf("in loop topIndex: %d\n", topIndex);
				printf("in loop top: ");
				printCommandType(top);
				printf("in loop input: ");
				printCommandType(input);*/
			}


			operatorList[operatorIndex] = i;
			operatorIndex++;
		}

	}

	while(operatorIndex > 0)
	{
		popOperator(operatorIndex-1, operandIndex-1, operatorList, operandList, commList);
		operandIndex--;
		operatorIndex--;

	//	printf("stack3:\n");
	//	printStack(operatorIndex-1, operandIndex-1, operatorList, operandList);
	}
	free(operatorList);
	free(operandList);
	rootList[rootListIndex] = operandList[0];
	return returnIndex;
}

int countSequenceCommands(command_t commList)
{
	int count = 0;
	int i;
	for(i = 0; i < g_commandListLength; i++)
	{
		if(commList[i].type == NEW_LINE)
		{
			count++;
		}
	}
	return count;
}



command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
	//exit(1);
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
	//printf("make_command_stream");

	char* fileString = getTextFromFile(get_next_byte, get_next_byte_argument);
	//printString(fileString);
	//checkParen(fileString);

	//printf("before makeCommandList");
	command_t commList = makeCommandList(fileString);
	//printString(fileString);
	//printf("sizeList: %d\n", g_commandListLength);
	//printCommandList(commList, g_commandListLength);
	//printCommandList(commList, g_commandListLength);
	//printString(fileString);
	checkCommandList(commList);

	//printCommandList(commList, g_commandListLength);
	checkParen(commList);
	//printCommandList(commList, g_commandListLength);
	
	int numSequenceCommands = countSequenceCommands(commList);

	int* rootList = malloc(sizeof(int)*numSequenceCommands);
	int rootListIndex = 0;
	int i = 0;
	//int index = -1;
	int index = buildTree(commList, i, rootListIndex, rootList);
	while(index != -1)
	{
		rootListIndex++;
		//printf("rootListIndex: %d\n", rootListIndex);
	//	printf("last index: %d\n", index);
		index = buildTree(commList, index+1, rootListIndex, rootList);
	}
	int j;
	for(j = 0; j < numSequenceCommands+1; j++)
	{
		//printf("index %d: %d\n", j, rootList[j]);
	}

	int k;
	command_t* finalCommList = malloc(sizeof(command_t)*(numSequenceCommands+1));
	int kmax = numSequenceCommands;
	//printf("numSequenceCommands: %d\n", numSequenceCommands);
	int last = g_commandListLength-1;
	/*for(last; last> 0; last--)
	{
		if(commList[last].type != IGNORE_COMMAND && commList[last].type != NEW_LINE && commList[last].type != SEQUENCE_COMMAND)
		{
			break;
		}
		if(commList[last].type == SEQUENCE_COMMAND)
		{
			kmax--;
		}
	}*/
	/*if(commList[g_commandListLength-2].type == SEQUENCE_COMMAND)
	{
		printf("kmax--");
		kmax--;
	}
	else
	{

	}*/
	for(k = 0; k < kmax; k++)
	{
		//printf("add to final comm list: %d\n", k);
		int index = rootList[k];
		finalCommList[k] = &commList[index];
	}

	int x;
	for(x = 0; x < kmax; x++)
	{
		//printf("print  final comm list: %d\n", x);
		//print_command(finalCommList[x]);
	}
	
	command_stream_t treeList = malloc(sizeof(struct command_stream));
	treeList->treeHead = finalCommList;
	treeList->size = kmax;
	/*//int size = sizeof(command_t);
	//int k = sizeof(int*);
	//printf("size of int*: %d\n", k);
	//printf(typeof(command_t));
	//printf("size of command_t: %d\n", sizeof(command_t));
	*/
	/*int i;
	for(i = 0; i < 3; i++)
	{
		printf("commList[%d].status: %d", i, commList[i].status);
	}*/
	
  //error (1, 0, "command reading not yet implemented");
	/*char* ch = malloc(sizeof(char)*2);
	ch[0] = 't';
	ch[1] = 'e';
	printf("%s\n", ch);*/
	//free(rootList);
	free(fileString);
  return treeList;
  return 0;
}



command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
	//printf("read_cocommand reading not yet implemented");
	command_t returnComm =  *(s->treeHead);
	if(s->size != 0)
	{
		s->treeHead = &(s->treeHead[1]);
		s->size--;
	}
	else
	{
		return 0;
	}
	return returnComm;
//  return 0;
}




//mmand_stream");
	//printf("%c", get_next_byte(get_next_byte_argument));
  //error (1, 0, "