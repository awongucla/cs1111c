// UCLA CS 111 Lab 1 command internals

enum command_type
  {
    AND_COMMAND,         // A && B //1
    SEQUENCE_COMMAND,    // A ; B // 2
    OR_COMMAND,          // A || B // 3 
    PIPE_COMMAND,        // A | B // 4
    SIMPLE_COMMAND,      // a simple command // 5
    SUBSHELL_COMMAND,    // ( A ) // 6

	NEW_LINE, 
	OPEN_PAREN, // ( A
	CLOSED_PAREN, // A )
	LEFT_REDIRECT, // A < B
	RIGHT_REDIRECT, // A > B
	IGNORE_COMMAND,

  };

// Data associated with a command.
struct command
{
  enum command_type type;

  // Exit status, or -1 if not known (e.g., because it has not exited yet).
  int status;

  // I/O redirections, or 0 if none.
  char *input;
  char *output;

  union
  {
    // for AND_COMMAND, SEQUENCE_COMMAND, OR_COMMAND, PIPE_COMMAND:
    struct command *command[2];

    // for SIMPLE_COMMAND:
    char **word;

    // for SUBSHELL_COMMAND:
    struct command *subshell_command;
  } u;

  int numWords; // for SIMPLE_COMMAND ONLY
};
