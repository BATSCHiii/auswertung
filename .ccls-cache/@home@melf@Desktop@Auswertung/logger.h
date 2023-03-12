#ifndef __LOGGER_HEADER__
#define __LOGGER_HEADER__

#include "iostream"

class Logger {
	public:
		std::string name;
		int reads = 0;
		int writes = 0;
		int hits = 0;
		int read_hits = 0;
		int write_hits = 0;
		int misses = 0;
		int read_misses = 0;
		int write_misses = 0;
		int replacements = 0;

	void print() {
		std::cout << "***** [LOGGER: " << name << "] *****" << std::endl;
		std::cout << "Hits: " << std::dec << read_hits + write_hits << std::endl;
		std::cout << "Misses: " << read_misses + write_misses << std::endl;
		std::cout << "Reads : " << reads << std::endl;
		std::cout << "Read hits: " << read_hits << std::endl;
		std::cout << "Read misses: " << read_misses << std::endl;
		std::cout << "Writes: " << writes << std::endl;
		std::cout << "Write hits: " << write_hits << std::endl;
		std::cout << "Write misses: " << write_misses << std::endl;
		std::cout << "Replacements: " << replacements << std::endl;
	}
};

#endif
