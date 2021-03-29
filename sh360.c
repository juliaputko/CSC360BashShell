#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h> 
#include <fcntl.h>

#define MAX_ARGS 7              //max arguments in any command 
#define MAX_INPUT_LINE 80       //max length of input line provided by user 
#define MAX_PROMPT 10           //max length of prompt 
#define MAX_DIR 10              //max number of directories in path 
#define MAX_DIR_NAMELEN 20

int main(int argc, char *argv[]) {
    char input[MAX_INPUT_LINE];                 //storing input from stdin 
    char input_buffer[MAX_INPUT_LINE];          //storing input from stdin                             
    char *token[MAX_PROMPT];                    //tokenizing 
    char dir[MAX_INPUT_LINE][MAX_INPUT_LINE];   //storing what is in .sh360rc
    char buffer[MAX_INPUT_LINE];                //buffer for fgets
    char *t;                                    //tokenizing 
    int num_tokens;                             //tokenizing 
    int i=0;                                    //incrementing in dir[] 
    int k =0;                                   //keeping track of directories we have gone through
    FILE* fp;                                   //file pointer
    char place[MAX_INPUT_LINE][MAX_INPUT_LINE]; //place holder for new and improved pathname 
    int pid;                                    //process id
    int status;                                 //process status
    char *envp[] = { 0 };                       //environement variable 
    int fd;                                     //file opener 
    int fd2[2];
    int pid_head, pid_tail;

   if ((fp = fopen("sh360rc", "r")) == NULL){   //open file with directories listed
       fprintf(stderr, "Error opening file\n"); //print error message if there is an error 
       exit(0);
   }
                                                            // remove newline off the end of what is retreived by fgets 
   while(i< MAX_INPUT_LINE && fgets(buffer, MAX_INPUT_LINE, fp) != NULL){
       buffer[strcspn(buffer, "\n")] = 0;                   //https://www.tutorialspoint.com/c_standard_library/c_function_strcspn.htm
       strncpy(dir[i], buffer, MAX_INPUT_LINE);             //copy buffer of fgets into an array called dir 
       i++;
    }

    for(;;) {     
                                                                //infinite for loop 
        k=0;                                                    //reset k back to 0 
        fprintf(stdout, "%s ", dir[0]);                                                                              //prompt for our shell
        fflush(stdout);                                         //flush the stream 
        fgets(input, MAX_INPUT_LINE, stdin);                    //get user input from stdin
        if (input[strlen(input) - 1] == '\n') {                 // if input is "enter", reprompt with "uvic%"
            input[strlen(input) - 1] = '\0';
                                                                //repetedly prompt user with uvic %
        }
        if (strcmp(input, "exit") == 0) {                       //if user enters exit, exit 
           exit(0);
        }

        strncpy(input_buffer, input, MAX_INPUT_LINE);   //copy stin into a string to use with token so og string isnt destroyed

        if (input_buffer[strlen(input_buffer)-1] != '\0'){                   //store stdin in token 
            num_tokens = 0;
            t = strtok(input_buffer, " ");                            //input is what the user inputs in stdin 
            while (t != NULL && num_tokens < MAX_PROMPT) {
                token[num_tokens] = t;
                num_tokens++;
                t = strtok(NULL, " ");
            } 
          
               //"OR "commands:
            if(strcmp(token[0], "OR")== 0){ //user inputs OR followed by a space 
                for(int j=1; j< MAX_DIR; j++){

                    strncpy(place[j-1], dir[j], MAX_INPUT_LINE); //copy dir[j] into place holder place[j-1], where j=1 
                    strcat(place[j-1], "/");                     //add a /  to the pathanme 
                    strcat(place[j-1], token[1]);                //add the input arg command to the pathname 

                    char *args[] = {place[j-1], token[2], 0};   //place [j-1] now contains a path from .sh360rc, token [1] has the cmd line args 
                                                                //puts(args[1]); //print what is in place j-1


                    if (k== 8){                                 //executable not found anywhere and therefore input results in error message
                        fprintf(stderr, "Does not exist, or improperly formatted\n");
                    }
                                                                //create child process
                    if ((pid = fork()) == 0) {
                        fd = open(token[4], O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
                                                                                //OCREAT : if file does not exist it will be created by open 
                                                                                //ORDWR: file read/write
                                                                                //S_IRUSR: user has read permissions
                                                                                //S_IWUSR: user has write permissions

                         if (fd == -1) {
                            fprintf(stderr, "cannot open output.txt for writing\n");
                            exit(1);
                        }

                        dup2(fd, 1); //file descriptor corresponding to stream1(stdout) , now connected to what fd refers to 
                        dup2(fd, 2); //file descriptor corresponding to 2, now connected to what fd refers to 
                                                                //printf("child: about to start...\n");
                        execve(args[0], args, envp);            //start the child process with the initial path 
                        k++; 
                       // printf("child: SHOULDN'T BE HERE.\n");                                   //if doesnt execute, we end up here 
                                                                // if k reaches 8, the executable was not found anytwhere in 
                                                                //the list of file paths 
                    } //end if 

                    waitpid(pid, &status, 0); //waitpid() pid: get status info, and pid = fork() == 0 so child process starts
                    
                

                } //end for 

                exit(0);

            } //end if

         
            //"PP " commands:
            else if(strcmp(token[0], "PP")){


                for(int j=1; j< MAX_DIR; j++){

                    strncpy(place[j-1], dir[j], MAX_INPUT_LINE); //copy dir[j] into place holder place[j-1], where j=1 
                    strcat(place[j-1], "/");                     //add a /  to the pathanme 
                    strcat(place[j-1], token[1]);                //add the input arg command to the pathname 
                    char *cmd_head[] = { place[j-1], token[2], 0 };
                    if (k== 8){                                 //executable not found anywhere and therefore input results in error message
                        fprintf(stderr, "Does not exist, or improperly formatted\n");
                    }
                                                                //create child process
                    if ((pid_head = fork()) == 0) {
                        dup2(fd2[1], 1);
                        close(fd2[0]);
                        execve(cmd_head[0], cmd_head, envp);
                        k++;
                      
                                                                //if doesnt execute, we end up here 
                                                                // if k reaches 8, the executable was not found anytwhere in 
                                                                //the list of file paths 
                    } //end if 

                } //end for 

                k=0;
                for(int j=1; j< MAX_DIR; j++){

                    strncpy(place[j-1], dir[j], MAX_INPUT_LINE); //copy dir[j] into place holder place[j-1], where j=1 
                    strcat(place[j-1], "/");                     //add a /  to the pathanme 
                    strcat(place[j-1], token[4]);                //add the input arg command to the pathname 
                    char *cmd_tail[] = { place[j-1], token[5], 0 };
                    if (k== 8){                                 //executable not found anywhere and therefore input results in error message
                        fprintf(stderr, "Does not exist, or improperly formatted\n");
                    }
                                                                //create child process
                    if ((pid_tail = fork()) == 0) {
                                                                // printf("child (tail): re-routing plumbing; STDOUT to pipe.\n");
                        dup2(fd2[0], 0);
                        close(fd2[1]);
                        execve(cmd_tail[0], cmd_tail, envp);
                        k++;
                                                                //if doesnt execute, we end up here 
                                                                // if k reaches 8, the executable was not found anytwhere in 
                                                                //the list of file paths 
                    } //end if 
                } //end for
                                                            //waitpid(pid, &status, 0); //waitpid() pid: get status info, and pid = fork() == 0 so child process starts
                close(fd2[0]);
                close(fd2[1]);
                waitpid(pid_head, &status, 0);
                waitpid(pid_tail, &status, 0); 
                exit(0);
        
            }//end else
            
            //regular command execution: 
            else{

                for(int j=1; j< MAX_DIR; j++){

                    strncpy(place[j-1], dir[j], MAX_INPUT_LINE); //copy dir[j] into place holder place[j-1], where j=1 
                    strcat(place[j-1], "/");                     //add a /  to the pathanme 
                    strcat(place[j-1], token[0]);                //add the input arg command to the pathname 

                    char *args[] = {place[j-1], token[1], 0};   //place [j-1] now contains a path from .sh360rc, token [1] has the cmd line args 
                                                                //puts(args[1]); //print what is in place j-1


                    if (k== 7){                                 //executable not found anywhere and therefore input results in error message
                        fprintf(stderr, "Does not exist, or improperly formatted\n");
                    }
                                                                //create child process
                    if ((pid = fork()) == 0) {
                        execve(args[0], args, envp);            //start the child process with the initial path 
                        k++;                                    //if doesnt execute, we end up here 
                                                                // if k reaches 8, the executable was not found anytwhere in the list of file paths 
                    } //end if 

                    while (wait(&status) > 0) {                 // no longer waiting for child process to finish 
                                                                // printf("parent: child is finished.\n");
                    }
                } //end for
            }//end else 
        } // end if != /0
    } //end infinity for  
} //end main 


