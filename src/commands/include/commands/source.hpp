#ifndef COMMANDS_SOURCE_HPP
#define COMMANDS_SOURCE_HPP

#include <variant>
#include <string>

namespace commands
{
using from_file = struct {
    std::string path;
};
using from_prompt = struct {
};
using source = std::variant<commands::from_file, commands::from_prompt>;
}

#endif  // COMMANDS_SOURCE_HPP