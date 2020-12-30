#pragma once

#include <database/Key.hpp>
#include <database/Record.hpp>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

using namespace std::string_view_literals;
namespace commands {
struct command_read {
    key::Key key;
};
struct command_insert {
    key::Key key;
    record::Record record;
};
struct command_show {};
struct command_reorganize {};
struct command_unknown {};

struct command_exit {};

struct command_delete {
    key::Key key;
};

struct command_update {
    key::Key key;
    record::Record record;
};
struct command_show_sorted {};
using possible_command =
    std::variant<command_read, command_insert, command_show, command_reorganize,
                 command_unknown, command_exit, command_delete, command_update,
                 command_show_sorted>;

namespace {
possible_command match_command_str(const std::string &command) {
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
            {"show_sorted", []() { return command_show_sorted{}; }},
            {"update", []() { return command_update{}; }}};
    auto result = possibilities.find(command);
    if (result == possibilities.end()) {
        return command_unknown{};
    }
    return result->second();
}
} // namespace
} // namespace commands
