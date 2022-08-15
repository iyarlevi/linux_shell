/*
 * "mini shell"
 * C Program that act like a linux shell
 * iyar levi
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

#define INPUT_LEN 512

void printDetails(FILE*, char[]);
int checkForPipe(char[]);
int howManyPipes(char[]);
char** fillArgv(char inputStr[], int wordCount);
char** split(char* inputArr, char* delim);
void removeSpaces(char str[]);
void noExtraSpaces(char str[]);
void freeArgv(char** argv, int wordCount);
int howManyWords(char inputStr[]);
void sig_handler(int sigNum);

int main() {

    char inputStr[INPUT_LEN] = "";
    char word[INPUT_LEN] = "";
    char readingStr[INPUT_LEN] = "";
    char space = ' ';
    char cwd[256]; // array that hold the current path
    int wordCount = 0, charCount = 0, numOfCom = 0, numOfWords = 0, numOfPipes = 0, umpFlag = 0, i;
    int numOfLines = 0, lineCheck = 0;
    int wordCountLeft, wordCountRight, wordCountMid;
    int status; // son return status
    char** argvRight;
    char** argvLeft;
    char** argvMid;
    char** argvRight2;
    char** argvLeft2;
    char** outputArr;
    char** outputArr2;
    pid_t pidSon;
    pid_t pidSonInput;
    pid_t pidSonMid;
    pid_t pidSonOutput;
    FILE *fp; // pointer to the file - "file.txt"

    while(strcmp(inputStr, "done") != 0) {
        if (getcwd(cwd, sizeof(cwd)) == NULL)
            perror("getcwd() error\n"); // if there is an error
        else
            printf("%s>", cwd);
        fgets(inputStr, INPUT_LEN, stdin); // the input from the user
        inputStr[strlen(inputStr) - 1] = '\0';

        for (i = 0; inputStr[i] != '\0'; i++)  // check how many words and how many char is in the input string
        {
            if (inputStr[i] == ' ' && inputStr[i + 1] != ' ' && inputStr[i + 1] != '\0')
                wordCount++;
            if (inputStr[i] != ' ' && inputStr[i] != '\0')
                charCount++;
        }
        if(inputStr[0] != ' '){ // count the first word
            wordCount++;
        }
        if(inputStr[i-1] == '&'){ // raise a flag if there is a '&' char awt the end
            umpFlag = 1;
            inputStr[i-1] = '\0';
        }
        if(strcmp(inputStr, "done") == 0){ // if the input is "done", finish the loop
            numOfCom++; // increase how many commands we have
            break;
        }
        removeSpaces(inputStr); // remove all the extra spaces
        for(int t = strlen(inputStr); t < INPUT_LEN; t++){ // clean all the leftovers from the original input
            inputStr[t] = '\0';
        }

        for(int iter = 0; inputStr[iter]; ++iter){ // take care of the spaces
            if(howManyPipes(inputStr) == 1){ // if there is 1 pipe
                if(inputStr[iter] == '|' && inputStr[iter-1] == ' ' && inputStr[iter+1] == ' '){
                    noExtraSpaces(inputStr);
                }
            }
            else if(howManyPipes(inputStr) == 2){ // if there is 2 pipes
                if(inputStr[iter] == '|' && inputStr[iter-1] == ' ' && inputStr[iter+1] == ' '){
                    noExtraSpaces(inputStr);
                }
            }
        }

        if(inputStr[0] == '!'){ // if the input is '!' with a number
            char* cutInput = inputStr + 1;
            int inputAsInt = atoi(cutInput)-1;
            fp = fopen("file.txt","r");
            if(fp == NULL)
            {
                printf("cannot open file\n");
                return 1;
            }
            while (fgets(readingStr, INPUT_LEN, fp)!=NULL){ // reading one line every loop
                if(lineCheck == inputAsInt){ // check if you in the right line
                    readingStr[strlen(readingStr) - 1] = '\0';
                    for(int y = 0; y < INPUT_LEN; y++){
                        inputStr[y] = '\0';
                    }
                    strcpy(inputStr, readingStr);
                    break;
                }
                else{
                    lineCheck++;
                }
            }
            fclose(fp);
            if (lineCheck < inputAsInt){ // if the input is greater than the number of commands in the file
                printf("NOT IN HISTORY\n");
                continue;
            }
        }
        lineCheck = 0;

        if(strcmp(inputStr, "cd") == 0){ // if the command is "cd"
            printf("command not supported (Yet)\n");
            numOfCom++;
            numOfWords++;
            continue;
        }

        if(strcmp(inputStr, "history") == 0){ // if the input is "history", print the history of commands
            fp = fopen("file.txt","r");
            if(fp == NULL)
            {
                printf("cannot open file\n");
                return 1;
            }
            while(fgets(readingStr, INPUT_LEN, fp)!=NULL) {
                printf("%d%c%s", numOfLines, space, readingStr); // print the history in a specific format
                numOfLines++; // counting how many commands is in the history file
            }
            fclose(fp);
            printDetails(fp, inputStr); // calling the helper function
            numOfLines = 0;
            continue;
        }

        if(checkForPipe(inputStr) == 0){ // if the input contains the char "|" that represent a pipe
            if(howManyPipes(inputStr) == 1){ // if there is one pipe
                numOfPipes++;
                printDetails(fp, inputStr); // calling the helper function
                outputArr = split(inputStr, "|"); // splitting the input before and after the pipe

                char* leftCommand = outputArr[0]; // the left command
                char* rightCommand = outputArr[1]; // the right command

                int pipe_fd[2]; // create a pipe
                if(pipe(pipe_fd) == -1){
                    perror("cannot open pipe\n");
                    exit(EXIT_FAILURE);
                }

                if((pidSonInput = fork()) == 0){ // the inputSon process
                    dup2(pipe_fd[1], STDOUT_FILENO); // changing the fd output to be to the pipe
                    close(pipe_fd[0]);
                    close(pipe_fd[1]);

                    strcpy(inputStr, leftCommand); // put the left command as inputStr
                    wordCountLeft = howManyWords(inputStr); // count how many words in the command

                    for(int t = strlen(leftCommand); t < INPUT_LEN; t++){ // clean all the leftovers from the original input
                        inputStr[t] = '\0';
                    }

                    argvLeft = fillArgv(inputStr, wordCountLeft); // fill the argv with the correct values

                    if(execvp(argvLeft[0], argvLeft) == -1){ // if there was an error
                        perror("execvp() left failed\n");
                        exit(1);
                    }
                    freeArgv(argvLeft, wordCountLeft); // free the array
                }
                else if((pidSonOutput = fork()) == 0){ // the outputSon process
                    dup2(pipe_fd[0], STDIN_FILENO);
                    close(pipe_fd[1]);
                    close(pipe_fd[0]);

                    strcpy(inputStr, rightCommand); // put the right command as inputStr
                    wordCountRight = howManyWords(inputStr); // count how many words in the command

                    for(int t = (int)strlen(inputStr); t < INPUT_LEN; t++){ // clean all the leftovers from the original input
                        inputStr[t] = '\0';
                    }
                    argvRight = fillArgv(inputStr, wordCountRight); // fill the argv with the correct values

                    if(execvp(argvRight[0], argvRight) == -1){ // if there was an error
                        perror("execvp() right failed\n");
                        exit(1);
                    }
                    freeArgv(argvRight, wordCountRight); // free the array
                }
                else{ // the father process
                    close(pipe_fd[1]);
                    close(pipe_fd[0]);
                    wait(&status);
                    wait(&status);
                    freeArgv(outputArr, 2);
                }
            }

            else if(howManyPipes(inputStr) == 2){ // if there is two pipes
                numOfPipes += 2;

                printDetails(fp, inputStr); // calling the helper function
                outputArr2 = split(inputStr, "|"); // splitting the input before and after the pipe ("|")

                char* leftCommand = outputArr2[0]; // the left command
                char* midCommand = outputArr2[1]; // the mid-command
                char* rightCommand = outputArr2[2]; // the right command

                int pipe_fd1[2]; // create a pipe
                if(pipe(pipe_fd1) == -1){
                    perror("cannot open pipe\n");
                    exit(EXIT_FAILURE);
                }

                int pipe_fd2[2]; // create a pipe
                if(pipe(pipe_fd2) == -1){
                    perror("cannot open pipe\n");
                    exit(EXIT_FAILURE);
                }

                if((pidSonInput = fork()) == 0){ // the inputSon process
                    dup2(pipe_fd1[1], STDOUT_FILENO); // changing the fd output to be to the pipe
                    close(pipe_fd1[0]);
                    close(pipe_fd1[1]);
                    close(pipe_fd2[0]);
                    close(pipe_fd2[1]);

                    strcpy(inputStr, leftCommand); // put the left command as inputStr
                    wordCountLeft = howManyWords(inputStr); // count how many words in the command

                    for(int t = (int)strlen(inputStr); t < INPUT_LEN; t++){ // clean all the leftovers from the original input
                        inputStr[t] = '\0';
                    }

                    argvLeft2 = fillArgv(inputStr, wordCountLeft); // fill the argv with the correct values

                    if(execvp(argvLeft2[0], argvLeft2) == -1){ // if there was an error
                        perror("execvp() left failed\n");
                        exit(1);
                    }
                    freeArgv(argvLeft2, wordCountLeft); // free the array
                }
                else if((pidSonMid = fork()) == 0) { // the outputSon process
                    dup2(pipe_fd1[0], STDIN_FILENO);
                    dup2(pipe_fd2[1], STDOUT_FILENO);
                    close(pipe_fd1[1]);
                    close(pipe_fd2[0]);
                    close(pipe_fd1[0]);
                    close(pipe_fd2[1]);

                    strcpy(inputStr, midCommand); // put the right command as inputStr
                    wordCountMid = howManyWords(inputStr); // count how many words in the command

                    for(int t = (int)strlen(inputStr); t < INPUT_LEN; t++){ // clean all the leftovers from the original input
                        inputStr[t] = '\0';
                    }

                    argvMid = fillArgv(inputStr, wordCountMid); // fill the argv with the correct values

                    if(execvp(argvMid[0], argvMid) == -1){ // if there was an error
                        perror("execvp() left failed\n");
                        exit(1);
                    }
                    freeArgv(argvMid, wordCountMid); // free the array
                }

                else if((pidSonOutput = fork()) == 0){ // the outputSon process
                    dup2(pipe_fd2[0], STDIN_FILENO);
                    close(pipe_fd2[1]);
                    close(pipe_fd2[0]);
                    close(pipe_fd1[0]);
                    close(pipe_fd1[1]);

                    strcpy(inputStr, rightCommand); // put the right command as inputStr
                    wordCountRight = howManyWords(inputStr); // count how many words in the command

                    for(int t = (int)strlen(inputStr); t < INPUT_LEN; t++){ // clean all the leftovers from the original input
                        inputStr[t] = '\0';
                    }
                    argvRight2 = fillArgv(inputStr, wordCountRight); // fill the argv with the correct values

                    if(execvp(argvRight2[0], argvRight2) == -1){ // if there was an error
                        perror("execvp() right failed\n");
                        exit(1);
                    }
                    freeArgv(argvRight2, wordCountRight); // free the array

                }
                else{ // the father process
                    close(pipe_fd1[1]);
                    close(pipe_fd1[0]);
                    close(pipe_fd2[1]);
                    close(pipe_fd2[0]);
                    wait(&status);
                    wait(&status);
                    wait(&status);
                    freeArgv(outputArr2, 3);
                }
            }
            else{ // if there is more than two pipes
                printf("the system doesn't support more than two pipes\n");
                continue;
            }
            continue;
        }

        char** argv = (char**)malloc(sizeof(char*)*(wordCount+1)); // allocate dynamic memory for the execvp
        int a = 0, b = 0, c = 0;
        while (inputStr[a] != '\0') { // check the words and put it in the argv[]   // replace to while!!!!
            while (inputStr[a] != ' ' && inputStr[a] != '\0') {
                word[b] = inputStr[a];
                b++;
                a++;
            }
            word[b] = '\0';
            argv[c] = (char *) malloc(sizeof(char) * (b)); // allocate memory
            strcpy(argv[c], word);
            c++; // move to the next index in argv
            b = 0;
            a++;
        }
        if(wordCount == 1 && inputStr[0] == '\0'){ // if the input is only the key "ENTER"
            wordCount--;
            continue;
        }
        else if(charCount == 0){ // if the input is only spaces
            numOfCom--;
        }
        a = 0;
        argv[c] = NULL; // put NULL in the end of argv array
        c = 0;

        if(strcmp(argv[0], "nohup") == 0) { // if there is "nohup" at the start of the input
            pid_t t;
            t = fork();
            if (t == 0) {
                close(STDIN_FILENO);
                int fd = open("nohup.txt", O_WRONLY | O_CREAT | O_APPEND,
                              S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH); // open nohup.txt file
                int value = dup2(fd, STDERR_FILENO);
                if (value == -1) {
                    fprintf(stderr, "dup2 failed.\n");
                    exit(1);
                }
                signal(SIGHUP, SIG_IGN); // ignore the SIGHUP signal
                close(STDOUT_FILENO);
                if ((execvp(argv[1], argv + 1)) == -1) {
                    perror("execvp failed.\n");
                    exit(1);
                }
            }
            continue;
        }

        pidSon = fork(); // creating new process
        if(pidSon == -1){
            perror("fork failed\n");
            exit(1);
        }
        if(pidSon == 0){ // if you are the son process
            execvp(argv[0], argv);
            if(execvp(argv[0], argv) == -1){ // if there was an error
                perror("execvp() failed\n");
                exit(1);
            }
            for(int m = 0; m < wordCount; m++){ // free all the dynamic allocated memory
                free(argv[m]);
            }
            free(argv);
            exit(0);
        }
        else{ // if you are the father process, wait to the son process
            if(umpFlag == 1){ // if there is an '&' at the end of the input
                signal(SIGCHLD, sig_handler);
            }
            else{ // if there is no '&'
                wait(&status);
            }
            if(WEXITSTATUS(status) == 0){ // if the son return without a problem
                numOfCom++;
                numOfWords += wordCount;
                printDetails(fp, inputStr); // calling the helper function
            }
            for(int m = 0; m < wordCount; m++){ // free all the dynamic allocated memory
                free(argv[m]);
            }
            free(argv);
        }
        wordCount = 0;
        umpFlag = 0;
    }
    return 0;
}

void printDetails(FILE* fp, char inputStr[]){ // helper function that print to the file
    fp = fopen("file.txt","a");
    if(fp == NULL) // Check if file exists
    {
        printf("cannot open file\n");
    }
    fputs(inputStr, fp); // write the input to the file
    fputs("\n", fp);
    fclose(fp);
}

int checkForPipe(char input[]){ // helper function that check if the input contain pipes
    int pipeFlag = 1;
    if(strchr(input, '|') != NULL){
        pipeFlag = 0;
    }
    return pipeFlag;
}

int howManyPipes(char input[]){ // helper function that check how many pipes there is in the input
    int pipeCounter = 0;
    for(int i = 0; i < strlen(input); i++){
        if(input[i] == '|'){
            pipeCounter++;
        }
    }
    return pipeCounter;
}

char** split(char* input, char* delim) { // helper function that split the input by the delimiter
    char *temp;
    char** output2 = (char**)malloc(sizeof(char*)*(3));
    temp = strtok(input, delim);

    for(int i = 0; temp != NULL; i++) {
        output2[i] = (char*)malloc(sizeof(char)*(strlen(temp)));
        strcpy(output2[i], temp);
        temp = strtok(NULL, delim);
    }
    return output2;
}

int howManyWords(char inputStr[]){ // helper function that count how many words is in the input
    int wordCount = 0;
    for (int i = 0; inputStr[i] != '\0'; i++)  // check how many words and how many char is in the input string
    {
        if (inputStr[i] == ' ' && inputStr[i + 1] != ' ' && inputStr[i + 1] != '\0')
            wordCount++;
    }
    if (inputStr[0] != ' '){ // count the first word
        wordCount++;
    }
    return wordCount;
}

char** fillArgv(char inputStr[], int wordCount){ // helper function that fill the argv array with the input
   char word[INPUT_LEN];
   char** argv = (char**)malloc(sizeof(char*)*(wordCount+1)); // allocate dynamic memory for the execvp
   if(argv == NULL){
       printf("cannot allocate memory");
       exit(1);
   }
   int k=0,j=0,n=0;
    while (inputStr[k] != '\0') { // check the words and put it in the argv[]
        while (inputStr[k] != ' ' && inputStr[k] != '\0') {
            word[j] = inputStr[k];
            j++;
            k++;
        }
        word[j] = '\0';
        argv[n] = (char*)malloc(sizeof(char)*(j)); // allocate memory
        if(argv[n] == NULL){
            printf("cannot allocate memory");
            free(argv);
            exit(1);
        }
        strncpy(argv[n], word,j);
        n++; // move to the next index in argv
        j = 0;
        k++;
    }
    argv[n] = NULL; // put NULL in the end of argv array
    return argv;
}

void freeArgv(char** Arr, int wordCounter){ // helper function that free al the dynamic allocated memory
    for(int ma = 0; ma <= wordCounter; ma++){ // free all the dynamic allocated memory
        free(Arr[ma]);
    }
    free(Arr);
}

void noExtraSpaces(char str[]){ // helper function that remove the extra spaces in the input
    char temp[512];
    int index, index2, index3, index4;
    if(howManyPipes(str) == 1){ // if there is 1 pipe
        for(index =0; str[index]; ++index){
            if(str[index] == '|' && str[index-1] == ' ' && str[index+1] == ' '){
                for(index2 = 0; str[index2] != '|'; ++index2){
                    temp[index2] = str[index2];
                }
                temp[index2-1] = '|';
                break;
            }
        }
        for(index3 = index2; str[index3] != '\0'; ++index3){
            temp[index3] = str[index+2];
            index++;
        }
        temp[index+2] = '\0';
        strcpy(str, temp);
        for(int t = strlen(str); t < INPUT_LEN; t++){ // clean all the leftovers from the original input
            str[t] = '\0';
        }
    }
    if(howManyPipes(str) == 2){ // if there is 2 pipes
        for(index =0; str[index]; ++index){
            if(str[index] == '|' && str[index-1] == ' ' && str[index+1] == ' '){
                for(index2 = 0; str[index2] != '|'; ++index2){
                    temp[index2] = str[index2];
                }
                temp[index2-1] = '|';
                break;
            }
        }
        for(index3 = index2; str[index3+2] != '|'; ++index3){
            temp[index3] = str[index+2];
            index++;
        }
        temp[index3-1] = '|';
        for(index4 = index3; str[index4+4] != '\0'; ++index4){
            temp[index4] = str[index4+4];
        }
        temp[index4+4] = '\0';
        strcpy(str, temp);
        for(int t = strlen(str) - 1; t < INPUT_LEN; t++){ // clean all the leftovers from the original input
            str[t] = '\0';
        }
    }
}
void removeSpaces(char str[]){ // helper function that remove all the spaces in the input
    int i, x;
    for(i=x=0; str[i]; ++i)
        if(!isspace(str[i]) || (i > 0 && !isspace(str[i-1])))
            str[x++] = str[i];
    str[x] = '\0';
}

void sig_handler(int sigNum){ // handler function for the '&' case
    waitpid(-1, NULL, WNOHANG);
}
