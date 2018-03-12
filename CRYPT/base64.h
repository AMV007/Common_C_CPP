#ifndef _M_BASE64_H_
#define _M_BASE64_H_

#include <string>

std::string base64_encode(const unsigned char * , unsigned __int32 len=0);
std::string base64_decode(std::string const& s);

#endif //_M_BASE64_H_
