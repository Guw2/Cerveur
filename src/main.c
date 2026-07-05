#include "logger.h"

int main(void){

	logger_info("Cerveur starting...");
	logger_warn("Warning msg test.");
	logger_error("error msg test.");

	return 0;
}