#include <commands/source.hpp>
#include <cxxopts.hpp>
#include <database/DBMS.hpp>
#include <fmt/format.h>
#include <string>
#include <string_view>

auto define_options(const std::string_view program_name) {
    cxxopts::Options opts(program_name.data(),
                          "Client app for indexed-sequential file structure");
    opts.add_options() /**/
        ("p,prefix",
         "Prefix for files used to store database records. DBMS'll try to use "
         "existing files first or create new if it doesn't exist.",
         cxxopts::value<std::string>()) /**/
        ("c,commands",
         "Path to file with commands. If not given interactive prompt is used.",
         cxxopts::value<std::string>()) /**/
        ("h,help", "Prints help.");
    return opts;
}

auto unpack_required_options(cxxopts::ParseResult &parse_result) {
    try {
        return std::tuple{
            [&]() { return parse_result["prefix"].as<std::string>(); }(),
            [&]() -> commands::source {
                if (parse_result["commands"].count()) {
                    return commands::from_file{
                        parse_result["commands"].as<std::string>()};
                }
                return commands::from_prompt{};
            }()};
    } catch (const std::exception &e) {
        fmt::print("Invalid arguments. Message: {}", e.what());
        std::exit(-1);
    }
}

int main(int argc, const char *argv[]) {
    auto opts = define_options(argv[0]);
    auto result = opts.parse(argc, argv);
    if (result.count("help")) {
        fmt::print("{}", opts.help());
        std::exit(0);
    }

    try {
        auto &&[files_prefix, commands_source] =
            unpack_required_options(result);
        db::DBMS dbms(files_prefix, commands_source);
        dbms.Run();
    } catch (const std::exception &e) {
        fmt::print("Something went wrong. Message: {}\n", e.what());
        std::exit(-1);
    }
}