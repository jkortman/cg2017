// Implementation of Console class member functions.

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "Console.hpp"



//VarType := enum class {Function, Int, Float, Bool};
//Var := struct {VarType type, void* pointer, bool writable, std::string what};



Input tokenize(std::string str, const std::string delims)
{
    Input tokens;
    int pos = str.find(delims);
    if ( pos == -1)
    {
        tokens.command = str;
    } else
    {
        tokens.command = str.substr(0, pos);
        str.erase(0,pos+1);
        pos = str.find(delims);
        while (pos != -1)
        {
            tokens.values.push_back(str.substr(0, pos));
            str.erase(0,pos+1);
            pos = str.find(delims);
        }
        tokens.values.push_back(str);
    }
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
        std::string input_string;
        getline(std::cin, input_string);

        if ( input_string.size() == 0) return -1; // No input

        // Strip leading/trailing spaces
        int start = input_string.find_first_not_of(" ");
        int end = input_string.find_last_not_of(" ");
        if (start == -1) start = 0; // No leading spaces
        if (end == -1) end = 0;     // No trailing spaces
        input_string = input_string.substr(start,end-start+1);

        if ( input_string.size() == 1) return -1; // Only spaces

        // Tokenise input
        Input input = tokenize(input_string, " "); // Input = {string command, vector<string> values }

        // Handle input
        Var container;
        //std::string cmd = InputStorage.at(0);
        std::string name;
        //Special commands
        if (input.command == "continue")
        {
            isRunning = false;
            std::cout << "Returning to game" << std::endl;
        } else if (input.command == "quit")
        {
            isRunning = false;
            std::cout << "Shutting down program" << std::endl;
        } else if (input.command == "help")
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
        } else if (input.command == "whatis")
        {
            name = input.values.at(0);
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
            
        } else if (input.command == "noclip")
        {
            // Set noclip flag
        } 

        // Variable Set/Get commands
        container = variables[input.command];
        if (container.type != Invalid)
        {
            if (input.values.size() == 0)
            {
                get_var(container);
                //std::cout << cmd << " = " << "value" << std::endl;    
            } else if (input.values.size() > 0)
            {
                //std::cout << cmd << " has been set to " << InputStorage.at(1) << std::endl;
                set_var(container, input.values);
            }
        } else
        {
            // Error
        }



             
        //        
        
    }
    
    
    return 0;
}



void Console::register_var(std::string name, VarType type, void* ptr, std::string what, bool writable)
{
    Var h = {name, type, ptr, what, writable};
    variables[name] = h;
};



void Console::initialize()
{
    //register_var(Function,"help",nullptr,false,"Lists possible commands");
    register_var("mode",Int,nullptr,"Reports the current mode.", true);
    //register_var(Float,"fov",nullptr, true,"Sets the current Field of View of camera.");

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


void Console::get_var(Var container)
{
    std::cout << container.name << " = ";// << (container.pointer) <<std::endl;
    if (container.type == Float)
    {
        float* ptr = (float*)container.pointer;
        std::cout << *ptr << std::endl;
    }
    switch(container.type)
    {
        case VarType::Float:
        {
            float* ptr = (float*)container.pointer;
            std::cout << *ptr << std::endl;
        }
        break;
        case VarType::Int:
        {
            int* ptr = (int*)container.pointer;
            std::cout << *ptr << std::endl;
            
        }
        break;
        case VarType::Bool:
        {
            bool* ptr = (bool*)container.pointer;
            
            //*ptr = stob(values.at(0));
            std::cout << *ptr << std::endl;
            
        }
        break;
        case VarType::Invalid:
        default:
            //error
        break;

    }
}

void Console::set_var(Var container, std::vector<std::string> values)
{
    if (container.writable)
    {
        std::cout << "Set " << container.name << " to ";
        switch(container.type)
        {
            case VarType::Float:
            {
                float* ptr = (float*)container.pointer;
                *ptr = std::stof(values.at(0));
                std::cout << *ptr << std::endl;
            }
            break;
            case VarType::Int:
            {
                int* ptr = (int*)container.pointer;
                *ptr = std::stoi(values.at(0));
                std::cout << values.at(0) << std::endl;
            }
            break;
            case VarType::Bool:
            {
                bool* ptr = (bool*)container.pointer;
                if ((values.at(0) == "0") || (values.at(0) == "false")) *ptr = false;
                else if ((values.at(0) == "1") || (values.at(0) == "true")) *ptr = true;
                std::cout << *ptr << std::endl;        
            }
            break;
            case VarType::Invalid:
            default:
                //error
            break;
        }        
    }
    else
    {
        std::cout << "Error: " << container.name << " is not writable. Value not set." << std::endl;
    }
    
}