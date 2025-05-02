#include "FileUtils.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <fstream>

#ifdef _WIN32
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

#include "DataUtils.h"
#include "StringUtils.h"
#include "SystemUtils.h"

bool FileUtils::checkHardlinkedTogether(const char* pathA, const char* pathB) {
#ifdef _WIN32
	constexpr auto getFileID = [](const char* path, FILE_ID_INFO* id) {
		HANDLE f_handle = CreateFileA(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
		if (f_handle == INVALID_HANDLE_VALUE) {
			return GetLastError();
		}else{
			if (GetFileInformationByHandleEx(f_handle, FileIdInfo, id, sizeof(*id)) == 0) {
				return GetLastError();
			}
			CloseHandle(f_handle);
		}
		return (DWORD)0;
	};

	DWORD res;

	FILE_ID_INFO fileInfoA;
	if ((res = getFileID(pathA, &fileInfoA)) != 0) throw std::runtime_error(SystemUtils::getErrorCodeMsg(res));
	FILE_ID_INFO fileInfoB;
	if ((res = getFileID(pathB, &fileInfoB)) != 0) throw std::runtime_error(SystemUtils::getErrorCodeMsg(res));

	return (fileInfoA.VolumeSerialNumber == fileInfoB.VolumeSerialNumber) && std::memcmp(fileInfoA.FileId.Identifier, fileInfoB.FileId.Identifier, sizeof(fileInfoA.FileId.Identifier)) == 0;
#elif defined(__unix__) || defined(__APPLE__)
	int res;

	struct stat stA;
	if((res = lstat(pathA, &stA)) != 0) throw std::runtime_error(strerror(res));
	struct stat stB;
	if((res = lstat(pathB, &stB)) != 0) throw std::runtime_error(strerror(res));

	return (stA.st_ino == stB.st_ino) && (stA.st_dev == stB.st_dev);
#endif
	abort();
}

char FileUtils::fileTypeModeToLetter(uint32_t mode) {
	// https://linux.die.net/man/2/stat
	// https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c

	switch (mode & S_IFMT) {
		// TODO: how does that correlate to EntType_DEV
#ifdef S_IFBLK
		case S_IFBLK:  return 'b';
#endif
		case S_IFCHR:  return 'c';

#ifdef S_IFIFO
		case S_IFIFO:  return 'p';
#endif
#ifdef S_IFSOCK
		case S_IFSOCK: return 's';
#endif
#ifdef S_IFDOOR
		case S_IFDOOR: return 'D';
#endif

		case S_IFDIR:  return 'd';
#ifdef S_IFLNK
		case S_IFLNK:  return 'l';
#endif
		case S_IFREG:  return '-';
		default:       return '?';
	}
}

void FileUtils::fileModeToStr(char* buf, uint32_t mode) {
	// https://linux.die.net/man/2/chmod
	// https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c

	constexpr uint32_t S_UID = 4 << 9, S_GID = 2 << 9, S_VTX = 1 << 9;
	constexpr uint32_t R_USR = 4 << 6, W_USR = 2 << 6, X_USR = 1 << 6;
	constexpr uint32_t R_GRP = 4 << 3, W_GRP = 2 << 3, X_GRP = 1 << 3;
	constexpr uint32_t R_OTH = 4 << 0, W_OTH = 2 << 0, X_OTH = 1 << 0;

	buf[0] = fileTypeModeToLetter(mode);

	buf[1] = (mode & R_USR) ? 'r' : '_';
	buf[2] = (mode & W_USR) ? 'w' : '_';
	if (!(mode & S_UID)) {
		buf[3] = (mode & X_USR) ? 'x' : '_';
	}
	else {
		buf[3] = (mode & X_USR) ? 's' : 'S';
	}

	buf[4] = (mode & R_GRP) ? 'r' : '_';
	buf[5] = (mode & W_GRP) ? 'w' : '_';
	if (!(mode & S_GID)) {
		buf[6] = (mode & X_GRP) ? 'x' : '_';
	}
	else {
		buf[6] = (mode & X_GRP) ? 's' : 'l';
	}

	buf[7] = (mode & R_OTH) ? 'r' : '_';
	buf[8] = (mode & W_OTH) ? 'w' : '_';
	if (!(mode & S_VTX)) {
		buf[9] = (mode & X_OTH) ? 'x' : '_';
	}
	else {
		buf[9] = (mode & X_OTH) ? 't' : 'T';
	}

	buf[10] = '\0';
}

FileUtils::CmpFileError::CmpFileError(ErrSource src, const std::ios_base::failure& e) : std::ios_base::failure(e), src_(src) {

}
FileUtils::CmpFileError::ErrSource FileUtils::CmpFileError::src() const {
	return src_;
}

std::optional<FileUtils::CmpFileResult> FileUtils::compareFiles(PathType path1, PathType path2, char* buf1, char* buf2, size_t bufSize, std::function<bool(uint64_t,uint64_t)> callB) {
	if (callB != NULL && callB((uint64_t)-1, (uint64_t)-1))
		return std::nullopt;

	DataUtils::AlignedBuffer buf;
	if (buf1 == NULL || buf2 == NULL || bufSize == 0) {
		if (bufSize == 0) {
			bufSize = (size_t)1 << 22;
		}

		constexpr size_t align_amt = 32; // way overkill lol

		buf = DataUtils::AlignedBuffer(bufSize*2, align_amt);

		buf1 = &(buf.get())[0];
		buf2 = &(buf.get())[bufSize];
	}

	if (callB != NULL && callB((uint64_t)-1, (uint64_t)-1))
		return std::nullopt;

	CmpFileError::ErrSource src = CmpFileError::ErrSource::A;
	CmpFileResult result;

	try {
		constexpr auto A = CmpFileError::ErrSource::A;
		constexpr auto B = CmpFileError::ErrSource::B;

		std::ifstream f1;
		std::ifstream f2;

		src = A;
		f1.exceptions(std::ios_base::failbit | std::ios_base::badbit);
		src = B;
		f2.exceptions(std::ios_base::failbit | std::ios_base::badbit);

		src = A;
		f1.open(path1, std::ios::binary);

		src = B;
		f2.open(path2, std::ios::binary);

		src = A;
		f1.seekg(0, std::ios::end);
		result.size_a = f1.tellg();
		f1.seekg(0, std::ios::beg);

		src = B;
		f2.seekg(0, std::ios::end);
		result.size_b = f2.tellg();
		f2.seekg(0, std::ios::beg);

		if (result.size_a != result.size_b)
			return result;

		const auto file_size = result.size_a;

		if (callB != NULL && callB(0, file_size))
			return std::nullopt;

		for (uint64_t o = 0; o < file_size; o+= bufSize) {
			size_t readAmt = (size_t)std::min(file_size - o, (uint64_t)bufSize);

			src = A;
			f1.read(buf1, readAmt);

			src = B;
			f2.read(buf2, readAmt);

			if (std::memcmp(buf1, buf2, readAmt) != 0) {
				auto res = std::mismatch(buf1, buf1+readAmt, buf2);
				result.cmp_offset = o + std::distance(buf1, res.first);
				return result;
			}

			if (callB != NULL && callB(o+readAmt, file_size))
				return std::nullopt;
		}
	}
	catch (const std::ios_base::failure& e) {
		throw CmpFileError(src, e);
	}

	result.are_same = true;

	return result;
}


FileUtils::MappedFile::MappedFile(const char* path) {

}
