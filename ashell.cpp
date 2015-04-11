#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <termios.h>
#include <ctype.h>
#include <list>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

using namespace std;

//Key types
enum keys
{
    regular = 0,
    up = 1,
    down = 2,
    enter = 3,
    del = 4,
    back = 5,
};

//commands
enum commands
{
    eCd = 0,
    eLs,
    ePwd,
    eHistory,
    eExit,
    eOther
};

//sets to noncanonical mode, meaning input can be processed before pressing enter
void setNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    char *name;
    
    // Make sure stdin is a terminal. 
    if(!isatty(fd)){
        fprintf (stderr, "Not a terminal.\n");
        exit(0);
    }
    
    // Save the terminal attributes so we can restore them later. 
    tcgetattr(fd, savedattributes);
    
    // Set the funny terminal modes. 
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO. 
    TermAttributes.c_cc[VMIN] = 1;
    TermAttributes.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}

//Determine what key the user pressed
keys determineKey(char c)
{    
    //up or down arrow or delete
    if(c == 0x1B)
    {
        read(STDIN_FILENO, &c, 1);
        if(c == 0x5B)
        {
            read(STDIN_FILENO, &c, 1);
            if(c == 0x41)
            {
                return up;
            }
            else if(c == 0x42)
            {
                return down;
            }
            else if(c == 0x33)
            {
                read(STDIN_FILENO, &c, 1);
                if(c==0x7E)
                {
                    return del;
                }               
            }   

        }
    }

    //backspace
    else if(c == 0x7F)
    {
        return back;
    }

    //enter
    else if(c == 0x0A)
    {
        return enter;
    }
    //regular input
    else
    {
        return regular;
    }
}

//determine what command was entered
commands determineCommand(string command)
{
    if(command == "cd")
    {
        return eCd;
    }
    else if(command == "ls")
    {
        return eLs;
    }
    else if(command == "pwd")
    {
        return ePwd;
    }
    else if(command == "history")
    {
        return eHistory;
    }
    else if(command == "exit")
    {
        return eExit;
    }
    else
    {
        return eOther;
    }
}

//Displays the previous command in the historyList if up arrow is pressed
void upCommand(const list<string> &commandList, list<string>::const_iterator &it, string &command, string &original)
{
    //only show the the previous command if we aren't at the beginning and the list isn't empty
    if(it != commandList.begin() && !commandList.empty() )
    {
        //If we press up when we are at the currentCommand(haven't pressed up before), then we need to store the original command
        if(it == commandList.end())
        {    
            original = command;
        }

        it--;
        //print backspaces to delete the currentCommand
        for(int i = 0; i < command.length(); i++)
        {
            write(STDOUT_FILENO, "\b \b", 3);
        }
                                
        write(STDOUT_FILENO, it->c_str(), it->length());
        command = *it;
    }
	else
		write(STDOUT_FILENO, "\a",1);
}

//When you press the down arrow key, it displays the next command in the history list
void downCommand(const list<string> &commandList, list<string>::const_iterator &it, string &command, string &original)
{
    //Only show the next command if we aren't at the end and the list isn't empty
    if(it != commandList.end() && !commandList.empty() )
    {
        it++;
        for(int i = 0; i < command.length(); i++)
        {
            write(STDOUT_FILENO, "\b \b", 3);
        }

        //if we are at the end after incrementing, we return to the original command
        if(it == commandList.end())
        {
            command = original;
        }
        else
        {
            command = *it;
        }                      
        write(STDOUT_FILENO, command.c_str(), command.length());
    }
	else
		write(STDOUT_FILENO,"\a",1);
}

//Shows the current directory, placed at beginning of each line
void beginning()
{
	string path;
	path=get_current_dir_name();
	int pathSize=path.size();
	//checks if path's length is less then 15 characters
	if(path.size()<=15)
	{
		write(STDOUT_FILENO, path.c_str(),path.length());
		write(STDOUT_FILENO, "> ",2);
	}
	// if path is more then 15 characters go here
	else
	{
		//checks back of path and iterates downward till it finds first "/"
		for(pathSize-1; pathSize!=0; pathSize--)
		{
			if(path[pathSize]=='/')
			{
				//stores a substring of path after it sees first "/"
				string temp=path.substr(pathSize);
				write(STDOUT_FILENO, "/...",4);
				write(STDOUT_FILENO,temp.c_str(), temp.length());
				write(STDOUT_FILENO, "> ",2);
				break;
			}
		}
	}
}
		
//Delete the previous character 
void backspace(string &current)
{
    if(!current.empty())
    {
        current.pop_back();
        write(STDOUT_FILENO, "\b \b", 3);     
    }
	else
		write(STDOUT_FILENO, "\a",1);
}


//Shows the previous 10 commands
void showHistory(const list<string> commandList)
{
    list<string>::const_iterator it;
    int count = 0;
    string charCount;

    for(it = commandList.begin(); it!=commandList.end(); it++)
    {
        charCount = char(count + '0');
        write(STDOUT_FILENO, charCount.c_str(), charCount.size());
        write(STDOUT_FILENO, " ", 1);
        write(STDOUT_FILENO, it->c_str(), it->size());
        write(STDOUT_FILENO, "\r\n", 2);
        count++;
    }
}

