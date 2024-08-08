#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

inline std::string ReadFile(const char* f_name = "kernels.cl")
{
	std::fstream newfile;

	newfile.open(f_name, std::ios::in); //open a file to perform read operation using file object
	if (newfile.is_open())
	{
		std::stringstream strStream;
		strStream << newfile.rdbuf(); //read the file
		std::string text = strStream.str(); //str holds the content of the file
		std::cout << text << std::endl;

		newfile.close(); //close the file object.

		return text;
	}
}
