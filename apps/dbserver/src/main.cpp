#include <cstdint>

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

#include "DatabaseManager.h"

int main(int argc, char** argv)
{
    CLI::App app{"KalOnline DB Server (c) Bango Emu"};

    std::string db_address{"localhost"};
    std::string db_port{"3301"};
    std::string db_user{"root"};
    std::string db_password;
    std::string db_schema{"kalonline"};

    std::string server_address{"0.0.0.0"};
    std::uint16_t server_port = 2999;

    app.add_option("--db_address", db_address, "Database address");
    app.add_option("--db_port", db_port, "Database port");
    app.add_option("--db_user", db_user, "Database user");
    app.add_option("--db_password", db_password, "Database password");

    app.add_option("--server_address", server_address, "Server address");
    app.add_option("--server_port", server_port, "Server port");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    DatabaseManager manager;

    manager.Initialize();

    if (!manager.ConnectToPool(db_address, db_port, db_user, db_password, db_schema)) {
        std::cerr << "Cannot connect to database " << db_address << ":" << db_port << " (user: " << db_user << ")" << std::endl;
        return 1;
    }
    std::cout << "Connected to database " << db_address << ":" << db_port << " (user: " << db_user << ")" << std::endl;

    manager.StartDBServer(server_address, server_port);
    std::cout << "DB Server has started on address " << server_address << ":" << server_port << std::endl;

    std::cin.get();
    return 0;
}
