#include "DBMS-internal.hpp"
#include <fmt/format.h>

template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

db::DBMSInternal::DBMSInternal(const std::string_view filenames_prefix,
                               commands::source& src)
    : commands_interpreter_(src), db_(filenames_prefix)
{
}

void db::DBMSInternal::Run()
{
    enum class processing_state
    {
        running,
        exit,
        empty_buffer
    } state = processing_state::empty_buffer;
    while (state != processing_state::exit)
    {
        switch (state)
        {
            case processing_state::running:
                DispatchCommand(commands_interpreter_.PopCommand());
                state = commands_interpreter_.CountBufferedCommands()
                            ? processing_state::running
                            : processing_state::empty_buffer;
                break;
            case processing_state::empty_buffer:
                commands_interpreter_.LoadCommands();
                state = commands_interpreter_.EndOfCommands()
                            ? processing_state::exit
                            : processing_state::running;
                break;
        }
    }
}

void db::DBMSInternal::DispatchCommand(commands::possible_command&& command)
{
    std::visit(
        overloaded{[&](commands::command_read&& c) {
                       fmt::print("Reading record - key: {}\n",
                                  static_cast<std::string>(c.key));
                       auto record = db_.Read(c.key);
                   },
                   [](commands::command_write&& c) {
                       fmt::print("Writing record - key: {}, value: {}\n",
                                  static_cast<std::string>(c.key),
                                  static_cast<std::string>(c.record));
                   },
                   [](commands::command_reorganize&& c) {
                       fmt::print("Reorganizing files\n");
                   },
                   [](commands::command_show&& c) {
                       fmt::print("Showing whole index and file\n");
                   },
                   [](commands::command_exit&& c) { fmt::print("Exiting\n"); },
                   [](commands::command_unknown&& c) {
                       fmt::print("Unknown command\n");
                   },
                   [](commands::command_delete&& c) {
                       fmt::print("Deleting record - key: {}\n",
                                  static_cast<std::string>(c.key));
                   }},
        std::move(command));
}