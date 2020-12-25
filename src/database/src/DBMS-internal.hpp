#pragma once

#include "Database.hpp"
#include <string_view>
#include <commands/Interpreter.hpp>
#include <commands/source.hpp>
#include <commands/commands.hpp>
#include <database/Key.hpp>
#include <database/Record.hpp>

namespace db
{
class DBMSInternal
{
   public:
    DBMSInternal(const std::string& database_name,
                 commands::source& src);
    ~DBMSInternal();
    void Run();
    void DispatchCommand(commands::possible_command&& command);

   private:
    DataBase<key::Key, record::Record> db_;
    commands::Interpreter commands_interpreter_;
};
}  // namespace db
