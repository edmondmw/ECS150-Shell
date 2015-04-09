#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <termios.h>
#include <ctype.h>
#include <list>

using namespace std;

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

//Returns the direction of the arrow key pressed. If UP = 1, DOWN = 2, NONE = 0
int determineDirection(char c)
{
    if(c == 0x1B)
    {
        read(STDIN_FILENO, &c, 1);
        if(c == 0x5B)
        {
            read(STDIN_FILENO, &c, 1);
            if(c == 0x41)
            {
                return 1;
            }
            else if(c == 0x42)
            {
                return 2;
            }
        }
    }
    return 0;
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
    //arrow key direction
    int direction;
	char *path;
	path=get_current_dir_name();

    struct termios savedTermAttributes;
    
    setNonCanonicalMode(STDIN_FILENO, &savedTermAttributes);
    
    while(true)
    {
		if(0/*path.size<=16*/)
			write(STDOUT_FILENO,path,16);
		else
		{
			write(STDOUT_FILENO,"/...",4);
			write(STDOUT_FILENO,path,11);	//strlen(path)-10
			write(STDOUT_FILENO,">",1);
			write(STDOUT_FILENO,"\n",1);
		}
		
//		cout<< path<< endl;
        //read the user input
        read(STDIN_FILENO, &character, 1);
        direction = determineDirection(character);

        //if backspace then set character to \b \b so that it doesn't display
        //Also delete the last element in currentCommand
        if(character == 0x7F)
        {
            if(!currentCommand.empty())
            {
                currentCommand.pop_back();
                write(STDOUT_FILENO, "\b \b", 3);     
            }
        }

        //if character is up arrow then show the previous command 
        else if(direction == 1)
        {           
            upCommand(historyList,it,currentCommand,originalCommand);
        }    

        //if character is down arrow, display the next command in the historyList
        else if(direction == 2)
        {
            downCommand(historyList,it,currentCommand,originalCommand);
        }

        //if character is enter, add the currentCommand to the historyList, clear currentCommand, and reset the iterator
        else if(character == 0x0A)
        {
            //remove the oldest command if we already have 10 commands stored
            if(historyList.size() >= 10)
            {
                historyList.pop_front();
            }
            historyList.push_back(currentCommand);
            currentCommand.clear();
            it = historyList.end();
            write(STDOUT_FILENO, "\r\n", 2);
        }

        //regular input, then just add the character to the command string and write it out
        else
        {
            currentCommand+=character;
            write(STDOUT_FILENO, &character, 1);
        }

    }//while loop

	return 0;
}

