linux shell
authored by Iyar Levi

------------DESCRIPTION--------------------------------------------

The program get string as an input fron the user.
The program will run the input as a command.
the program can work with 1 or 2 pipes in addition to the "normal" commands.
all the command will added to a file called file.txt.
If the input is the word "history" - the program will print all the previous inputs.
If the input is the word "done" - the program will exit and finish.
the program also support "nohup" and '&'.

------------functions----------------------------------------------

printDetails - function that gets the file and the input
               and print the input string to the file.

checkForPipe - function that check if there is a pipe in the input.

howManyPipes - function that check how many pipes are in the input.

fillArgv - function that fill the array with the command from the input.

split - function that split the input by the char '|'.

removeSpaces - function that remove all the spaces fron the input.

noExtraSpaces - function that remove only spacific spaces from the input.

freeArgv - function that free 2D arrays.

howManyWords - function that count how many words are in the input.

sig_handler - function that change the default SIGCHLD signal.


------------files---------------------------------------------------

there is a file called "file.txt" that save the commands we used.
only the valid commands and the "history" command will be entered into the file.

the second file is nohup.txt that save the outpot of the command
after we shup down the program.
               

------------INPUT--------------------------------------------------

the input is a sentence that can be as long as 512 chars.

------------OUTPUT-------------------------------------------------

if the input is valid linux command - the output is the execution of that command.
if the input is not a valid command - the output is  an ERR.
if the input is "history" - the output will be all the previous commands.
if the input is "!<number> - the output is the execution of the command that is <number> in the history file.
if the input is "done" - the output is how many commands we had and how many words in these commands.

after the input "done" - the program end.

