// Console class
// A class to manage user input via the terminal.

#ifndef CONSOLE_H
#define CONSOLE_H

enum VarType {Invalid, Function, Int, Float, Bool};

class Console
{
public:
	void initialize();
	int parse();
    // get(std::string var);
    // set(std::string var, std::string value);
    void register_var(VarType type, std::string var, void* ptr, bool writeable=true, std::string what="");
};

#endif // CONSOLE_H