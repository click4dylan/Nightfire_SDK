#pragma once
#include <Windows.h>
#include <stdio.h>

inline void CopyDirectoryName(const char* path, char* directory) 
{
    const char* last_separator = &path[strlen(path) - 1];
    char current_char;

    // Find the last directory separator ('/' or '\')
    for (; last_separator != path; --last_separator) {
        current_char = *(last_separator - 1);
        if (current_char == '\\' || current_char == '/')
            break;
    }

    // Calculate the length of the directory name
    unsigned int directory_length = last_separator - path;

    // Copy the directory name to the output buffer
    memcpy(directory, path, directory_length);
    directory[directory_length] = '\0';
}

inline void ExtractFileNameWithoutExtension(char* output, char* filePath) 
{
    char* lastSeparator = &filePath[strlen(filePath) - 1];
    char currentChar;

    // Find the last directory separator ('\' or '/')
    for (; lastSeparator != filePath; --lastSeparator) {
        currentChar = *(lastSeparator - 1);
        if (currentChar == '\\' || currentChar == '/')
            break;
    }

    // Extract the file name without extension
    char* result = lastSeparator;
    char currentCharResult;
    for (currentCharResult = *result; currentCharResult; ++result) {
        if (currentCharResult == '.')
            break;
        *output = currentCharResult;
        currentCharResult = result[1];
        ++output;
    }
    *output = '\0';
}

extern FILE* SafeOpenWrite(const char* const filename);
extern FILE* SafeOpenRead(const char* filename);
extern void SafeRead(FILE* f, void* buffer, int count);
extern void SafeWrite(FILE* f, const void* const buffer, int count);
extern int q_filelength(FILE* f);

extern int LoadFile(const char* name, void** bufferptr);