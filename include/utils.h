#ifndef _H_UTIL_H_
#define _H_UTIL_H_

#include <string>
#include <sstream>
#include <vector>
#include <iterator>


/*
    This is taken from https://stackoverflow.com/questions/236129/the-most-elegant-way-to-iterate-the-words-of-a-string?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
    It's an easy way to split strings by sticking to the std libs.
    I'd rather not re-invent the wheel.

*/

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

static std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

#endif 