#include "..\include\commands\FileReader.hpp"
#include <commands/FileReader.hpp>
#include <fmt/format.h>
#include <string>

commands::FileReader::FileReader(const std::string& path)
try : path_(path), stream_(std::make_unique<std::ifstream>(path))
{
}
catch (const std::exception& e)
{
    throw std::runtime_error(
        fmt::format("ERROR: Failed to open file at [{}].", path));
}

bool commands::FileReader::EndOfFile() const { return stream_->eof(); }

std::string commands::FileReader::ReadNextCommand()
{
    std::string line;
    std::getline(*stream_, line);
    return line;
}

