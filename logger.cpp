#ifdef _DEBUG

#ifdef __STDC_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif
#include <stdlib.h> // for getenv()

#include "logger.h"

#include <stdexcept>
#include <iomanip>
#include <vector>
#include <cstring>
#include <chrono>
#include <cassert>
#include <ctime>

#ifdef _MSC_VER
#include <process.h> // for _getpid()
#else
#include <unistd.h> // for getpid()
typedef int errno_t;
#endif

namespace {
	inline auto GetPID()
	{
	#ifdef _MSC_VER
		return _getpid();
	#else
		return getpid();
	#endif
	}

	inline errno_t GetEnv( size_t *len, char *value,
                  size_t valuesz, const char *name )
	{
	#if !__STDC_WANT_SECURE_LIB__ || defined __STDC_LIB_EXT1__
		char* p = ::getenv(name);
		if (p) {
			auto length = std::strlen(p);
			if (len) *len = length;
			if (value) std::strncpy(value, p, valuesz);
	}
		else {
			if (len) *len = 0;
			if (value) *value = 0;
		}
		return 0;
	#else
		return ::getenv_s(len, value, valuesz, name);
	#endif
	}

	template<typename T>
	auto GetMilliseconds(const T& duration)
	{
		auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(duration);
		auto now_sec = std::chrono::time_point_cast<std::chrono::seconds>(duration);

		return std::chrono::milliseconds{ now_ms - now_sec };
	}

	std::string LocalTime()
	{
		auto now = std::chrono::system_clock::now();
		auto now_ms = GetMilliseconds(now);
		auto now_ms_str = std::to_string(now_ms.count());

		auto timenow = std::chrono::system_clock::to_time_t(now);

		struct tm tm_buf {};

#ifdef _MSC_VER
		errno_t err = ::localtime_s(&tm_buf, &timenow);
		assert(err == 0);
#else
		auto tm_buf_ptr = ::localtime_r(&timenow, &tm_buf);
		assert(tm_buf_ptr == &tm_buf);
#endif

		std::string time;
		char time_buf[70];

		size_t numberOfBytesWritten = strftime(time_buf, sizeof time_buf, "%F %T", &tm_buf);
		assert(numberOfBytesWritten > 0);

		if (numberOfBytesWritten) {
			time = time_buf;

			if (now_ms_str.length() == 1)      // now_ms < 10
				now_ms_str = "00" + now_ms_str;
			else if (now_ms_str.length() == 2) // now_ms < 100
				now_ms_str = "0" + now_ms_str;

			time += '.' + now_ms_str;
		}

		return time;
	}
}

namespace DebugUtils
{
// Explicit instantiation
template class Logger<ConsoleLogger>;
template class Logger<FileLogger>;

template<>
LogLevel Logger<ConsoleLogger>::system_log_level_{ LogLevel::Info };
template<>
LogLevel Logger<FileLogger>::system_log_level_{ LogLevel::Info };

template<>
thread_local size_t Logger<ConsoleLogger>::log_indent_ = 0;
template<>
thread_local size_t Logger<FileLogger>::log_indent_ = 0;

constexpr char kDefaultLogFileName[] = "log.txt";
constexpr char kLogPathEnvVarName[] = "LOG_PATH";

std::string FileLogger::log_path_(kDefaultLogFileName);
std::once_flag FileLogger::once_;
std::ofstream FileLogger::fstream_;

std::mutex ConsoleLogger::lock_;
std::mutex FileLogger::lock_;

ConsoleLogger::ConsoleLogger(LogLevel level) :
	Logger<ConsoleLogger>(level) {}

FileLogger::FileLogger(LogLevel level) :
	Logger<FileLogger>(level)
{
	std::call_once(once_, []() {
		size_t len {};

		auto err = GetEnv(&len, nullptr, 0, kLogPathEnvVarName);
		if (err == 0 && len != 0) {
			std::vector<char> buf(len);
			err = GetEnv(&len, buf.data(), buf.size(), kLogPathEnvVarName);

			if (err == 0) {
				log_path_.assign(buf.data(), buf.size());
			}
		}

		std::cout << "The log file path is: " << log_path_ << '\n';

		fstream_.open(log_path_, std::ios_base::out | std::ios_base::app);
	});

	if (!fstream_.is_open()) {
		throw std::runtime_error("Could not open log file [" + log_path_ + "]");
	}
}

std::string FormatHeader(const char* file, const char* function, int line, size_t logIndent, DebugUtils::LogLevel level)
{
	std::ostringstream ss;
	ss << std::setw(6) << std::right << GetPID()
		<< std::setw(6) << std::right << std::hex << std::this_thread::get_id() << std::dec
		<< ' ' << LocalTime() << ' '
		<< std::setw(6) << std::right << levelNames[static_cast<int>(level)] << ' '
		<< std::string(static_cast<size_t>(4 * logIndent), ' ')
		<< file << ':' << line << ' ' << function << ' ';

	return ss.str();
}

} // namespace DebugUtils

#endif // _DEBUG