#ifndef COMMANDS_COMMANDS_HPP
#define COMMANDS_COMMANDS_HPP

#include <variant>
#include <unordered_map>
#include <string>
#include <string_view>
#include <functional>
#include <database/Key.hpp>
#include <database/Record.hpp>

using namespace std::string_view_literals;
namespace commands
{
struct command_read
{
    area::Key key;
};
struct command_insert
{
    area::Key key;
    area::Record record;
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

struct command_delete
{
    area::Key key;
};

struct command_update
{
    area::Key key;
    area::Record record;
};
using possible_command =
    std::variant<command_read, command_insert, command_show, command_reorganize,
                 command_unknown, command_exit, command_delete, command_update>;

namespace
{
    possible_command match_command_str(const std::string& command)
    {
        const static std::unordered_map<std::string,
                                        std::function<possible_command(void)>>
            possibilities = {
                {"read", []() { return command_read{}; }},
                {"insert", []() { return command_insert{}; }},
                {"show", []() { return command_show{}; }},
                {"reorganize", []() { return command_reorganize{}; }},
                {"exit", []() { return command_exit{}; }},
                {"quit", []() { return command_exit{}; }},
                {"delete", []() { return command_delete{}; }},
                {"update", []() { return command_update{}; }}};
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