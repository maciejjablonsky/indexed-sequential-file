#include <cxxopts.hpp>
#include <fmt/format.h>

auto define_options()
{
    cxxopts::Options opts("database", "Client app for indexed-sequential file structure");
    opts.add_options() /**/
        ("p,prefix", "Prefix for files used to store database records.",
         cxxopts::value<std::string>()) /**/
        ("c,commands", "Path to file with commands. If not given interactive prompt is used.",
         cxxopts::value<std::string>()) /**/
        ("h,help", "Print help");
    return opts;
}

auto unpack_required_options(cxxopts::ParseResult& parse_result)
{
    try
    {
        return std::tuple{[&]() { return parse_result["prefix"].as<std::string>(); }(), [&]() {}()};
    }
    catch (const std::exception& e)
    {
        fmt::print("Invalid arguments. Message: {}", e.what());
    }
}

int main(int argc, const char* argv[])
{
    auto opts = define_options();
    auto result = opts.parse(argc, argv);
    if (result.count("help"))
    {
        fmt::print("{}", opts.help());
        std::exit(0);
    }

    auto&& [files_prefix, commands_src] = unpack_required_options(result);
}