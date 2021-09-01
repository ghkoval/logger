#ifndef __LOGGER_H__
#define __LOGGER_H__

#pragma once

#ifdef _DEBUG

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>

// #include <source_location> // since C++20

namespace DebugUtils
{
	enum class LogLevel {
		Error = 1,
		Warning = 2,
		Info = 3,
		Debug = 4,
	};

	constexpr char const* levelNames[] = { "Reserved", "ERROR", "WARN", "INFO", "DEBUG" };

	std::string FormatHeader(const char* file, const char* function, int line, size_t logIndent, LogLevel level);

	template <typename T>
	class Logger
	{
	public:
		Logger(LogLevel level = LogLevel::Error) : log_level_(level) {
			log_indent_++;
		}

		~Logger() {
			log_indent_--;
		}

		template <typename... Args>
		void Log(const char* file, const char* function, int line, Args&&... args);

		static void setSystemLogLevel(LogLevel level) { system_log_level_ = level; }

	protected:
		LogLevel log_level_;
		static LogLevel system_log_level_;
		thread_local static size_t log_indent_;
	};

	// explicit instantiation declaration - to implicitly declare log_indent_ and system_log_level_ static variables instances
	class ConsoleLogger;
	class FileLogger;
	extern template class Logger<ConsoleLogger>;
	extern template class Logger<FileLogger>;

	class ConsoleLogger : public Logger<ConsoleLogger> {
	public:
		ConsoleLogger(LogLevel level);

		template <typename... Args>
		void LogMessage(const char* file, const char* function, int line, Args&&... args);
	private:
		static std::mutex lock_;
	};

	class FileLogger : public Logger<FileLogger> {
	public:
		FileLogger(LogLevel level);

		template <typename... Args>
		void LogMessage(const char* file, const char* function, int line, Args&&... args);

	private:
		static std::string log_path_;
		static std::once_flag once_;
		static std::ofstream fstream_;
		static std::mutex lock_;
	};

	template <typename T>
	template <typename... Args>
	void Logger<T>::Log(const char* file, const char* function, int line, Args&&... args) {

		if (log_level_ <= system_log_level_) {
			auto p = static_cast<T*>(this);
			p->LogMessage(file, function, line, std::forward<Args>(args)...);
		}
	}

	template <typename... Args>
	auto UnpackArgs(Args&&... args) {
		std::ostringstream ss;
		ss << '[';
		(ss << ... << args);
		ss << ']';

		return ss.str();
	}

	template <typename... Args>
	void ConsoleLogger::LogMessage(const char* file, const char* function, int line, Args&&... args) {
		auto header = FormatHeader(file, function, line, log_indent_, log_level_);
		auto msg = UnpackArgs(std::forward<Args>(args)...);

		std::unique_lock l(lock_);
		std::this_thread::sleep_for(std::chrono::milliseconds(3));
		std::cout << header << msg << '\n';
	}

	template <typename... Args>
	void FileLogger::LogMessage(const char* file, const char* function, int line, Args&&... args) {
		auto header = FormatHeader(file, function, line, log_indent_, log_level_);
		auto msg = UnpackArgs(std::forward<Args>(args)...);

		std::unique_lock l(lock_);
		std::this_thread::sleep_for(std::chrono::milliseconds(3));
		fstream_ << header << msg << '\n';
	}


	#define __LOGVARNAME_CONCAT(name, line) name##line
	#define __LOGVARNAME(name, line) __LOGVARNAME_CONCAT(name,line)

	#define LOG_INFO(...) DebugUtils::FileLogger __LOGVARNAME(logger, __LINE__)(DebugUtils::LogLevel::Info); __LOGVARNAME(logger, __LINE__).Log(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
	#define LOG_WARN(...) DebugUtils::FileLogger  __LOGVARNAME(logger, __LINE__)(DebugUtils::LogLevel::Warning); __LOGVARNAME(logger, __LINE__).Log(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
	#define LOG_ERROR(...)  DebugUtils::FileLogger  __LOGVARNAME(logger, __LINE__)(DebugUtils::LogLevel::Error); __LOGVARNAME(logger, __LINE__).Log(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
	#define LOG_DEBUG(...)  DebugUtils::FileLogger  __LOGVARNAME(logger, __LINE__)(DebugUtils::LogLevel::Debug); __LOGVARNAME(logger, __LINE__).Log(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);

} // namespace DebugUtils

#else

#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_DEBUG(...)

#endif

#endif // !__LOGGER_H__