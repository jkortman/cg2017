// Console class
// A class to manage user input via the terminal.

#ifndef CONSOLE_H
#define CONSOLE_H

enum VarType {Invalid, Int, Float, Bool};
struct Var {std::string name; VarType type; void* pointer; std::string what; bool writable;};
struct Input {std::string command; std::vector<std::string> values;};

class Console
{
public:
	void initialize();
	int parse();
    void get_var(Var container);
    void set_var(Var container, std::vector<std::string> values);
    void register_var(std::string name, VarType type, void* ptr, std::string what="", bool writeable=true);

private:
	std::unordered_map<std::string, Var> variables;
};

#endif // CONSOLE_H