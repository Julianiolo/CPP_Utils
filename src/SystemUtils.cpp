#include "SystemUtils.h"

#include <functional>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <fstream>
#include <cmath>
#include <string>

#include "StringUtils.h"
#include "DataUtils.h"

#ifdef _WIN32
inline std::wstring toWstr(const char* str) {
    const size_t str_len = std::strlen(str);
    if (str_len > INT_MAX)
        throw std::runtime_error("toWstr failed: input str too long");

    const int size = MultiByteToWideChar(CP_UTF8, 0, str, (int)str_len, NULL, 0);
    if (size == 0)
        throw std::runtime_error(std::string("toWstr failed: ") + SystemUtils::getErrorCodeMsg(GetLastError()));

    std::wstring wstr;
    wstr.resize(size);

    if (MultiByteToWideChar(CP_UTF8, 0, str, (int)str_len, &wstr[0], size) == 0)
        throw std::runtime_error(std::string("toWstr failed: ") + SystemUtils::getErrorCodeMsg(GetLastError()));

    return wstr;
}
#endif


bool SystemUtils::revealInFileExplorer(const char* path_) {
	// wtf this is way too hard, currently we can just open the folder, at least on windows we should be able to select the file

	std::string path = path_;

#ifdef __CYGWIN__ // cygwin hack to fix a "/cygdrive/c/..." to a "c:/..."
	{
		const char* searchFor = "/cygdrive/";
		const size_t searchForLen = strlen(searchFor);
		if(path.size() >= searchForLen+2 && strncmp(path.c_str(), searchFor, searchForLen) == 0) {
			const char driveLetter = path[searchForLen];
			path = std::string() + driveLetter + ':' + '/' + path.substr(searchForLen+2);
		}
	}
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
	std::replace(path.begin(), path.end(), '/', '\\');
#else
	std::replace(path.begin(), path.end(), '\\', '/');
#endif

	std::string target = StringUtils::getDirName(path.c_str());

	std::string cmd;
#if defined(_WIN32) || defined(__CYGWIN__)
	cmd = StringUtils::format("explorer \"%s\"", target.c_str());
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	cmd = StringUtils::format("xdg-open '%s'", target.c_str()); // Alternatives: firefox, x-www-browser
#endif
#if defined(__APPLE__)
	cmd = StringUtils::format("open '%s'", target.c_str());
#endif
	if (cmd.size() > 0) {
		printf(">%s\n", cmd.c_str());
        auto wstr = toWstr(cmd.c_str());
		return _wsystem(wstr.c_str()) != -1;
	}
	return false;
#ifdef _WIN32
	// https://learn.microsoft.com/de-de/windows/win32/api/shlobj_core/nf-shlobj_core-shopenfolderandselectitems?redirectedfrom=MSDN
#else

#endif
}

std::string SystemUtils::getErrorCodeMsg(int errorCode) {
	std::string errMsg;
#if defined(_WIN32)
	// https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	errMsg = std::string(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);
#else
	errMsg = strerror(errorCode);
#endif

	return errMsg;
}

int SystemUtils::getErrorCode() {
#ifdef _WIN32
	return GetLastError();
#else
	return errno;
#endif
}

double SystemUtils::winTimeToTimestamp(uint64_t val) {
	if (val == 0) return NAN;
	return (double)val / 1e7 - 11644473600LL;
}

// ######################

SystemUtils::CallProcThread::CallProcThread(const std::string& cmd) : cmd(cmd), f(nullptr) {

}

SystemUtils::CallProcThread::~CallProcThread() {
    if(f != nullptr) {
        pclose(f);
    }
    thread.join();
}

bool SystemUtils::CallProcThread::start() {
    f = popen(cmd.c_str(),"r");

    if(!f)
        return false;

    thread = std::thread(std::bind(&SystemUtils::CallProcThread::update, this));

    return true;
}

std::string SystemUtils::CallProcThread::get(){
    while(!mutex.try_lock());
    std::string s = std::move(out);
    out = "";
    mutex.unlock();
    return s;
}

bool SystemUtils::CallProcThread::isRunning() {
    return f != nullptr;
}
bool SystemUtils::CallProcThread::hasData() {
	std::unique_lock lock(mutex);
    return out.size() > 0;
}

void SystemUtils::CallProcThread::update() {
    char buf[2048];
    while(std::fgets(buf, sizeof(buf), f) != nullptr){
		std::unique_lock lock(mutex);
        out += buf;
    }
    pclose(f);
    f = nullptr;
}

// ###############################################################
