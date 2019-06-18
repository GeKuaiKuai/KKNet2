#ifndef KKNET_PUBLIC_H
#define KKNET_PUBLIC_H

#include <string>
#include <string.h>
#include <assert.h>
using std::string;

inline void memInit(void* data,size_t len)
{
    ::memset(data, 0, len);
}

inline void show(const string& word)
{
    printf("%s\n",word.c_str());
}

#endif