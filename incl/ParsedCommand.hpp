#pragma once
#include <string>
#include <vector>

struct ParsedCommand
{
	std::string					command;
	std::vector<std::string>	params;
};
