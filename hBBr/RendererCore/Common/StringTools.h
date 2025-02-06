#pragma once

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
//#define FMT_HEADER_ONLY

//#include <comutil.h>  
#include <ostream>
#include <vector>
#include <iostream>
#include "glm/glm.hpp"
#include <sstream>
#include <string>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <clocale>
#include <cwchar>
#include <utf/ww898/cp_utf8.hpp>

using namespace ww898;

//图方便,直接用SDL的函数吧，而且直接跨平台
#include <SDL3/SDL.h>
#include "ThirdParty/nlohmann/json.hpp"


#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "comsuppw.lib")
#define strcasecmp(str1, str2) _stricmp(str1, str2)
#pragma warning(disable : _STL_DISABLE_DEPRECATED_WARNING)
#else
#include <unistd.h>
#define strcpy_s(dst,size ,src) strlcpy(dst,src,size)
#define sprintf_s(buffer,buffer_size,format , ...) snprintf(buffer, buffer_size, format, __VA_ARGS__)
#define strcat_s(dest, dest_size ,src) strlcat(dest, src, dest_size)
#define strtok_s(str, delimiters, context) strtok_r(str, delimiters, context) 
#endif

class StringTool
{
public:

	//#ifdef _WIN32
#if 0
	//char 到 wchar_t的 转换
	wchar_t* ps2ws(const char* s)
	{
		//ReleaseCache();
		//size_t len = MultiByteToWideChar(CP_ACP, 0, s, (int)strlen(s), nullptr, 0);
		//m_wchar = new wchar_t[len + 1];
		//MultiByteToWideChar(CP_ACP, 0, s, (int)strlen(s), m_wchar, (int)len);
		//m_wchar[len] = '\0';
		//return m_wchar;
		ReleaseCache();
		size_t nc = strlen(s);
		size_t n = (size_t)MultiByteToWideChar(CP_ACP, 0, (const char*)s, (int)nc, NULL, 0);
		m_wchar = new wchar_t[n + 1];
		MultiByteToWideChar(CP_ACP, 0, (const char*)s, (int)nc, m_wchar, (int)n);
		m_wchar[n] = '\0';
		return m_wchar;
	}

	//wchar_t 到 char的 转换
	char* pws2s(const wchar_t* ws)
	{
		ReleaseCache();
		size_t len = WideCharToMultiByte(CP_ACP, 0, ws, (int)wcslen(ws), nullptr, 0, nullptr, nullptr);
		m_char = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, ws, (int)wcslen(ws), m_char, (int)len, nullptr, nullptr);
		m_char[len] = '\0';
		return m_char;
	}

