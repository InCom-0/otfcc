#pragma once

#include <algorithm>
#include <memory>
#include <set>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
   #define NOMINMAX
#endif
#include <windows.h>
#elif defined(__linux__)
#include <linux/limits.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <sys/syslimits.h>
#endif

#include <nowide/cstdio.hpp>

#ifdef _WIN32
#include <nowide/convert.hpp>
#endif

class Translator {
  public:
	Translator() {
		auto langs = getLanguageList();
		auto appDir = getAppDir();

		for (auto lang : langs) {
			auto moPath = appDir + '/' + lang + ".mo";
			if (loadMoFile(moPath))
				return;
		}
	}
	Translator(const Translator &) = delete;
	Translator &operator=(const Translator &) = delete;

	const char *gettext(const char *orig) {
		if (translations.empty() || !orig)
			return orig;

		auto it =
		    std::lower_bound(translations.begin(), translations.end(), orig);

		if (it != translations.end() && it->orig == orig)
			return it->trans.begin();
		else
			return orig;
	}

  private:
	struct Translation {
		std::string_view orig;
		std::string_view trans;

		bool operator<(const Translation &rhs) const { return orig < rhs.orig; }

		bool operator<(const char *rhs) const { return orig < rhs; }
	};

  private:
	std::string moFile;
	std::vector<Translation> translations;

  private:
	struct MoHeader {
		uint32_t magic;
		uint32_t version;
		uint32_t nStrings;
		uint32_t offsetOrig;
		uint32_t offsetTrans;
		uint32_t sizeHash;
		uint32_t offSetHash;
	};

	struct MoStr {
		uint32_t length;
		uint32_t offset;

		bool valid(const std::string &moFile) const {
			return offset + length <= moFile.size();
		}

		std::string_view toStringView(const std::string &moFile) const {
			return {moFile.data() + offset, length};
		}
	};

	bool loadMoFile(const std::string &filename) {
		std::string moFile = loadFile(filename);
		if (moFile.size() < sizeof(MoHeader))
			return false;

		MoHeader *header = reinterpret_cast<MoHeader *>(moFile.data());
		if (header->magic != 0x950412de)
			return false;
		if (header->version != 0)
			return false;

		if (moFile.size() < header->offsetOrig + header->nStrings * 8)
			return false;
		MoStr *origs =
		    reinterpret_cast<MoStr *>(moFile.data() + header->offsetOrig);

		if (moFile.size() < header->offsetTrans + header->nStrings * 8)
			return false;
		MoStr *trans =
		    reinterpret_cast<MoStr *>(moFile.data() + header->offsetTrans);

		std::vector<Translation> translations;
		translations.reserve(header->nStrings);
		for (uint32_t i = 0; i < header->nStrings; i++) {
			if (!origs[i].valid(moFile) || !trans[i].valid(moFile))
				return false;

			translations.emplace_back(origs[i].toStringView(moFile),
			                          trans[i].toStringView(moFile));
		}

		std::sort(translations.begin(), translations.end());

		this->moFile.swap(moFile);
		this->translations.swap(translations);
		return true;
	}

	static std::string loadFile(const std::string &filename) {
		FILE *fp = nowide::fopen(filename.c_str(), "rb");
		if (!fp)
			return {};
		fseek(fp, 0, SEEK_END);
		size_t size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		std::string result(size, 0);
		fread(result.data(), 1, size, fp);
		fclose(fp);
		return result;
	}

	struct LangCode {
		std::string lang;
		std::string region;
		std::string script;

		std::string toGnu() const {
			std::string result = lang;
			if (!region.empty())
				result += "-" + region;
			if (!script.empty())
				result += "-" + script;
			return result;
		}

		std::vector<std::string> extendedGnu() const {
			std::vector<std::string> result;
			if (!script.empty()) {
				if (!region.empty())
					result.push_back(toGnu());         // 1. perfetch match
				result.push_back(lang + "@" + script); // 2. lang & script
			}
			if (!region.empty())
				result.push_back(lang + "_" + region); // 3. lang & region
			result.push_back(lang);                    // 4. lang
			return result;
		}

		static LangCode fromGnu(std::string langStr) {
			// remove encoding: zh_CN.UTF-8
			auto pos3 = langStr.find('.');
			if (pos3 != std::string::npos)
				langStr.erase(pos3);

			LangCode result;
			auto pos2 = langStr.find('@');
			if (pos2 != std::string::npos) {
				result.script = langStr.substr(pos2 + 1);
				langStr = langStr.substr(0, pos2);
			}
			auto pos1 = langStr.find('_');
			if (pos1 != std::string::npos) {
				result.region = langStr.substr(pos1 + 1);
				langStr = langStr.substr(0, pos1);
			}
			result.lang = langStr;
			return result;
		}

