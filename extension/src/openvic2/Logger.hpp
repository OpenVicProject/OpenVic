#pragma once

#include <functional>
#include <sstream>
#ifdef __cpp_lib_source_location
#include <source_location>
#endif

namespace OpenVic2 {

	#ifndef __cpp_lib_source_location
	#include <string>
	//Implementation of std::source_location for compilers that do not support it
	//Note: uses non-standard extensions that are supported by Clang, GCC, and MSVC
	//https://clang.llvm.org/docs/LanguageExtensions.html#source-location-builtins
	//https://stackoverflow.com/a/67970107
	class source_location {
		std::string _file;
		int _line;
		std::string _function;

		public:
		source_location(std::string f, int l, std::string n) : _file(f), _line(l),  _function(n) {}
		static source_location current(std::string f = __builtin_FILE(), int l = __builtin_LINE(), std::string n = __builtin_FUNCTION()) {
			return source_location(f, l, n);
		}

		inline const char* file_name() const { return _file.c_str(); }
		inline int line() const {return _line; }
		inline const char* function_name() const { return _function.c_str(); }
	};
	#endif

	class Logger {
		using log_func_t = std::function<void(std::string&&)>;

		#ifdef __cpp_lib_source_location
		using source_location = std::source_location;
		#else
		using source_location = OpenVic2::source_location;
		#endif

		static log_func_t info_func, error_func;

		static const char* get_filename(const char* filepath);

		template <typename... Ts>
		struct log {
			log(log_func_t log_func, Ts&&... ts, const source_location& location) {
				if (log_func) {
					std::stringstream stream;
					stream << std::endl << get_filename(location.file_name()) << "(" << location.line() << ") `" << location.function_name() << "`: ";
					((stream << std::forward<Ts>(ts)), ...);
					stream << std::endl;
					log_func(stream.str());
				}
			}
		};
	public:
		static void set_info_func(log_func_t log_func);
		static void set_error_func(log_func_t log_func);

		template <typename... Ts>
		struct info {
			info(Ts&&... ts, const source_location& location = source_location::current()) {
				log<Ts...>{ info_func, std::forward<Ts>(ts)..., location };
			}
		};

		template <typename... Ts>
		info(Ts&&...) -> info<Ts...>;

		template <typename... Ts>
		struct error {
			error(Ts&&... ts, const source_location& location = source_location::current()) {
				log<Ts...>{ error_func, std::forward<Ts>(ts)..., location };
			}
		};

		template <typename... Ts>
		error(Ts&&...) -> error<Ts...>;
	};
}
