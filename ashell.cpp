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



int main()
{
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

    int direction;

    struct termios savedTermAttributes;
    
    setNonCanonicalMode(STDIN_FILENO, &savedTermAttributes);
    
    while(true)
    {
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
                write(STDOUT_FILENO, "\b \b", 5);     
            }
        }

        //if character is up arrow
        else if(direction == 1)
        {
            //gotta clear screen and show the previous thing on the stack
            //if the iterator is not at the beginning  and not empty then show the previous command
            if(it != historyList.begin() && !historyList.empty() )
            {
                //If we press up when we are at the currentCommand(haven't pressed up before), then we need to store the original command
                if(it == historyList.end())
                {
                    originalCommand = currentCommand;
                }

                it--;
                //print backspaces to delete the currentCommand
                for(int i = 0; i < currentCommand.length(); i++)
                {
                    write(STDOUT_FILENO, "\b \b", 5);
                }
                                
                write(STDOUT_FILENO, it->c_str(), it->length());
                currentCommand = *it;
            }
        }    

        //if character is down arrow
        else if(direction == 2)
        {
            if(it != historyList.end() && !historyList.empty())
            {    
                it++;
                for(int i = 0; i < currentCommand.length(); i++)
                {
                    write(STDOUT_FILENO, "\b \b", 5);
                }

                //if iterator is at the end then we write out the original command
                if(it == historyList.end())
                {
                    currentCommand = originalCommand;
                }

                else
                {
                    currentCommand = *it;
                }

                write(STDOUT_FILENO, currentCommand.c_str(), currentCommand.length());
            }
        }

        //if character is enter
        else if(character == 0x0A)
        {
            //place the current command into the history list
            historyList.push_back(currentCommand);
 
            //enter has been typed so currentCommand needs to be reset
            currentCommand.clear();

            it = historyList.end();
        }
        //regular input, then just add the character to the command string and right it out
        else
        {
            //append the character into currentCommand
            currentCommand+=character;
            write(STDOUT_FILENO, &character, 1);
        }
    }//while loop

	return 0;
}

