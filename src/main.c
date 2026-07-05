#include "logger.h"
#include "config.h"

#include "stdio.h"

int main(void){

	ServerConfig config;

	server_config_default(&config);
	server_config_load(&config, "cerveur.conf");

	logger_set_colors(config.log_colors);

	logger_info("Cerveur starting...");
	logger_warn("Warning msg test.");
	logger_error("error msg test.");
	logger_info("Port: %d", config.port);
    logger_info("Public dir: %s", config.public_dir);
    logger_info("Max request size: %zu", config.max_request_size);
    logger_info("Max headers: %zu", config.max_headers);
    logger_info("Log level: %s", config.log_level);
    logger_info("Log colors: %s", config.log_colors ? "true" : "false");

	server_config_free(&config);

	return 0;
}