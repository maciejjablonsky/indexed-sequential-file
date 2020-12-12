#ifndef DATABASE_DBMS_HPP
#define DATABASE_DBMS_HPP
#include <database/Database.hpp>
#include <commands/Interpreter.hpp>

namespace db
{
class DBMS
{
   public:
    DBMS(const std::string_view filenames_prefix, commands::source& src);
    void Run();

   private:
    DataBase db_;
    commands::Interpreter interpreter_;
};
}  // namespace db

#endif  // DATABASE_DBMS_HPP