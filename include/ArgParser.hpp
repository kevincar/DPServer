
#ifndef DPS_ARGPARSER_HPP
#define DPS_ARGPARSER_HPP

#include <string>
#include <vector>

class ArgParser
{
	public:

		ArgParser(int argc, char** argv);

		static std::vector<std::string> vectorize(int argc, char** argv);

		std::vector<std::string> getArgs(void);

	private:

		int nArgs;
		std::vector<std::string> args;
};

#endif /* DPS_ARGPARSER_HPP */