#else
	//char 到 wchar_t的 转换 ,需要自行释放内存
	static HBBR_INLINE wchar_t* cs2ws(const char* src)
	{
		std::setlocale(LC_ALL, ""); // 设置本地化
		size_t wstr_size = std::mbsrtowcs(nullptr, &src, 0, nullptr);
		if (wstr_size == static_cast<size_t>(-1)) {
			//throw std::runtime_error("Invalid multibyte character sequence.");
			SDL_Log("Invalid multibyte character sequence.");
		}
		wchar_t* m_wchar = new wchar_t[wstr_size + 1];
		std::mbsrtowcs(m_wchar, &src, wstr_size + 1, nullptr);
		m_wchar[wstr_size] = L'\0';
		return m_wchar;
	}

	// 将 wchar_t* 转换为 char*  ,需要自行释放内存
	static HBBR_INLINE char* ws2cs(const wchar_t* src)
	{
		std::setlocale(LC_ALL, ""); // 设置本地化

		size_t str_size = std::wcsrtombs(nullptr, &src, 0, nullptr);
		if (str_size == static_cast<size_t>(-1)) {
			//throw std::runtime_error("Invalid wide character sequence.");
			SDL_Log("Invalid wide character sequence.");
		}
		char* m_char = new char[str_size + 1];
		std::wcsrtombs(m_char, &src, str_size + 1, nullptr);
		m_char[str_size] = L'\0';
		return m_char;
	}

	//char 到 wchar_t的 转换 ,需要自行释放内存
	static HBBR_INLINE std::wstring str2wstr(std::string str)
	{
		std::setlocale(LC_ALL, ""); // 设置本地化
		const char* src = str.c_str();
		size_t wstr_size = std::mbsrtowcs(nullptr, &src, 0, nullptr);
		if (wstr_size == static_cast<size_t>(-1)) {
			//throw std::runtime_error("Invalid multibyte character sequence.");
			SDL_Log("Invalid multibyte character sequence.");
		}
		wchar_t* m_wchar = new wchar_t[wstr_size + 1];
		std::mbsrtowcs(m_wchar, &src, wstr_size + 1, nullptr);
		m_wchar[wstr_size] = L'\0';
		std::wstring result = m_wchar;
		delete[]m_wchar;
		return result;
	}

	// 将 wchar_t* 转换为 char*  ,需要自行释放内存
	static HBBR_INLINE std::string wstr2str(std::wstring str)
	{
		std::setlocale(LC_ALL, ""); // 设置本地化
		const wchar_t* src = str.c_str();
		size_t str_size = std::wcsrtombs(nullptr, &src, 0, nullptr);
		if (str_size == static_cast<size_t>(-1)) {
			//throw std::runtime_error("Invalid wide character sequence.");
			SDL_Log("Invalid wide character sequence.");
		}
		char* m_char = new char[str_size + 1];
		std::wcsrtombs(m_char, &src, str_size + 1, nullptr);
		m_char[str_size] = L'\0';
		std::string result = m_char;
		delete[]m_char;
		return result;
	}

