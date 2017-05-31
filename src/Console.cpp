// Implementation of Console class member functions.

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "Console.hpp"

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

void Console::parse()
{
    std::string output = "";
    std::cout << "\nEntering Console\n";
    bool isRunning = true;
    while (isRunning)
    {
        std::cout << ">>";

        // Load input
        std::string input_string;
        getline(std::cin, input_string);


        if ( input_string.size() == 0) continue; // No input

        // Strip leading/trailing spaces
        int start = input_string.find_first_not_of(" ");
        int end = input_string.find_last_not_of(" ");
        if (start == -1) start = 0; // No leading spaces
        if (end == -1) end = 0;     // No trailing spaces
        input_string = input_string.substr(start,end-start+1);

        if ( input_string.size() == 1) continue; // Only spaces

        // Tokenise input
        Input input = tokenize(input_string, " "); // Input = {string command, vector<string> values }

        // Handle input
        Var container;
        std::string name;

        //Special commands
        if (input.command == "go")
        {
            isRunning = false;
            std::cout << "Returning to game" << std::endl;
            continue;
        } else if (input.command == "quit")
        {
            isRunning = false;
            std::cout << "Shutting down program" << std::endl;
            // Do something to cause window to shutdown?
            continue;
        } else if (input.command == "help")
        {
            output = "Availible commands:\n";
            output += "help - prints out this.\n";
            output += "continue - unpauses game and ends terminal command session.\n";
            output += "quit - ends terminal command session.\n";
            output += "whatis <var> - describes var (if registered).\n";
            output += "noclip - toggles collisions and separates camera from player.\n";
            output += "<var> - returns value of variable (if registered).\n";
            output += "<var> <value> - sets value of variable (if registered and writable).\n";
            std::cout << output << "\n";
            continue;
        } else if (input.command == "whatis" )
        {
            name = input.values.at(0);
            
            if (variables.find(name) != variables.end())
            {
                container = variables[name];
                output =  "Usage: " + name;
                if (container.writable) 
                {
                    output.append(" <");
                    switch(container.type)
                    {
                        case VarType::Float:
                        {
                            output.append("float");
                        }
                        break;
                        case VarType::Int:
                        {
                            output.append("int");
                        }
                        break;
                        case VarType::Bool:
                        {
                            output.append("bool");
                        }
                        break;
                    }
                    output.append(">");
                }
                output += " - " + container.what;
            } else
            {
                output = name + " is not a registered variable.";
            }
            std::cout << output << "\n";
        } else if (input.command == "noclip" || input.command == "tcl")
        {
            output = "collisions turned ";
            container = variables["noclip"];
            bool* ptr = (bool*)container.pointer;
            if (*ptr == false)
            {
                *ptr = true;
                output.append("off.");
            }
            else
            {
                *ptr = false;
                output.append("on.");
            }
            std::cout << output << "\n";
        } else if (input.command == "demo")
        {
            output = "Demo turned ";
            container = variables["demo"];
            bool* ptr = (bool*)container.pointer;
            if (*ptr == false)
            {
                *ptr = true;
                output.append("on.");
            }
            else
            {
                *ptr = false;
                output.append("off.");
            }
            std::cout << output << "\n";
        } else if (variables.find(input.command) != variables.end())
        {   
            container = variables[input.command];
            if (input.values.size() == 0)
            {
                get_var(container);
            } else if (input.values.size() > 0)
            {
                set_var(container, input.values);
            }
        } else
        {
            std::cout << "Command not recognized." << std::endl;
            // Error
        }
        //              
    }

}

void Console::register_var(std::string name, VarType type, void* ptr, int size, std::string what, bool writable)
{
    Var h = {name, type, ptr, size, what, writable};
    variables[name] = h;
};



void Console::initialize()
{
    //Register necessary console commands here.
    console = this;
}


void Console::get_var(Var container)
{
    std::string output = container.name + " = ";
    switch(container.type)
    {
        case VarType::Float:
        {
            float* ptr = (float*)container.pointer;
            if (container.size == 1)
            {
                output.append(std::to_string(*ptr));
            } else if (container.size > 1)
            {
                output += "(";
                for (int i = 0; i < container.size; i++)
                {
                    output.append(std::to_string(ptr[i]));
                    if (i != container.size -1) output.append(", ");
                }
                output += ")";
            }  
        }
        break;
        case VarType::Int:
        {
            int* ptr = (int*)container.pointer;
            output.append(std::to_string(*ptr));
        }
        break;
        case VarType::Bool:
        {
            bool* ptr = (bool*)container.pointer;
            if (*ptr == true) output.append("true");
            else if (*ptr == false) output.append("false");
        }
        break;
        default:
            //error
        break;
    }
    std::cout << output << "\n";
}

int to_float(float* ptr, std::string val)
{
    if (val == "*") return 0;
    else
    {
        try {*ptr = std::stof(val);}
        catch (std::invalid_argument ia) {return -1;} 
        catch (std::out_of_range oor) {return -2;} 
    }
    return 0;
}

void Console::set_var(Var container, std::vector<std::string> values)
{
    std::string output;
    if (container.writable)
    {
        output = "Set " + container.name + " to ";
        switch(container.type)
        {
            case VarType::Float:
            {
                float* ptr = (float*)container.pointer;
                int err;
                if (container.size == 1)
                {
                    err = to_float(ptr,values.at(0));
                    switch(err)
                    {
                        case 0:
                        {
                            output.append(std::to_string(*ptr));
                        }
                        break;
                        case -1:
                        {
                            output = values.at(0) + " not recognized as a float.";
                        }
                        break;
                        case -2:
                        {
                           output = values.at(0) + " is out of range.";
                        }
                        break; 
                    }
                } else if (container.size > 1)
                {
                    std::vector<float> converted_vals;

                    output.append("(");

                    if (values.size() != container.size)
                    {
                        output = "Expected " + std::to_string(container.size);
                        output.append("input(s). Got ");
                        output += std::to_string(values.size()) + ".";
                    } else
                    {
                        for (int i = 0; i < values.size(); i++)
                        {
                            err = to_float(&(ptr[i]),values.at(i));
                            switch(err)
                            {
                                case 0:
                                {
                                    output.append(std::to_string(ptr[i]));
                                    if (i != container.size -1) output.append(", ");
                                }
                                break;
                                case -1:
                                {
                                    output = values.at(i) + " not recognized as a float.";
                                }
                                break;
                                case -2:
                                {
                                   output = values.at(i) + " is out of range.";
                                }
                                break; 
                            }
                            if (err < 0) break;
                        }
                        if ( err == 0) output += ")";
                    }
                }
            }
            break;
            case VarType::Int:
            {
                int* ptr = (int*)container.pointer;
                try
                {
                    *ptr = std::stoi(values.at(0));
                }
                catch (std::invalid_argument ia)
                {
                    output = values.at(0) + " not recognized as a int.";
                    break; 
                }
                catch (std::out_of_range oor) 
                {
                    output = values.at(0) + " is out of range.";
                    break;
                }
                output.append(std::to_string(*ptr));
            }
            break;
            case VarType::Bool:
            {
                bool* ptr = (bool*)container.pointer;
                if ((values.at(0) == "0") || (values.at(0) == "false"))
                {
                    *ptr = false;
                    output.append("false");
                }
                else if ((values.at(0) == "1") || (values.at(0) == "true"))
                {
                    *ptr = true;
                    output.append("true");
                }
            }
            break;
            default:
                //error
            break;
        }        
    }
    else
    {
       output = "Error: " + container.name + " is not writable. Value not set.";
    }
    std::cout << output << "\n";
    
}