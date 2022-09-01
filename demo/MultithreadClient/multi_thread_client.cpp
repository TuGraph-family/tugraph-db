#include <getopt.h>
#include <memory>
#include <string>
#include <iostream>
#include <functional>
#include "client.h"



static const char* short_options = "hg:u:m:t:";

struct option long_options[] = {
        { "host", 1, NULL, 's'},
        { "user", 1, NULL, 'u'},
        { "password", 1, NULL, 'w'},
        { "mode", 1, NULL, 'm'},
        { "thread", 1, NULL, 't'},
        { "input", 1, NULL, 'i'},
        { "output", 1, NULL, 'o'},
        { "continue_on_error", 1, NULL, 'c'},
        { "help", 0, NULL, 'h'}
};

static void print_usage() {
    fprintf(stdout,
            "options:\n"
            "   -h                      show this usage\n"
            "   -u --user               user name\n"
            "   -m --mode               which mode(callcypher，callplugin, loadplugin, deleteplugin)\n"
            "   -t --thread             client thread number, if value Is greater than cpu number，value is cpu number\n"
            "   --host                  server host(127.0.0.1:9190)\n"
            "   --input                 input file\n"
            "   --output                output file\n"
            "   --continue_on_error     ignore when an error occurs\n"
            "   --password              user password\n");
    exit(1);
}

static void parse_argv(int argc, char** argv, std::shared_ptr<multithread_client::Client> client) {
    int opt = -1;
    while((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch(opt) {
            case 's':
                client->set_host(optarg);
            case 'u':
                client->set_user(optarg);
                break;
            case 'w':
                client->set_password(optarg);
                break;
            case 'm':
                client->set_mode(optarg);;
                break;
            case 't':
                client->set_thread_num(optarg);
                break;
            case 'i':
                client->set_input(optarg);
                break;
            case 'o':
                client->set_output(optarg);
                break;
            case 'c':
                client->set_continue_on_error(optarg);
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
    std::shared_ptr<multithread_client::Client> client(std::make_shared<multithread_client::Client>());
    parse_argv(argc, argv, client);
    client->process();
}
