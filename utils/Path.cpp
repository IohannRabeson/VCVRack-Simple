#include "Path.hpp"

std::string Path::extractFileName(std::string const& path)
{
	auto lastSlashPos = path.find_last_of("\\/");
	auto lastDotPos = path.find_last_of(".");

	if (lastSlashPos == std::string::npos)
	{
		lastSlashPos = 0u;
	}
	if (lastDotPos == std::string::npos)
	{
		lastDotPos = path.size();
	}

	auto const extractSize = lastDotPos - lastSlashPos - 1;

	return path.substr(lastSlashPos + 1, extractSize);
}
