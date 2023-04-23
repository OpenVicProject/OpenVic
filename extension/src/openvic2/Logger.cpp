#include "openvic2/Logger.hpp"

#include <iostream>

using namespace OpenVic2;

Logger::log_func_t Logger::info_func = [](std::string&& str) { std::cout << str; };
Logger::log_func_t Logger::error_func = [](std::string&& str) { std::cerr << str; };

const char* Logger::get_filename(const char* filepath) {
	if (filepath == nullptr) return nullptr;
	const char *last_slash = filepath;
	while (*filepath != '\0') {
		if (*filepath == '\\' || *filepath == '/') last_slash = filepath + 1;
		filepath++;
	}
	return last_slash;
}

void Logger::set_info_func(log_func_t log_func) {
	info_func = log_func;
}

void Logger::set_error_func(log_func_t log_func) {
	error_func = log_func;
}
