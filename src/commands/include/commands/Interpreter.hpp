#ifndef COMMANDS_INTERPRETER_HPP
#define COMMANDS_INTERPRETER_HPP

#include <commands/source.hpp>

namespace commands
{
class Interpreter
{
   public:
    Interpreter(commands::source& source);

   private:
    commands::source* source_;
};
}  // namespace commands

#endif  // COMMANDS_INTERPRETER_HPP