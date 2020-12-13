#ifndef DATBASE_DBMS_INTERNAL_HPP
#define DATBASE_DBMS_INTERNAL_HPP

#include "Database.hpp"
#include <string_view>
#include <commands/Interpreter.hpp>
#include <commands/source.hpp>
#include <commands/commands.hpp>

namespace db
{
class DBMSInternal
{
   public:
    DBMSInternal(const std::string_view filenames_prefix,
                 commands::source& src);
    void Run();
    void DispatchCommand(commands::possible_command& command);

   private:
    DataBase db_;
    commands::Interpreter commands_interpreter_;
};
}  // namespace db

#endif  // DATBASE_DBMS_INTERNAL_HPP