#endif

	// 定义一个多平台兼容的宽字符字符串比较函数
	static int wcsicmp(const wchar_t* s1, const wchar_t* s2) {
		#if defined(_WIN32) || defined(_WIN64)
		return _wcsicmp(s1, s2);
		#else
		return wcscasecmp(s1, s2);
		#endif
	}

	template<typename... Args>
	static HBBR_INLINE std::string format(std::format_string<Args...> fmt, Args&&... args)
	{
		return std::format(fmt, std::forward<Args>(args)...);
	}

	template<typename ...Arg>
	static HBBR_INLINE std::string vformat(const char* in, Arg...args)
	{
		return std::vformat(in, std::make_format_args(args...));
	}

	HBBR_INLINE static size_t GetStrLen(std::string& str)
	{
		return SDL_utf8strlen(str.c_str());
	}

	HBBR_INLINE static size_t GetStrByteLen(std::string& str)
	{
		return str.size();
	}

	static HBBR_INLINE std::vector<std::string> split(const std::string& str, const char* delim) {
		std::vector<std::string> tokens;
		size_t start = 0;
		size_t end = str.find(delim);

		while (end != std::string::npos) {
			tokens.push_back(str.substr(start, end - start));
			start = end + 1;
			end = str.find(delim, start);
		}

		auto endStr = str.substr(start);
		if(endStr.length()>0)
		{
			tokens.push_back(endStr);
		}
		
		return tokens;
	}

	static HBBR_INLINE bool IsEqual(const std::string str0 , const std::string str1, bool strict = true)
	{
		if (strict == true)
		{
			if (str0.length() == str1.length())
			{
				if (SDL_strcmp(str0.c_str(), str1.c_str()) == 0)
				{
					return true;
				}
				else
					return false;
			}
			else
				return false;
		}
		else
		{
			if (stricmp(str0.c_str(), str1.c_str()) == 0)
			{
				return true;
			}
		}
		return false;
	}

	static HBBR_INLINE double ToDouble(const char* str)
	{
		return std::atof(str);
	}

	static HBBR_INLINE double ToDouble(const std::string str)
	{
		return std::stod(str.c_str());
	}

	static HBBR_INLINE float ToFloat(const std::string str)
	{
		return std::stof(str.c_str());
	}

	static HBBR_INLINE int ToInt(const char* str)
	{
		return atoi(str);
	}

	static HBBR_INLINE int ToInt(const std::string str)
	{
		return std::stoi(str.c_str());
	}

	static HBBR_INLINE long ToLong(const char* str)
	{
		return atol(str);
	}

	static HBBR_INLINE long ToLong(const std::string str)
	{
		return std::stol(str.c_str());
	}

	static HBBR_INLINE long long ToLongLong(const char* str)
	{
		return atoll(str);
	}

	static HBBR_INLINE long long ToLongLong(const std::string str)
	{
		return std::stoll(str.c_str());
	}

	static HBBR_INLINE unsigned long ToULong(const char* str)
	{
		return std::stoul(str);
	}

	static HBBR_INLINE unsigned long ToULong(const std::string str)
	{
		return std::stoul(str);
	}

	static HBBR_INLINE unsigned long long ToULongLong(const char* str)
	{
		return std::stoull(str);
	}

	static HBBR_INLINE unsigned long long ToULongLong(const std::string str)
	{
		return std::stoull(str);
	}

	static HBBR_INLINE bool ToBool(const char* str)
	{
		return  str[0] == 't' || str[0] == 'T' || atoi(str) > 0 ? true : false;
	}

	static HBBR_INLINE bool ToBool(const std::string str)
	{
		return  str[0] == 't' || str[0] == 'T' || std::stoi(str) > 0 ? true : false;
	}

	//
	static HBBR_INLINE std::string	FromUInt(unsigned int i)
	{
		std::string result = format("{}", i);
		return result;
	}

	static HBBR_INLINE std::string	FromInt(int i)
	{
		std::string result = format("{}", i);
		return result;
	}

	static HBBR_INLINE std::string	FromSize_t(size_t i)
	{
		std::string result = format("{}", i);
		return result;
	}

	static HBBR_INLINE std::string	FromBool(bool i)
	{
		return i == true ? "true" : "false";
	}

	static HBBR_INLINE std::string	FromFloat(float f, int precise = 6)
	{
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(precise) << f;
		return oss.str();
	}

	static HBBR_INLINE std::string	FromDouble(double f, int precise = 8)
	{
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(precise) << f;
		return oss.str();
	}

	static HBBR_INLINE std::string	FromVec2(glm::vec2 f)
	{
		std::string out;
		char str[128];
		sprintf_s(str, 128, "%f,%f", f.x, f.y);
		out = str;
		return out;
	}

	static HBBR_INLINE std::string	FromVec3(glm::vec3 f)
	{
		std::string out;
		char str[128];
		sprintf_s(str, 128, "%f,%f,%f", f.x, f.y, f.z);
		out = str;
		return out;
	}

	static HBBR_INLINE std::string	FromVec4(glm::vec4 f)
	{
		std::string out;
		char str[128];
		sprintf_s(str, 128, "%f,%f,%f,%f", f.x, f.y, f.z, f.w);
		out = str;
		return out;
	}

	/* 字符串包含 */
	static HBBR_INLINE bool Contains(std::string str, std::string whatStr, bool strict = true)
	{
		if (!strict)
		{
			// 将两个字符串转换为小写
			std::transform(str.begin(), str.end(), str.begin(), ::tolower);
			std::transform(whatStr.begin(), whatStr.end(), whatStr.begin(), ::tolower);
		}
		return str.find(whatStr) != std::string::npos;
	}

	//根据下标删除某段字符串
	static void	Remove(std::string& str, size_t begin, size_t strLength)
	{
		if (strLength < begin)
			return;
		else if (strLength > GetStrLen(str))
			strLength = GetStrLen(str);
		str.erase(str.begin() + begin, str.begin() + strLength);
	}

	//根据某段字符串删除
	static void Remove(std::string& str, const std::string removeStr)
	{
		size_t newLen = removeStr.length();
		if (newLen <= 0)
			return;
		std::string::size_type beginPos = str.find(removeStr);
		if (beginPos != std::string::npos)
		{
			str.erase(str.begin() + beginPos, str.begin() + beginPos + newLen);
		}
	}

	HBBR_INLINE static std::string GetSeparate()
	{
		#ifdef WIN32
				return "\\";
		#else
				return "/";
		#endif
	}

	//返回一个没有空格或者Tab的字符串
	HBBR_INLINE static std::string ClearSpace(std::string& str)
	{
		if (str.size() > 0)
		{
			std::string s = str;
			size_t index = 0;
			while ((index = s.find(' ', index)) != std::string::npos)
			{
				s.erase(index, 1);
			}
			index = 0;
			while ((index = s.find('\t', index)) != std::string::npos)
			{
				s.erase(index, 1);
			}
			return s.c_str();
		}
		return str;
	}

	HBBR_INLINE static std::string Left(std::string& str, size_t index)
	{
		auto strLen = GetStrLen(str);
		if (index >= strLen || index <= 0)
		{
			return str;
		}
		else
		{
			Remove(str, index, strLen);
		}
		return str;
	}

	HBBR_INLINE static std::string Right(std::string& str, size_t index)
	{
		auto strLen = GetStrLen(str);
		if (index >= strLen || index <= 0)
		{
			return str;
		}
		else
		{
			Remove(str, 0, index);
		}
		return str;
	}

	/*
		替换字符
	*/
	inline static void Replace(std::string& str, std::string whatStr, std::string newStr)
	{
		//std::vector<HString>split = this->Split(whatStr);
		//if (split.size() > 0)
		//{
		//	this->assign(split[0]);
		//	for (size_t i = 1; i < split.size(); i++)
		//	{
		//		this->append(newStr);
		//		this->append(split[i]);
		//	}
		//}
		std::string old_str = whatStr;
		std::string new_str = newStr;
		size_t pos = 0;
		while ((pos = str.find(old_str, pos)) != std::string::npos) {
			str.replace(pos, old_str.length(), new_str);
			pos += new_str.length();
		}
	}


	static HBBR_INLINE glm::vec2 ToVec2(std::string str)
	{
		auto vecStr = split(str, ",");
		if (vecStr.size() == 2)
			return glm::vec2(atof(vecStr[0].c_str()), atof(vecStr[1].c_str()));
		else if (vecStr.size() == 1)
			return glm::vec2(atof(vecStr[0].c_str()), 0);
		return glm::vec2(0, 0);
	}

	static HBBR_INLINE glm::vec3 ToVec3(std::string str)
	{
		auto vecStr = split(str, ",");
		if (vecStr.size() == 3)
			return glm::vec3(atof(vecStr[0].c_str()), atof(vecStr[1].c_str()), atof(vecStr[2].c_str()));
		else if (vecStr.size() == 2)
			return glm::vec3(atof(vecStr[0].c_str()), atof(vecStr[1].c_str()), 0);
		else if (vecStr.size() == 1)
			return glm::vec3(atof(vecStr[0].c_str()), 0, 0);
		return glm::vec3(0, 0, 0);
	}

	static HBBR_INLINE glm::vec4 ToVec4(std::string str)
	{
		auto vecStr = split(str, ",");
		if (vecStr.size() == 4)
			return glm::vec4(atof(vecStr[0].c_str()), atof(vecStr[1].c_str()), atof(vecStr[2].c_str()), atof(vecStr[3].c_str()));
		if (vecStr.size() == 3)
			return glm::vec4(atof(vecStr[0].c_str()), atof(vecStr[1].c_str()), atof(vecStr[2].c_str()), 0);
		else if (vecStr.size() == 2)
			return glm::vec4(atof(vecStr[0].c_str()), atof(vecStr[1].c_str()), 0, 0);
		else if (vecStr.size() == 1)
			return glm::vec4(atof(vecStr[0].c_str()), 0, 0, 0);
		return glm::vec4(0, 0, 0, 0);
	}

	static HBBR_INLINE const bool IsNumber(const char* str)
	{
		std::string stdstr = str;
		return std::all_of(stdstr.begin(), stdstr.end(), ::isdigit);
	}

	static HBBR_INLINE const bool IsNumber(std::string str)
	{
		std::string stdstr = str.c_str();
		return std::all_of(stdstr.begin(), stdstr.end(), ::isdigit);
	}

};
