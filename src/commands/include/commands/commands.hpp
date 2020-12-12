#ifndef COMMANDS_COMMANDS_HPP
#define COMMANDS_COMMANDS_HPP

#include <variant>
#include <unordered_map>
#include <string>
#include <string_view>
#include <functional>

using namespace std::string_view_literals;
namespace commands
{
struct command_read
{
    int64_t key;
};
struct command_write
{
    int64_t key;
    int record;
};
struct command_show
{
};
struct command_reorganize
{
};
struct command_unknown
{
};

struct command_exit
{
};

using possible_command =
    std::variant<command_read, command_write, command_show, command_reorganize,
                 command_unknown, command_exit>;

namespace
{
    possible_command match_command_str(const std::string& command)
    {
        const static std::unordered_map<std::string,
                                        std::function<possible_command(void)>>
            possibilities = {
                {"read", []() { return command_read{}; }},
                {"write", []() { return command_write{}; }},
                {"show", []() { return command_show{}; }},
                {"reorganize", []() { return command_reorganize{}; }},
                {"exit", []() { return command_exit{}; }},
                {"quit", []() { return command_exit{}; }}};
        auto result = possibilities.find(command);
        if (result == possibilities.end())
        {
            return command_unknown{};
        }
        return result->second();
    }
}  // namespace
}  // namespace commands

#endif  // COMMANDS_COMMANDS_HPP