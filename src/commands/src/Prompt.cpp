#include <commands/Prompt.hpp>
#include <iostream>
#include <fmt/format.h>

std::string commands::Prompt::GetNextCommand()
{
    fmt::print("> ");
    std::string line;
    std::getline(std::cin, line);
    return line;
}