//Prints the working directory
void printWorkingDirectory()
{
    string workingDirectory = get_current_dir_name();
    write(STDOUT_FILENO, workingDirectory.c_str(), workingDirectory.size());
    write(STDOUT_FILENO, "\r\n",2);
}

//changes the current directory
void changeDirectory(const vector<string> tokens)
{
    string path;
    int isError = -1;

    //if user only typed "cd" go home
    if(tokens.size() == 1)
    {
        path = getenv("HOME");
    }
    //otherwise go to the specified directory
    else
    {
        path = tokens[1];
    }

    isError = chdir(path.c_str());

    //directory change successful
    if(isError == 0)
    {
        return;
    }
    //unsuccessful directory change
    else
    {
        switch(errno)
        {
            case EACCES:
                write(STDOUT_FILENO, "Permission denied!", 18);
                write(STDOUT_FILENO, "\r\n", 2);
                break;
            case ENOTDIR:
                write(STDOUT_FILENO, tokens[1].c_str(), tokens[1].size());
                write(STDOUT_FILENO, " not a directory!", 16);
                write(STDOUT_FILENO, "\r\n", 2);
                break;
            default:
                write(STDOUT_FILENO, "Error changing directory", 24);
                write(STDOUT_FILENO, "\r\n", 2);
                break;    
        }
    }
}

//function to execute a command after enter is pressed
void executeCommand(const string command, const list<string> commandList)
{    
    //Used to store the string tokens after being parsed
    vector<string> tokens;
    stringstream iss(command);
    string aToken;

    //parse the string so that you get rid of whitespaces
    while(iss >> aToken)
    {
        tokens.push_back(aToken);
    }

    //if the command was all whitespace, we exit the function
    if(tokens.empty())
    {
        return;
    }
	for(int i=0; i<tokens.size();i++)
		cout<<tokens[i]<<endl;
    //switch used to determine what to do depending on the command
    switch(determineCommand(tokens[0]))
    {
        case eCd:
            changeDirectory(tokens);
            break;
        case eLs:
            //write a cd function
            break;
        case ePwd:
            printWorkingDirectory();
            break;
        case eHistory:
            showHistory(commandList);
            break;
        case eExit:
            exit(0);
            break;
        default:
            break;
    }
}

/*void forking()
{
	char *arg;	//last thing in args has to be NULL
	pid_t pid = fork();

	if(pid<0)
	{
		write(STDOUT_FILENO, "fork failed", 11);
		exit(1);
	}
	//child is born and now exec's
	else if(pid ==0)
	{	//reads in commands not created (cat)
		read(STDIN_FILENO,&arg,1);
		execvp("/bin/",arg);	//how to make it so execvp goes to bin and then user input
		exit(1);
	}
	//parent
	else if(pid>0
	{
		//do parent stuff and wait for child to finish 
		wait(NULL);
	}
}*/
//When enter key is pressed place the currentCommand into the linked list and new line
void enterCommand(list<string> &commandList, list<string>::const_iterator &it, string &current, string &original )
{    
    write(STDOUT_FILENO, "\r\n", 2);
    //if the list already has 10 values, we remove the least recent command
    if(commandList.size() >= 10)
    {
        commandList.pop_front();
    }
    //place the current comand into the list
    commandList.push_back(current);

    executeCommand(current, commandList);
    current.clear();
    original.clear();
    //reset the iterator to the end of the list
    it = commandList.end();
    beginning();
}

int main()
{
    //the user input
    char character;
    //the currentCommand being displayed
    string currentCommand;
    //the command that was originally typed in. 
    //Want to save this for when we use up/down arrows so we can return to the original command
    string originalCommand;
    //linked list of strigs that keeps track of previous commands
    list<string> historyList;
    //iterator to iterate through the historyList
    list<string>::const_iterator it;
    //set the iterator to the end of the list
    it = historyList.end();
    //what kind of key was pressed
    int key;

    struct termios savedTermAttributes;
    
    setNonCanonicalMode(STDIN_FILENO, &savedTermAttributes);
	beginning();    
   
	 while(true)
    {	
		
        read(STDIN_FILENO, &character, 1);
        //determine which key the user pressed
        key = determineKey(character);
        
		//switch statement to determine what to do based on input
        switch(key)
        {
            case up:
                upCommand(historyList, it, currentCommand, originalCommand);
                break;
            case down:
                downCommand(historyList, it, currentCommand, originalCommand);
                break;
            case enter:
                enterCommand(historyList, it, currentCommand, originalCommand);
                break;
            case back:
                backspace(currentCommand);
                break;
            case del:
                backspace(currentCommand);
                break;
            default:
                currentCommand+=character;
                write(STDOUT_FILENO, &character, 1);
                break;
        }
    }//while loop

	return 0;
}

