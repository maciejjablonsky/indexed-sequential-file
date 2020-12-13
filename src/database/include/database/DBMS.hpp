#ifndef DATABASE_DBMS_HPP
#define DATABASE_DBMS_HPP
#include <commands/commands.hpp>
#include <commands/source.hpp>


namespace db
{
class DBMSInternal;
class DBMS
{
   public:
    DBMS(const std::string_view filenames_prefix, commands::source& src);
    void Run();
    void DispatchCommand(commands::possible_command && command);
    ~DBMS();

   private:
    DBMSInternal* impl_;
};
}  // namespace db

#endif  // DATABASE_DBMS_HPP