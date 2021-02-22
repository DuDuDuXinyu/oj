#pragma once

#include <string>

class Base64
{
public:
	std::string Code(const std::string& strData);
	std::string DeCode(const std::string& strData);
};