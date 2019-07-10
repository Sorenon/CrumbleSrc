#include "FileUtil.h"
#include <fstream>
#include <string>
#include <iostream>

std::string FileUtil::readFile(std::string path)
{
	std::ifstream input(path, std::ios::ate);
	if (input)
	{
		std::string data;
		data.reserve(input.tellg());
		input.seekg(std::ios::beg);
		data.assign((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
		return data;
	}
	else
	{
		std::cout << "Couldn't find or access file " << path << std::endl;
		return "";
	}
}
