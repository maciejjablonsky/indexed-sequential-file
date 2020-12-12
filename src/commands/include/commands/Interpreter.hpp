#ifndef COMMANDS_INTERPRETER_HPP
#define COMMANDS_INTERPRETER_HPP

#include <commands/source.hpp>
#include <commands/commands.hpp>
#include <commands/FileReader.hpp>
#include <commands/Prompt.hpp>
#include <queue>

namespace commands
{
class Interpreter
{

   public:
    Interpreter(const commands::source& source);
    [[nodiscard]] bool EndOfCommands() const;
    void LoadCommands();
    [[nodiscard]] possible_command PopCommand();
    [[nodiscard]] int CountBufferedCommands() const;

   private:
    [[nodiscard]] possible_command UnzipCommandLine(
        const std::string& command_line) const;
    std::queue<possible_command> commands_queue_;
    using commands_source = std::variant<Prompt, FileReader>;
    commands_source source_;
};
}  // namespace commands

#endif  // COMMANDS_INTERPRETER_HPP