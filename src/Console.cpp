// Implementation of Console class member functions.

#include <iostream>
#include <string>
#include <vector>

#include "Console.hpp"

int Console::parse()
{
    // Load input
    std::string input;
    getline(std::cin, input);

    if ( input.size() == 0) return -1; // No input

    // Strip leading/trailing spaces
    int start = input.find_first_not_of(" ");
    int end = input.find_last_not_of(" ");
    if (start == -1) start = 0; // No leading spaces
    if (end == -1) end = 0;     // No trailing spaces
    input = input.substr(start,end-start+1);

    if ( input.size() == 1) return -1; // Only spaces
    
    // Tokenise input
    std::vector<std::string> InputStorage = tokenize(input, " ");

    // Handle input

    
    return 0;
}


std::vector<std::string> Console::tokenize(
    std::string str,
    const std::string delims)
{
    std::vector<std::string> tokens;
    int pos = str.find(delims);

    while (pos != -1)
    {
        tokens.push_back(str.substr(0, pos));
        str.erase(0,pos+1);
        pos = str.find(delims);
    }
    tokens.push_back(str);
    return tokens;
}