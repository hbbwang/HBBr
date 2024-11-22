#pragma once

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

//#include <comutil.h>  
#include <ostream>
#include <vector>
#include <iostream>
#include "glm/glm.hpp"

//图方便,直接用SDL的函数吧，而且直接跨平台
#include <SDL3/SDL.h>
#include "ThirdParty/nlohmann/json.hpp"

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
	wchar_t* cs2ws(const char* src)
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
	char* ws2cs(const wchar_t* src) 
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
	std::wstring str2wstr(std::string str)
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
	std::string wstr2str(std::wstring str) 
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

};
