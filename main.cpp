#include "ServerConfigParser.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "colours.h"

#include <exception>
#include <ios>
#include <map>
#include <set>
#include <sys/_types/_int8_t.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <poll.h>
#include <fcntl.h>

using std::string;
using std::cout;
using std::endl;

using namespace webserv;

void start_server()
{
	std::ifstream		conf_file("./webserv.conf");
	ServerConfigParser	config_parser(conf_file);

	config_parser.parse_config();
	config_parser.validate_config();

	cout << config_parser << endl;

	Server				server(config_parser);
	server.launch();
}

int main(void)
{
    // string d_conf_file;

    // if (argc < 2)
    //     d_conf_file = "./webserv.conf";
    // else
    //     d_conf_file = argv[1];

    // if (access(d_conf_file.c_str(), F_OK) != 0) {
    //     perror("Could not find webserv.conf in current directory");
    //     return -1;
    // }
	
	try
	{
		start_server();
	}
	catch (std::exception &e)
	{
		cout << RED << e.what() << RESET << endl;
		return (1);
	}
	return (0);
}
