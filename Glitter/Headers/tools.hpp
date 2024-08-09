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

inline std::string ReadFile2(const char* f_name = "kernels.cl")
{
    std::string kernel_code;
    std::ifstream kernel_file;
    // ensure ifstream objects can throw exceptions:
    kernel_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open file
        kernel_file.open(f_name);
        std::stringstream kernel_stream;
        // read file's buffer content into stream
        kernel_stream << kernel_file.rdbuf();
        // close file handlers
        kernel_file.close();
        // convert stream into string
        kernel_code = kernel_stream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }
    
    // return char sequence
    return kernel_code.c_str();
}
