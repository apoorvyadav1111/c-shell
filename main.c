#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_ssize_t.h>
#include <sys/syslimits.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <libgen.h>
#include <limits.h>
#include "main.h"


char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "history"
};

int (*builtin_func[]) (char **) = {
  &sh_builtin_cd,
  &sh_builtin_help,
  &sh_builtin_exit,
  &sh_builtin_history
};

int sh_builtin_cd(char **args)
{
    if (args[1] == NULL){
        if(chdir("~") != 0){
            perror("mysh");
        }
    } else {
        if(chdir(args[1]) != 0){
            perror("mysh");
        }
    }
    return 1;
}

int sh_builtin_help(char **args){
    int i;
    printf("\nApoorv Yadav's MYSH (MyShell)\n\n");
    printf("Based on Stephen Brennan's LSH\n");
    printf("https://brennan.io/2015/01/16/write-a-shell-in-c/\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < sh_len_builtin(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int sh_builtin_exit(char **args){
    return 0;
}

int sh_builtin_history(char **args){
    FILE *fptr;
    if ((fptr = fopen(".myshhistory", READ))==NULL){
        fprintf(stderr,"Error! opening file");
        exit(EXIT_FAILURE);
    }
    char c;
    c = fgetc(fptr);
    while (c != EOF){
        printf ("%c", c);
        c = fgetc(fptr);
    }
    fclose(fptr);
    return 1;
}

int sh_len_builtin(){
    return sizeof(builtin_str) / sizeof(char *);
}

int main(int argc, char **argv) 
{
    mysh_loop();
    return EXIT_SUCCESS;
}

void mysh_loop()
{
    char *line;
    char **args;
    command *cmds;
    FILE *fptr;
    int status;
    printf("user login: ");
    char *userlogin = NULL;
    ssize_t bufsize = 0;
    if (getline(&userlogin, &bufsize, stdin) == -1){
        if (feof(stdin)) {
            exit(EXIT_SUCCESS); 
        } else  {
            perror("mysh: cannot read user");
            exit(EXIT_FAILURE);
        }
    }
    
    if (userlogin[strlen(userlogin) - 1] == '\n') {
        userlogin[strlen(userlogin) - 1] = '\0';
    }
    do{
        char cwd[PATH_MAX];
        char *folder_name;
        int i;

        if (getcwd(cwd, sizeof(cwd)) != NULL){
            folder_name = basename(cwd);
        } else {
            perror("mysh: cannot get current working directory");
        }
        printf("%s %s %% ", userlogin, folder_name);
        line = sh_read_line();
        args = sh_parse_line(line);
        cmds = sh_parse_multiple_cmds(args);

        if ((fptr = fopen(".myshhistory", APPEND))==NULL){
            fprintf(stderr,"Error! opening file");
            exit(EXIT_FAILURE);
        }
        while(args[i] != NULL){
            fwrite(args[i], 1, strlen(args[i]), fptr);
            fwrite(" ", 1, 1, fptr);
            i++;
        }
        fwrite("\n", 1, 1, fptr);
        fclose(fptr);
        status = sh_execute(args);
        free(line);
        free(args);
    }while(status);
    free(userlogin);
}

char *sh_read_line()
{
    int bufsize = SH_RL_BUFSIZE;
    int position = 0;
    char *buf = malloc(sizeof(char) * bufsize);
    int c;
    
    if(!buf){
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        c = getchar();
        if (c==EOF || c=='\n'){
            buf[position] = '\0';
            return buf;
        } else {
            buf[position] = c;
        }
        position++;

        if(position >= bufsize){
            bufsize += SH_RL_BUFSIZE;
            buf = realloc(buf, bufsize);
            if(!buf){
                fprintf(stderr, "mysh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **sh_parse_line(char *line)
{
    int bufsize = SH_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize*sizeof(char*));
    char *token;

    if(!tokens){
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SH_TOK_DELIM);
    while(token!=NULL){
        tokens[position] = token;
        position++;

        if (position >= bufsize){
            bufsize += SH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize*sizeof(char*));
            if(!tokens){
                fprintf(stderr, "mysh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, SH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

command* sh_parse_multiple_cmds(char **args){
    printf("Here\n");
    command *cmds = malloc(sizeof(command)*SH_NUM_COMMANDS);
    int i = 0;
    int j = 0;
    int k = 0;
    while(args[i] != NULL){
        if(strcmp(args[i], SH_PIPE) == 0){
            cmds[j][k] = '\0';
            printf("%s",cmds[j]);
            j++;
        } else {
            cmds[j][k] = args[i];
            j++;
        }
        i++;
    }
    cmds[j][k] = '\0';
    return cmds;
}

int sh_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0){
        // In Child
        if(execvp(args[0], args) == -1){
            perror("mysh");
        }
        exit(EXIT_FAILURE);
    } else if(pid<0){
        // Fork Failed
        perror("mysh");
    } else {
        do {
            wpid = waitpid(pid,&status,WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int sh_execute(char ** args){
    int i;

    if(args[0] == NULL){
        return 1;
    }

    for(i = 0; i < sh_len_builtin(); i++){
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return sh_launch(args);
}

int sh_execute_multiple_cmds(command *cmds){
    int i = 0;
    // int tmp_in = dup(0);
    // int tmp_out = dup(1);

    // // Current support only > operator
    // // incase of support for < op, following logic needs to be updated
    // int fdin = dup(tmp_in);

    // int ret;
    // int fdout;

    while(cmds[i] != NULL){
        // sh_execute((char**)(cmds[i]));
        printf(cmds[i]);
        i++;
    }
    return 1;
}