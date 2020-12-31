#include "DBMS-internal.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <overloaded/overloaded.hpp>

db::DBMSInternal::DBMSInternal(const std::string &database_name,
                               commands::source &src)
    : commands_interpreter_(src) {
    std::filesystem::create_directory(database_name);
    std::filesystem::path path = database_name;
    path /= database_name;
    database_prefix_ = path.string();
    try {
        db_.Setup(database_prefix_);
        disk_metrics_csv_ << fmt::format(
            "primary_reads,primary_writes,primary_all,overflow_reads,overflow_"
            "writes,overflow_all,all_reads,all_writes,all\n");
    } catch (const std::exception &e) {
        throw std::runtime_error(fmt::format(
            "Failed to initialize database. Message:\n{}", e.what()));
    }
}

db::DBMSInternal::~DBMSInternal() { 
    db_.Save();
    std::ofstream ofs(database_prefix_ + "_disk_access.csv");
    ofs << disk_metrics_csv_.rdbuf();
}

void db::DBMSInternal::Run() {
    enum class process {
        running,
        exit,
        empty_buffer
    } state = process::empty_buffer;
    while (state != process::exit) {
        switch (state) {
        case process::running:
            DispatchCommand(commands_interpreter_.PopCommand());
            state = commands_interpreter_.CountBufferedCommands()
                        ? process::running
                        : process::empty_buffer;
            break;
        case process::empty_buffer:
            commands_interpreter_.LoadCommands();
            state = commands_interpreter_.EndOfCommands() ? process::exit
                                                          : process::running;
            break;
        }
    }
}

void db::DBMSInternal::DispatchCommand(commands::possible_command &&command) {
    std::visit(
        overloaded{[&](commands::command_read &&c) {
                       if (c.key.IsValid()) {
                           db_.Read(c.key);
                       } else {
                           fmt::print("Invalid key.\n");
                       }
                       db_.DumpDiskAccessMetric(disk_metrics_csv_);
                   },
                   [&](commands::command_insert &&c) {
                       if (c.key.IsValid()) {
                           db_.Insert(c.key, c.record);
                       } else {
                           fmt::print("Invalid key.\n");
                       }
                       db_.DumpDiskAccessMetric(disk_metrics_csv_);
                   },
                   [&](commands::command_reorganize &&c) {
                       db_.Reorganize();
                       db_.DumpDiskAccessMetric(disk_metrics_csv_);
                   },
                   [&](commands::command_show &&c) { db_.Show(); },
                   [](commands::command_exit &&c) { fmt::print("Exiting\n"); },
                   [](commands::command_unknown &&c) {
                       fmt::print("Unknown command\n");
                   },
                   [&](commands::command_delete &&c) {
                       if (c.key.IsValid()) {
                           db_.Delete(c.key);
                       } else {
                           fmt::print("Invalid key.\n");
                       }
                       db_.DumpDiskAccessMetric(disk_metrics_csv_);
                   },
                   [&](commands::command_update &&c) {
                       if (c.key.IsValid()) {
                           db_.Update(c.key, c.record);
                       } else {
                           fmt::print("Invalid key.\n");
                       }
                       db_.DumpDiskAccessMetric(disk_metrics_csv_);
                   },
                   [&](commands::command_show_sorted &&) { db_.ShowSorted(); }},
        std::move(command));
}