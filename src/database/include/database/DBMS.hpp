#pragma once

#include <commands/commands.hpp>
#include <commands/source.hpp>

namespace db {
class DBMSInternal;
class DBMS {
  public:
    DBMS(const std::string &database_name, commands::source &src);
    void Run();
    void DispatchCommand(commands::possible_command &&command);
    ~DBMS();

  private:
    DBMSInternal *impl_;
};
} // namespace db
