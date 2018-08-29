#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <fstream>
#include <stdexcept>

/**
 * Returns the contents of the file denoted by the provided path.
 *
 * @param path Path to the file whose contents to read.
 * @throw std::ifstream::failure if the provided path is invalid.
 * @return string containing the contents of the read file.
 */
std::string file_str(const std::string & path)
{
    // Stream to file (Automatically closes in destructor).
    std::ifstream ifs(&path[0], std::ios_base::in | std::ios_base::binary);
    if (!ifs)
        throw std::invalid_argument("File: " + path + " not found!");
    // Enable exceptions
    ifs.exceptions();

    // Init string buffer to hold file data.
    std::string buffer;
    // Set ifs input position indicator to end.
    ifs.seekg(0, std::ios::end);
    // Set the buffer capacity
    buffer.resize(ifs.tellg());
    // Set ifs input position indicator to beginning.
    ifs.seekg(0);
    // Read file contents into string buffer.
    ifs.read(&buffer[0], buffer.size());
    ifs.close();
    // Return by RVO (Reurn value optimization).
    return buffer;
}

#endif
