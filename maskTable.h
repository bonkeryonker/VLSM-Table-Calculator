#pragma once
#include <string>
#include <cmath>

long hostCount(std::string slashMask)
{
	slashMask = slashMask.substr(1, slashMask.length()); //remove / character
	int n = std::stoi(slashMask);
	return std::pow(2, (32 - n));
}

std::string str_hostCount(std::string slashMask)
{
	slashMask = slashMask.substr(1, slashMask.length()); //remove / character
	int n = std::stoi(slashMask);
	std::string val = "2^";
	val += std::to_string(32 - n);
	return val;
}

std::string getNetmask(long neededSize)
{
	std::string val = "/";
	for (int n = 0; n <= 32; n++)
	{
		if (std::pow(2, n) > neededSize)
			return val += std::to_string(32 - n);
	}
}