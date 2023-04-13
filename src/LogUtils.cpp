#include "LogUtils.h"

#include <map>
#include <iostream>

static std::pair<LogUtils::LogCallB, void*> staticLogContext = { 0,0 };

void LogUtils::_log(uint8_t logLevel, std::pair<LogCallB,void*> context, const char* fileName, int lineNum, const char* module, const char* msg) {
	if (context.first && context.second) {
		context.first(logLevel, msg, fileName, lineNum, module, context.second);
	}
	else {
		staticLogContext.first(logLevel, msg, fileName, lineNum, module, staticLogContext.second);
	}
}
void LogUtils::_log(uint8_t logLevel, std::pair<LogCallB, void*> context, const char* fileName, int lineNum, const char* module, const std::string& msg) {
	_log(logLevel, context, fileName, lineNum, module, msg.c_str());
}


void LogUtils::activateLogTarget(LogCallB callB, void* userData) {
	staticLogContext = {callB, userData};
}
