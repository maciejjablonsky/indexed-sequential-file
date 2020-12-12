#ifndef COMMANDS_PROMPT_HPP
#define COMMANDS_PROMPT_HPP

#include <string>

namespace commands
{
class Prompt
{
   public:
    explicit Prompt() = default;
    [[nodiscard]] std::string GetNextCommand();
};
}  // namespace commands

#endif  // COMMANDS_PROMPT_HPP