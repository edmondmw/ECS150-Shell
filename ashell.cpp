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

using namespace std;

//Key types
static const int regular = 0;
static const int up = 1;
static const int down = 2;
static const int enter = 3;
static const int del = 4;
static const int back = 5;

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
int determineKey(char c)
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

//Delete the previous character 
void backspace(string &current)
{
    if(!current.empty())
    {
        current.pop_back();
        write(STDOUT_FILENO, "\b \b", 3);     
    }
}

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

void executeCommand(const string command, const list<string> commandList)
{    
    //Used to store the string tokens after being parsed
    vector<string> tokens;
    stringstream iss(command);
    string aToken;

    //parse the string
    while(iss >> aToken)
    {
        tokens.push_back(aToken);
    }

    //if the command was all whitespace, we exit the function
    if(tokens.empty())
    {
        return;
    }

    if(tokens[0] == "history")
    {
        showHistory(commandList);
    }
}

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
    
    while(true)
    {
        //read the user input
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

