#include "DBMS-internal.hpp"
#include <database/DBMS.hpp>

db::DBMS::DBMS(const std::string &database_name, commands::source &src)
    : impl_(new DBMSInternal(database_name, src)) {}

db::DBMS::~DBMS() { delete impl_; }

void db::DBMS::Run() { impl_->Run(); }

void db::DBMS::DispatchCommand(commands::possible_command &&command) {
    impl_->DispatchCommand(std::move(command));
}
