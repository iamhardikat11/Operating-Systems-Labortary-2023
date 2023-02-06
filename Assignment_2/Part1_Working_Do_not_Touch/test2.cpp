#include <iostream>
#include <vector>
#include <dirent.h>
#include <string.h>

using namespace std;

// function to match a string with a pattern
bool match(char * str, char * pattern)
{
// iterate through the pattern
for (int i = 0; i < strlen(pattern); i++)
{
// if the pattern has a '' character, skip it and all the following characters
if (pattern[i] == '')
{
// find the next character after '' that is not ''
int j = i + 1;
while (j < strlen(pattern) && pattern[j] == '*')
j++;

c

        // if there is no next character, return true
        if (j == strlen(pattern))
            return true;

        // compare the rest of the string with the rest of the pattern
        char * sub_pattern = pattern + j;
        char * sub_str = str + strlen(str) - strlen(sub_pattern);
        while (sub_str >= str)
        {
            if (match(sub_str, sub_pattern))
                return true;
            sub_str--;
        }

        // if there is no match, return false
        return false;
    }

    // if the pattern has a '?' character, skip it
    if (pattern[i] == '?')
        continue;

    // if the pattern and the string have different characters, return false
    if (pattern[i] != str[i])
        return false;
}

// if the pattern and the string have the same length, return true
if (strlen(str) == strlen(pattern))
    return true;

// if the pattern is longer, return false
return false;

}

// function to get all the files in a directory that match a pattern
vector<string> getFiles(char * dir, char * pattern)
{
vector<string> files;

scss

// open the directory
DIR * d = opendir(dir);
if (!d)
    return files;

// iterate through the entries in the directory
dirent * entry;
while ((entry = readdir(d)) != NULL)
{
    // check if the entry is a file and matches the pattern
    if (entry->d_type == DT_REG && match(entry->d_name, pattern))
    {
        // add the file to the vector
        char file[strlen(dir) + strlen(entry->d_name) + 2];
        strcpy(file, dir);
        strcat(file, "/");
        strcat(file, entry->d_name);
        files.push_back(file);
    }
}

// close the directory
closedir(d);

return files;

}

// function to expand a wildcard pattern into a list of files
vector<string> expandWildcard(char * pattern)
{
vector<string> files;
// split the pattern into directory and file parts
char * dir = pattern;
char * file = strr