#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <termios.h>
#include <ctype.h>

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

//determine if the input, c, is a up arrow
bool isUp(char c)
{
    if(c == 0x1B)
    {
        read(STDIN_FILENO, &c, 1);
        if(c == 0x5B)
        {
            read(STDIN_FILENO, &c, 1);
            if(c == 0x41)
            {
                return true;
            }
        }
    }
    return false;
}

//Determines if the input,c, is a down arrow
bool isDown(char c)
{
    if(c == 0x1B)
    {
        read(STDIN_FILENO, &c, 1);
        if(c == 0x5B)
        {
            read(STDIN_FILENO, &c, 1);
            if(c == 0x42)
            {
                return true;
            }
        }
    }

    return false;
}

int main()
{
    //string currentCommand;
    char character;
    string currentCommand;
    struct termios savedTermAttributes;

    setNonCanonicalMode(STDIN_FILENO, &savedTermAttributes);
    
    while(1)
    {
        //read the user input
        read(STDIN_FILENO, &character, 1);

        //if backspace then set character to \b \b so that it doesn't display
        //Also delete the last element in currentCommand
        if(character == 0x7F)
        {
            currentCommand.pop_back();
            write(STDOUT_FILENO, "\b \b", 5);
        }

        //if character is up arrow
        else if(isUp(character))
        {
            //gotta clear screen and show the previous thing on the stack
        }    

        //if character is down arrow
        else if(isDown(character))
        {
            //do down arrow stuff
        }

        //if character is enter
        else if(character == 0x0A)
        {
            //do enter stuff
        }
        //regular input, then just add the character to the command string and right it out
        else
        {
            //append the character into currentCommand
            currentCommand+=character;
            write(STDOUT_FILENO, &character, 1);
        }
    }

	return 0;
}

