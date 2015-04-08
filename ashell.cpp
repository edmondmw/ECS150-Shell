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

        //regular input, then just add the character to the command string and right it out
        else
        {
            //append the character into currentCommand
            currentCommand+=character;
            write(STDOUT_FILENO, &character, 1);
        }
         cout<<"\nSTRING: "<<currentCommand<<endl;            
    }
	return 0;
}

