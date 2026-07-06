#include "logger.h"
#include "config.h"
#include "server.h"
#include "http_method.h"

#include "stdio.h"

int main(void){

	ServerConfig config;

	server_config_default(&config);
	server_config_load(&config, "cerveur.conf");

	logger_set_colors(config.log_colors);

	logger_info("Cerveur starting...");
	logger_info("Port: %d", config.port);
	
	Server server;

	server_init(&server, &config);

	int result = server_start(&server);


	server_free(&server);
	server_config_free(&config);

	return result == 0 ? 0 : 1;
}