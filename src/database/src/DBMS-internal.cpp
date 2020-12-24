#include "DBMS-internal.hpp"
#include <fmt/format.h>
#include <overloaded/overloaded.hpp>
#include <filesystem>

db::DBMSInternal::DBMSInternal(const std::string& database_name, commands::source& src) : commands_interpreter_(src)
{
    std::filesystem::create_directory(database_name);
    std::filesystem::path path = database_name;
    path /= database_name;
    if (!db_.Setup(path.string()))
    {
        throw std::runtime_error("Failed to initialize database.");
    }
}

void db::DBMSInternal::Run()
{
    enum class process
    {
        running,
        exit,
        empty_buffer
    } state = process::empty_buffer;
    while (state != process::exit)
    {
        switch (state)
        {
            case process::running:
                DispatchCommand(commands_interpreter_.PopCommand());
                state = commands_interpreter_.CountBufferedCommands() ? process::running : process::empty_buffer;
                break;
            case process::empty_buffer:
                commands_interpreter_.LoadCommands();
                state = commands_interpreter_.EndOfCommands() ? process::exit : process::running;
                break;
        }
    }
}

void db::DBMSInternal::DispatchCommand(commands::possible_command&& command)
{
    std::visit(
        overloaded{[&](commands::command_read&& c) {
                       fmt::print("Reading record - key: {}\n", static_cast<std::string>(c.key));
                   },
                   [&](commands::command_insert&& c) {
                       fmt::print("Writing record - key: {}, value: {}\n", static_cast<std::string>(c.key),
                                  static_cast<std::string>(c.record));
                   },
                   [](commands::command_reorganize&& c) { fmt::print("Reorganizing files\n"); },
                   [&](commands::command_show&& c) {}, [](commands::command_exit&& c) { fmt::print("Exiting\n"); },
                   [](commands::command_unknown&& c) { fmt::print("Unknown command\n"); },
                   [](commands::command_delete&& c) {
                       fmt::print("Deleting record - key: {}\n", static_cast<std::string>(c.key));
                   },
                   [](commands::command_update&& c) {
                       fmt::print("Updating record - key: {} with {} value\n", static_cast<std::string>(c.key),
                                  static_cast<std::string>(c.record));
                   }},
        std::move(command));
}