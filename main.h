#include <stdio.h>

#define SH_RL_BUFSIZE 1024
#define SH_TOK_BUFSIZE 64
#define SH_COMMAND_BUFSIZE 1024
#define SH_NUM_COMMANDS 10
#define SH_TOK_DELIM " \t\r\n\a"
#define SH_OUTPUT_REDIRECT ">"
#define SH_PIPE "|"
#define APPEND "a+"
#define READ "r"

typedef char command[SH_COMMAND_BUFSIZE];

void mysh_loop(); // Function prototype
char* sh_read_line();
char** sh_parse_line(char*);
command* sh_parse_multiple_cmds(char **);
int sh_execute(char**);
int sh_launch(char**);
int sh_execute_multiple_cmds(command *);


int sh_builtin_cd(char **args);
int sh_builtin_help(char **args);
int sh_builtin_exit(char **args);
int sh_builtin_history(char **args);
int sh_len_builtin();



