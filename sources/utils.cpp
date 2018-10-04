#include "utils.hpp"

using namespace utils;

std::string utils::hexlify(uint32_t num)
{
	std::string result = "xxxx";
	result[0] = (char)(num >> 24);
	result[1] = (char)(num >> 16);
	result[2] = (char)(num >> 8);
	result[3] = (char)(num);
	return result;
}

std::string utils::hexlify(const std::string& hash)
{
	auto convert_ch = [](char c) -> char
	{
		if (c >= '0' && c <= '9')
			return (c - '0');
		if (c >= 'a' && c <= 'f')
			return (c - 'a' + 10);

	};

	std::string result(hash.size() / 2, 'x');
	auto j = 0;
	for (size_t i = 0; i < hash.size() / 2; i++)
	{
		result[i] = convert_ch(hash[j++]) * 16;
		result[i] += convert_ch(hash[j++]);
	}
	return result;
}