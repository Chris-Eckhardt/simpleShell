/****************************************************************
 * Name        :  Chris Eckhardt                                *
 * Class       :  CSC 415                                       *
 * Date        :  06/26/2019                                    *
 * Description :  Writting a simple bash shell program          *
 *                that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

/* CANNOT BE CHANGED */
#define BUFFERSIZE 256
/*--------------------*/
#define PROMPT ANSI_COLOR_GREEN "myshell@ %s >> " ANSI_COLOR_RESET,cwd
#define PROMPTSIZE sizeof(PROMPT)

/* MY CONSTANTS */
#define clear() printf("\033[H\033[J")
#define HOME "~/"
#define HOME_COMP_1 "/home/"
#define HOME_COMP_2 "/home"
#define EXIT_CMD "exit"
#define CD_CMD "cd"
#define PWD_CMD "pwd"
#define DELIMITER " \t\n\0"
#define PIPE "|"
#define REDIRECT_IN_SYMBOL "<"
#define REDIRECT_OUT_SYMBOL ">"
#define REDIRECT_APP_SYMBOL ">>"
#define BACKGROUND_SYMBOL "&"
#define REDIRECT_OUT_FLAG 1
#define REDIRECT_IN_FLAG 2
#define REDIRECT_APP_FLAG 3
#define BACKGROUND_FLAG 1
#define ARRAY_SIZE_STANDARD 24
/*------------------*/