		static LangCode fromWin32(std::string langStr) {
			// ref.
			// https://learn.microsoft.com/en-us/windows/win32/intl/language-names

			// remove supplemental: en-US-x-fabricam
			if (auto pos = langStr.find("-x-"); pos != std::string::npos)
				langStr.erase(pos);

			LangCode result;
			auto pos1 = langStr.find("-");
			result.lang = langStr.substr(0, pos1);
			if (pos1 == std::string::npos)
				return result; // neutral

			langStr = langStr.substr(pos1 + 1);
			auto pos2 = langStr.find("-");
			if (pos2 != std::string::npos) {
				result.script = langStr.substr(0, pos2);
				result.script[0] = toupper(result.script[0]);
				result.region = langStr.substr(pos2 + 1);
			} else
				result.region = langStr;
			return result;
		}
	};

	static std::vector<std::string> getLanguageList() {
		std::vector<std::string> result;
		std::set<std::string> added;

		auto add = [&result, &added](const LangCode &langCode) {
			for (auto lang : langCode.extendedGnu())
				if (added.find(lang) == added.end()) {
					result.push_back(lang);
					added.insert(lang);
				}
		};

		// LANGUAGE=zh_CN:en_US
		if (const char *langStr = getenv("LANGUAGE"); langStr) {
			auto langs = split(langStr);
			for (auto lang : langs)
				add(LangCode::fromGnu(lang));
		}

		// LC_ALL=zh_CN.UTF-8
		if (const char *lang = getenv("LC_ALL"); lang)
			add(LangCode::fromGnu(lang));

		// LC_MESSAGES=zh_CN.UTF-8
		if (const char *lang = getenv("LC_MESSAGES"); lang)
			add(LangCode::fromGnu(lang));

		// LANG=zh_CN.UTF-8
		if (const char *lang = getenv("LANG"); lang)
			add(LangCode::fromGnu(lang));

#ifdef _WIN32
		for (auto lang : getWin32LanguageList())
			add(LangCode::fromWin32(lang));
#endif

		return result;
	}

	static std::vector<std::string> split(const std::string &str,
	                                      char sep = ':') {
		std::vector<std::string> result;
		size_t pos = 0;
		while (pos < str.size()) {
			size_t next = str.find(sep, pos);
			if (next == std::string::npos)
				next = str.size();
			size_t length = next - pos;
			if (length > 0)
				result.push_back(str.substr(pos, length));
			pos = next + 1;
		}
		return result;
	}

#ifdef _WIN32
	static std::vector<std::string> getWin32LanguageList() {
		static auto pGetUserPreferredUILanguages =
		    reinterpret_cast<decltype(&GetUserPreferredUILanguages)>(
		        GetProcAddress(GetModuleHandleW(L"kernel32.dll"),
		                       "GetUserPreferredUILanguages"));
		if (!pGetUserPreferredUILanguages)
			return {};

		constexpr int bufLen = 4096;
		wchar_t wbuf[bufLen];
		ULONG len = bufLen;
		ULONG numLangs;
		if (!pGetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numLangs, wbuf,
		                                  &len))
			return {};

		char buf[bufLen];
		for (int i = 0; i < len; i++)
			buf[i] = wbuf[i];

		std::vector<std::string> result;
		char *pos = buf;
		while (*pos) {
			std::string lang = std::string(pos);
			pos += lang.size() + 1;
			result.push_back(lang);
		}
		return result;
	}
#endif

	static std::string getAppDir() {
		// ref.
		// https://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
#if defined(_WIN32)
		wchar_t buf[MAX_PATH];
		if (DWORD len = GetModuleFileNameW(NULL, buf, MAX_PATH); len) {
			std::wstring res(buf, len);
			auto pos = res.find_last_of('\\');
			if (pos != std::wstring::npos)
				return nowide::narrow(res.substr(0, pos));
		}
#elif defined(__linux__)
		char buf[PATH_MAX];
		if (ssize_t len = readlink("/proc/self/exe", buf, PATH_MAX);
		    len != -1) {
			std::string res(buf, len);
			auto pos = res.find_last_of('/');
			if (pos != std::string::npos)
				return res.substr(0, pos);
		}
#elif defined(__APPLE__)
		char buf[PATH_MAX];
		uint32_t len = PATH_MAX;
		if (_NSGetExecutablePath(buf, &len) == 0) {
			std::string res(buf, len);
			auto pos = res.find_last_of('/');
			if (pos != std::string::npos)
				return res.substr(0, pos);
		}
#endif
		return {};
	}
};

inline std::unique_ptr<Translator> gTranslator = std::make_unique<Translator>();

inline const char *_(const char *origStr) {
	return gTranslator ? gTranslator->gettext(origStr) : origStr;
}
