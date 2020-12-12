#ifndef DATABASE_DBMS_HPP
#define DATABASE_DBMS_HPP
#include <database/Database.hpp>
#include <commands/Interpreter.hpp>
#include <commands/commands.hpp>

namespace db
{
const static auto inline DEFAULT_CONFIG_PATH = "dbms_config.json";
class DBMS
{
   public:
    DBMS(const std::string_view filenames_prefix, commands::source& src);
    void Run();
    void DispatchCommand(commands::possible_command & command);

   private:
    DataBase db_;
    commands::Interpreter commands_interpreter_;
};
}  // namespace db

#endif  // DATABASE_DBMS_HPP