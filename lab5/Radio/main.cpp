#include "./src/RadioServer.h"
#include "./src/RadioClient.h"
#include <iostream>
#include <csignal>
#include <stdexcept>
#include <memory>

std::unique_ptr<RadioServer> g_server;
std::unique_ptr<RadioClient> g_client;

void SignalHandler(int)
{
    if (g_server)
    {
        g_server->Stop();
    }
    if (g_client)
    {
        g_client->Stop();
    }
}

struct Args
{
    enum class Mode
    {
        Server, Client
    };

    Mode mode;
    std::string address;
    int port;
};

Args ParseArgs(int argc, char* argv[])
{
    if (argc < 2 || argc > 3)
    {
        throw std::invalid_argument(
                "Usage:\n"
                "  Server: " + std::string(argv[0]) + " <port>\n"
                                                      "  Client: " + std::string(argv[0]) + " <address> <port>\n"
        );
    }

    Args args{};

    try
    {
        if (argc == 2)
        {
            args.mode = Args::Mode::Server;
            args.port = std::stoi(argv[1]);
        } else
        {
            args.mode = Args::Mode::Client;
            args.address = argv[1];
            args.port = std::stoi(argv[2]);
        }
    } catch (const std::invalid_argument&)
    {
        throw std::invalid_argument("Port must be a valid number");
    } catch (const std::out_of_range&)
    {
        throw std::invalid_argument("Port number is out of range");
    }

    if (args.port < 1 || args.port > 65535)
    {
        throw std::invalid_argument("Port must be between 1 and 65535");
    }

    return args;
}

int main(int argc, char* argv[])
{
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    try
    {
        Args args = ParseArgs(argc, argv);

        if (args.mode == Args::Mode::Server)
        {
            g_server = std::make_unique<RadioServer>(args.port);
            g_server->Run();
        } else
        {
            g_client = std::make_unique<RadioClient>(args.address, args.port);
            g_client->Run();
        }
    } catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}