#pragma once

#include "Database.hpp"
#include <commands/Interpreter.hpp>
#include <commands/commands.hpp>
#include <commands/source.hpp>
#include <database/Record.hpp>
#include <string_view>

namespace db {
class DBMSInternal {
  public:
    DBMSInternal(const std::string &database_name, commands::source &src);
    ~DBMSInternal();
    void Run();
    void DispatchCommand(commands::possible_command &&command);

  private:
    DataBase<record::Record> db_;
    commands::Interpreter commands_interpreter_;
};
} // namespace db