/* COLORED TEXT */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[92m" // orig. 32
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[96m" // orig. 36
#define ANSI_COLOR_RESET   "\x1b[0m"
/*------------------*/

/////////////////////////////////////////////////
////////////////// COMMAND //////////////////////
/////////////////////////////////////////////////

/* this is the basic structure of the commands that are built from user input.
A command is a the entirety of all instructions entered at a single time, I'm calling these instructions "calls"
just because I couldnt figure out a better name. But each command contains an array of calls, these calls will be piped together
before execution. */

struct 
Call
{
    char * myargv [ARRAY_SIZE_STANDARD];
    int num_of_args;
};

struct 
Command {
    struct Call * calls [ARRAY_SIZE_STANDARD];
    int number_of_calls;
    int flag;
    int background;
    char * redirect_target;
};

/////////////////////////////////////////////////
//////////////// HISTORY ////////////////////////
/////////////////////////////////////////////////

/*
commands will be stored as their origional string input,
The input vatiable will be set to the current nodes value
and then parsed and turned into a struct command.

NOTE : didn't have time to impliment this. But leaving this so I can do it on my own later.
*/

struct 
Entry
{
    char * cmd;
    struct Entry * next;
    struct Entry * prev;
};


void
add_history(char * entry)
{


}

void
iterate_next() 
{

}

void
iterate_prev()
{

}

/////////////////////////////////////////////////
/////////////////// CD & PWD ////////////////////
/////////////////////////////////////////////////

/* this is the internally implimented CD and PWD commands. */

int 
cd(char * path) {
    return chdir(path);
}

char * 
pwd(char * pwd_buff) {
    
    if(getcwd(pwd_buff, BUFFERSIZE) == NULL) {
        perror(" error getting current working directory. ");
    } else {
        return pwd_buff;
    }
    return NULL;
}

/////////////////////////////////////////////////
//////////////// BUILD COMMAND //////////////////
/////////////////////////////////////////////////

/*
malloc is ok to use here because it is only called 
once at the start of a program.
If you're on some exotic OS that requires you to 
explicitly free resources then this may cause trouble.
*/

struct Command *
build_cmd()
{
    struct Command * cmd = malloc( sizeof(struct Command) );

    for(int i = 0; i < ARRAY_SIZE_STANDARD; i++) {
        cmd->calls[i] = malloc( sizeof(struct Call) );
        for(int j = 0; j < ARRAY_SIZE_STANDARD; j++) {
            cmd->calls[i]->myargv[j] = NULL;
        }
    }

    return cmd;
}

/////////////////////////////////////////////////
///////////////// CLEAR MYARGV //////////////////
/////////////////////////////////////////////////

/* 
this will zero out all command values, 
it is called at the end of every iteration
of the main loop.
 */
void
clear_cmd(struct Command * cmd)
{
    // zero out all values here ..
    for(int i = 0; i < cmd->number_of_calls; i++) {
        for(int j = 0; j < ARRAY_SIZE_STANDARD; j++) {
            cmd->calls[i]->myargv[j] = NULL;
        }
        cmd->calls[i]->num_of_args = 0;
    }
    cmd->flag = 0;
    cmd->background = 0;
    cmd->number_of_calls = 0;
    cmd->redirect_target = NULL;
}

/////////////////////////////////////////////////
//////////// PROMPT & GET CWD ///////////////////
/////////////////////////////////////////////////

/* The main part of this section is the prompt method, get_path() is a halper method that 
retrieves the current working directory and formats it for display */

char *
get_path(char * buffer)
{
    char cwd_buff[BUFFERSIZE];
    char * cwd = getcwd( cwd_buff, sizeof(cwd_buff) );
    char * pos;
    char * comp;


    if(pos = strstr(cwd, HOME_COMP_1)) 
    {
        comp = HOME_COMP_1;
    } 
    else if (pos = strstr(cwd, HOME_COMP_2)) 
    {
        comp = HOME_COMP_2;
    }
    else
    {
        return cwd;
    }
    
    strncpy(buffer, cwd, pos-cwd);
    buffer[pos-cwd] = '\0';
    sprintf( buffer+(pos-cwd) , "%s%s", HOME, pos+strlen(HOME_COMP_1) );

    return buffer;
}
/* prompt just gets the cwd, displays the prompt to the user and then waits for a command.
Once it recieves input from the user it returns a pointer to that string */

char *
prompt( char * input, char * buffer)
{
    // vars
    char * cwd = get_path(buffer);

    // check cwd
    if(cwd != NULL) {
        printf( PROMPT );
    } else {
        perror("error @ getcwd() in prompt() : ");
    }
    
    // get input
    fgets(input, BUFFERSIZE, stdin);

    // return input
    return input;
}

/////////////////////////////////////////////////
////////////////// PARSE INPUT //////////////////
/////////////////////////////////////////////////

/* parse_input goes through the string provided by the 
user and then uses it to build the command that is to be executed.*/

void
parse_input( char * input, struct Command * cmd)
{

    int call_index = 0;
    int arg_index = 0;
    int num_of_calls = 1;

    char * token = strtok(input, DELIMITER);
    cmd->flag = 0;

    while ( token != NULL ) {



        if ( strcmp(token, PIPE) == 0 ) { // if pipe, create new command in array, incriment all the things
            cmd->calls[call_index]->num_of_args = arg_index;
            num_of_calls++;
            call_index++;
            arg_index = 0;
        }
        else if ( strcmp(token, REDIRECT_OUT_SYMBOL) == 0 ) { // redirect out >
            cmd->flag = REDIRECT_OUT_FLAG;
            cmd->redirect_target = strtok(NULL, DELIMITER);
        }
        else if ( strcmp(token, REDIRECT_IN_SYMBOL) == 0 ) { // redirect in <
            cmd->flag = REDIRECT_IN_FLAG;
            cmd->redirect_target = strtok(NULL, DELIMITER);
        } 
        else if ( strcmp(token, REDIRECT_APP_SYMBOL) == 0 ) { //  redirect append >>
            cmd->flag = REDIRECT_APP_FLAG;
            cmd->redirect_target = strtok(NULL, DELIMITER);
        } 
        else if ( strcmp(token, BACKGROUND_SYMBOL) == 0 ) { // background process &
            cmd->background = BACKGROUND_FLAG;
        }
        else {                                              // if regular token it's assumed to be argument and added to myargv
            cmd->calls[call_index]->myargv[arg_index] = token;
            arg_index++;
        }
        
        token = strtok(NULL, DELIMITER); // next token might be NULL, should be assigned before the next loop check
    }
    cmd->calls[call_index]->num_of_args = arg_index;
    cmd->number_of_calls = num_of_calls; // assign num of calls to command member

}

///////////////////////////////////////////
/////////////// EXECUTE ///////////////////
///////////////////////////////////////////

/* This is where commands are executed. if there are multiple commands then pipes 
will direct input/output to their destinations.*/

static void
execute(struct Command * cmd)
{
    int fd[2];
    pid_t pid;
    int fd_alt = 0;

    for(int i = 0; i < cmd->number_of_calls; i++) {
        pipe(fd);

        if((pid = fork()) == -1) { // ERROR
            perror("Error at fork \n");
            exit(1);
        }
        else if(pid == 0) { // CHILD

            // redirect flag checks
            if (cmd->flag == REDIRECT_OUT_FLAG)
            {
                int fd_out = creat(cmd->redirect_target, 0666) ;
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }
            if (cmd->flag == REDIRECT_IN_FLAG)
            {
                int fd_in = open(cmd->redirect_target, O_RDONLY);
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }
            if (cmd->flag == REDIRECT_APP_FLAG)
            {
                int fd_app = open(cmd->redirect_target, O_WRONLY | O_APPEND, 0666) ;
                dup2(fd_app, STDOUT_FILENO);
                close(fd_app);
            }

            dup2(fd_alt, 0);
            if(cmd->calls[i+1]->myargv[0] != NULL) {
                dup2(fd[1], 1);
            }

            close(fd[0]);
            execvp(cmd->calls[i]->myargv[0], cmd->calls[i]->myargv);
            perror("Error at execvp \n");
            exit(1);
        }
        else { // PARENTAL UNIT

            if(BACKGROUND_FLAG) {
                wait(NULL);
            }
            close(fd[1]);
            fd_alt = fd[0];

        }
    }
}


/////////////////////////////////////////////////
//////////////////// MAIN ///////////////////////
/////////////////////////////////////////////////

/* Just the main method, it first allocates resources that are needed, then inside the loop
the user is prompted for input, that input is parsed and turned into a 
command that can be passed to execvp to be executed. at the end of the loop all of the 
values inside of the command are zeroed/nulled out so it can be reused in the next iteration. */

int
main(int argc, char ** argv) {

    int running = 1;
    char * input = malloc( sizeof(char) * BUFFERSIZE);
    char * buffer = malloc( sizeof(char) * BUFFERSIZE);
    char * pwd_buff = malloc(sizeof(char) * BUFFERSIZE);

    struct Command * cmd = build_cmd();

    clear();

    while (running) {

        input = prompt(input, buffer);

        parse_input(input, cmd);

        if(cmd->calls[0]->myargv[0] == NULL) {
            continue;
        } // check if args are null

        if (strcmp(cmd->calls[0]->myargv[0], EXIT_CMD) == 0) {
            if(cmd->calls[0]->myargv[1] == NULL) {
                running = 0;
                continue;
            }
            else {
                perror("exit command should have no arguments : ");
                continue;
            }
        }

        if (strcmp(cmd->calls[0]->myargv[0], CD_CMD) == 0) {
            if (cd(cmd->calls[0]->myargv[1]) < 0) {
                perror(cmd->calls[0]->myargv[1]);
            }
            clear_cmd(cmd);
            continue;
        }

        if (strcmp(cmd->calls[0]->myargv[0], PWD_CMD) == 0) {
            char * dir;
            if(( dir = pwd(pwd_buff)) == NULL) {
                perror("cant get dir ");
            } else {
                printf("%s\n", dir);
            }
            clear_cmd(cmd);
            continue;
        }
        
        execute(cmd);
        
        clear_cmd(cmd);

    } // exit main loop

    /* 
    free resources before exit.
    this is not usually necessary 
    but just in case you run this 
    on some exotic OS. However the build_cmd()
    may cause a problem in this case.
    */
    free(input);
    free(buffer);
    free(pwd_buff);

    return 0;
}