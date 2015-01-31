#pragma  once

#include <string>

#ifndef _BASE64_H_
#define _BASE64_H_

std::string base64_encodes(unsigned char const* , unsigned int len);
std::string base64_decodes(std::string const& s);

#endif