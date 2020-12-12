#ifndef INTERPRETER_HPP

#include <variant>
#include <string>

namespace tui
{
struct commands_file
{
    std::string path;
};
struct commands_prompt
{
};
using commands_src = std::variant<commands_file, commands_prompt>;

class Interpreter
{

};
}  // namespace tui

#endif  // INTERPRETER_HPP