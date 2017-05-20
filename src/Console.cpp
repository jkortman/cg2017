// Implementation of Console class member functions.

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "Console.hpp"



//VarType := enum class {Function, Int, Float, Bool};
//Var := struct {VarType type, void* pointer, bool writable, std::string what};
struct Var {VarType type; void* pointer; bool writable; std::string what;};
std::unordered_map<std::string, Var> variables;

std::vector<std::string> tokenize(std::string str, const std::string delims)
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

int Console::parse()
{
    std::cout << "Entering Console" << std::endl;
    bool isRunning = true;
    while (isRunning)
    {
        std::cout << ">>";
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
        Var container;
        std::string cmd = InputStorage.at(0);
        std::string name, value;
        //Special commands
        if (cmd == "continue")
        {
            isRunning = false;
            std::cout << "Returning to game" << std::endl;
        } else if (cmd == "quit")
        {
            isRunning = false;
            std::cout << "Shutting down program" << std::endl;
        } else if (cmd == "help")
        {
            std::cout << "Availible commands:" << std::endl;
            std::cout << "help - prints out this." << std::endl;
            std::cout << "continue - unpauses game and ends terminal command session." << std::endl;
            std::cout << "quit - ends terminal command session." << std::endl;
            std::cout << "whatis <var> - describes var (if registered)." << std::endl;
            std::cout << "noclip - toggles collisions and separates camera from player." << std::endl;
            std::cout << "<var> - returns value of variable (if registered)." << std::endl;
            std::cout << "<var> <value> - sets value of variable (if registered and writable)." << std::endl << std::endl;
            // Print Help
        } else if (cmd == "whatis")
        {
            name = InputStorage.at(1);
            container = variables[name];
            if (container.type != Invalid)
            {
                std::cout << "Usage: " << name;
                if (container.writable) std::cout << " <" << container.type << ">";
                //std::cout << "variables[\"" << InputStorage.at(0) <<"\"] = {VarType = " <<  container.type << "; ptr = " << container.pointer <<"; writable = " << container.writable << "; what = "<< container.what <<"}" << std::endl;
                std::cout << " - " << container.what << std::endl;
            } else
            {
                std::cout << name << " is not a registered variable." << std::endl;
            }
            
        } else if (cmd == "noclip")
        {
            // Set noclip flag
        } 

        // Variable Set/Get commands
        container = variables[cmd];
        if (container.type != Invalid)
        {
            if (InputStorage.size() == 1)
            {
                // get_var()
                std::cout << cmd << " = " << "value" << std::endl;    
            } else if (InputStorage.size() > 1)
            {
                std::cout << cmd << " has been set to " << InputStorage.at(1) << std::endl;
                // set_var()
            }
        } else
        {
            // Error
        }



             
        //        
        
    }
    
    
    return 0;
}



void Console::register_var(VarType type, std::string var, void* ptr, bool writable, std::string what)
{
    Var h = {type, ptr, writable, what};
    variables[var] = h;
};



void Console::initialize()
{
    //register_var(Function,"help",nullptr,false,"Lists possible commands");
    register_var(Int,"mode",nullptr,true,"Reports the current mode.");
    register_var(Float,"fov",nullptr, true,"Sets the current Field of View of camera.");

}


// set(std::string varname, string value) {
//     switch (var.type)
//     {
//         case Type::Int:
//         {
//             // validate value is int
//             int i = to_int(value);
//             int* ptr = var.pointer;
//             *ptr = int;
//         }
//         break;
//         case Type::Float:
//         {
//             //etc
//         }
//         break;
//     }
// }


