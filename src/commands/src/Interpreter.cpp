#include <commands/Interpreter.hpp>
#include <string_view>
#include <sstream>
#include <type_traits>
template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

commands::Interpreter::Interpreter(const commands::source& source)
    : source_(std::visit(overloaded{
                             [](const from_file& s) -> decltype(source_) {
                                 return commands::FileReader(s.path);
                             },
                             [](const from_prompt& s) -> decltype(source_) {
                                 return commands::Prompt();
                             },
                         },
                         source))
{
}

bool commands::Interpreter::EndOfCommands() const
{
    return std::visit(overloaded{[](const FileReader& file_reader) {
                                     return file_reader.EndOfFile();
                                 },
                                 [](...) { return false; }},
                      source_) ||
           std::visit(overloaded{[](const command_exit&) { return true; },
                                 [](...) { return false; }},
                      commands_queue_.front());
}

void commands::Interpreter::LoadCommands()
{
    std::string command_line = std::visit(
        overloaded{
            [](commands::FileReader& file_reader) {
                return file_reader.ReadNextCommand();
            },
            [](commands::Prompt& prompt) { return prompt.GetNextCommand(); }},
        source_);
    commands_queue_.push(UnzipCommandLine(command_line));
    std::visit(overloaded{[&](const commands::Prompt&) {
                              if (!std::holds_alternative<command_show>(
                                      commands_queue_.back()))
                              {
                                  commands_queue_.push(command_show{});
                              }
                          },
                          [](const auto&) { /* any other case */ }},
               source_);
}
commands::possible_command commands::Interpreter::PopCommand()
{
    auto tmp = commands_queue_.front();
    commands_queue_.pop();
    return tmp;
}
commands::possible_command commands::Interpreter::UnzipCommandLine(
    const std::string& command_line) const
{
    std::stringstream ss(command_line);
    std::string command_token = "";
    ss >> command_token;
    auto command = commands::match_command_str(command_token);
    std::visit(overloaded{[&](command_read& c) { ss >> c.key; },
                          [&](command_insert& c) { ss >> c.key >> c.record; },
                          [](auto& c) {}},
               command);
    return command;
}

int commands::Interpreter::CountBufferedCommands() const
{
    return static_cast<int>(commands_queue_.size());
}