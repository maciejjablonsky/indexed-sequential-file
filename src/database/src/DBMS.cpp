#include <database/DBMS.hpp>
#include "DBMS-internal.hpp"

db::DBMS::DBMS(const std::string_view filenames_prefix, commands::source& src)
    : impl_(new DBMSInternal(filenames_prefix, src))
{
}

db::DBMS::~DBMS() { delete impl_; }

void db::DBMS::Run() { impl_->Run(); }

void db::DBMS::DispatchCommand(commands::possible_command& command) {
    impl_->DispatchCommand(command);
}
