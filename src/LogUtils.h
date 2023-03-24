#ifndef __LOGUTILS_H__
#define __LOGUTILS_H__

#include <cstdint>
#include "StringUtils.h"


namespace LogUtils {
	enum {
		LogLevel_None = 0,
		LogLevel_DebugOutput,
		LogLevel_Output,
		LogLevel_Warning,
		LogLevel_Error,
		LogLevel_Fatal,
		LogLevel_COUNT
	};
	static constexpr const char* logLevelStrs[] = {"None","Debug","Output","Warning","Error","Fatal"};
	typedef void (*LogCallB)(uint8_t logLevel, const char* msg, const char* fileName , int lineNum, const char* module, void* userData);

	void activateLogTarget(LogCallB callB, void* userData);

	void _log(uint8_t logLevel, std::pair<LogCallB,void*> context, const char* fileName, int lineNum, const char* module, const char* msg);
	void _log(uint8_t logLevel, std::pair<LogCallB,void*> context, const char* fileName, int lineNum, const char* module, const std::string& msg);

	template<typename ... Args>
	void _logf(uint8_t logLevel, std::pair<LogCallB,void*> context, const char* fileName, int lineNum, const char* module, const char* msg, Args ... args) {
		_log(logLevel, context, fileName, lineNum, module, StringUtils::format(msg, args ...));
	}
}


#define LU_LOG(level, msg) LogUtils::_log(level,LU_CONTEXT,__FILE__,__LINE__,LU_MODULE,msg)
#define LU_LOGF(level, msg, ...) LogUtils::_logf(level,LU_CONTEXT,__FILE__,__LINE__,LU_MODULE,msg,__VA_ARGS__)
#define LU_LOG_(level, msg) LogUtils::_log(level,{0,0},__FILE__,__LINE__,LU_MODULE,msg)
#define LU_LOGF_(level, msg, ...) LogUtils::_logf(level,{0,0},__FILE__,__LINE__,LU_MODULE,msg,__VA_ARGS__)

#endif