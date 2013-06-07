#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

const char InvalidFilePathChars[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|'}; 


/// Summary:
/// Reads the entire contents of a file in text mode into a string.
/// Arg:
///     fileName - The path to the file to read
/// Returns:
///     A string containing the contents of the file
inline std::string ReadFileToString(const std::string& fileName)
{
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit );
    file.open(fileName);
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

/// Summary:
/// Reads the entire contents of a file and places each line of text into its own string.
/// Arg:
///     fileName - The path to the file to read
/// Returns:
///     vector<string> containing each line of the file. The order of the strings in the vector are the same as were in the file.
inline std::vector<std::string> ReadFileToStrings(const std::string& fileName)
{
    using std::getline;

    std::ifstream file;
    file.exceptions(std::ifstream::badbit);
    file.open(fileName);
    std::vector<std::string> lines;
    while (file)
    {
        std::string newLine;
        std::getline(file, newLine);
        if (file || !newLine.empty())
            lines.push_back(std::move(newLine));
    }
    return lines;
}


#ifdef WIN32
inline bool MakeFileOrDirHidden(const std::string& filePath)
{
    return SetFileAttributes(filePath.c_str(), GetFileAttributes(filePath.c_str()) | FILE_ATTRIBUTE_HIDDEN) != 0;
}
#endif // WIN32
