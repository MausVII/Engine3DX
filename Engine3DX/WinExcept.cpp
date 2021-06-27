#include "WinExcept.h";
#include <sstream>;

WinExcept::WinExcept(int line, const char* file) noexcept
	:
	line(line),
	file(file)
{}

const char* WinExcept::what() const noexcept {
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	// Save string in buffer before it dies at the end of func call
	return whatBuffer.c_str();
}

const char* WinExcept::GetType() const noexcept
{
	return "Exception";
}

int WinExcept::GetLine() const noexcept
{
	return line;
}

const std::string& WinExcept::GetFile() const noexcept
{
	return file;
}


std::string WinExcept::GetOriginString() const noexcept {
	std::ostringstream oss;
	oss << "[File] " << file << std::endl
		<< "[Line]" << line;
	return oss.str();
}