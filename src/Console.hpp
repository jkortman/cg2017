// Console class
// A class to manage user input via the terminal.

#ifndef CONSOLE_H
#define CONSOLE_H

enum VarType {Int, Float, Bool};
struct Var {std::string name; VarType type; void* pointer; int size; std::string what; bool writable;};
struct Input {std::string command; std::vector<std::string> values;};

class Console
{
public:
	void initialize();
	void parse();
    void get_var(Var container);
    void set_var(Var container, std::vector<std::string> values);
    void register_var(std::string name, VarType type, void* ptr, int size=1, std::string what="", bool writeable=true);

private:
	std::unordered_map<std::string, Var> variables;
};

#endif // CONSOLE_H