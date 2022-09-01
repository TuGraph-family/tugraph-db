#include "lgraph/lgraph_rpc_client.h"
#include <getopt.h>
#include <string>
#include <iostream>

static std::string ip;
static std::string port;
static std::string graph;
static std::string user;
static std::string password;
static std::string cypher;

static const char* short_options = "hi:p:g:u:c:";

struct option long_options[] = {
    { "ip", 1, NULL, 'i'},
    { "port", 1, NULL, 'p'},
    { "graph", 1, NULL, 'g'},
    { "user", 1, NULL, 'u'},
    { "password", 1, NULL, 'w'},
    { "cypher", 1, NULL, 'c'},
    { "help", 0, NULL, 'h'}
};

static void print_usage() {
    fprintf(stdout,
            "options:\n"
            "   -h              show this usage\n"
            "   -i --ip         ip for graph server\n"
            "   -p --port       port for graph server\n"
            "   -g --graph      graph name\n"
            "   -u --user       user name\n"
            "   --password      user password\n"
            "   -c --cypher     cypher to query\n");
    exit(1);
}

static void parse_argv(int argc, char** argv) {
    int opt = -1;
    while((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch(opt) {
            case 'i':
                ip = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 'g':
                graph = optarg;
                break;
            case 'u':
                user = optarg;
                break;
            case 'w':
                password = optarg;
                break;
            case 'c':
                cypher = optarg;
                break;
            case 'h':
                print_usage();
                break;
            case '?':
                print_usage();
                break;
        }

    }
}

int main(int argc, char** argv) {
    parse_argv(argc, argv);
    std::string addr = ip + ":" + port;
    lgraph::RpcClient client(addr, user, password);
    std::string res;
    bool ret = client.CallCypher(res, cypher, graph);
    if (ret) {
        std::cout << "query succeed and result : "  << res << std::endl;
    } else {
        std::cout << "query failed and because : " << res << std::endl;
    }
}