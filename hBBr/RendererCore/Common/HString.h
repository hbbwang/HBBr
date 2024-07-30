//---------------------------------------
//ClassName:	HString
//Date:			2020.6.23
//Author:		SCcDaniel
//Explain:		Dream Engine string class.
//---------------------------------------
#pragma once

#ifndef HBBR_INLINE
#if __ANDROID__
#define HBBR_INLINE inline
#else
#define HBBR_INLINE __forceinline
#endif
#endif

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

//#include <comutil.h>  
#include <ostream>
#include <vector>
#include <iostream>
#include "glm/glm.hpp"

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

#include <string>
#include <algorithm>
#include <cwchar>
#include <locale>
#include <codecvt>
#include <clocale>
#include <cwchar>

//图方便,直接用SDL的函数吧，而且直接跨平台
#include <SDL3/SDL.h>
#include "ThirdParty/nlohmann/json.hpp"
class  HString
{
private:
	char* m_char = nullptr;
	wchar_t* m_wchar = nullptr;
	char* _str = nullptr;
	wchar_t* _wstr = nullptr;
	size_t length = 0;
public:
	/* 
		this function is clear string memroy,not clear characters.
		If want to clear string characters,please use empty().
	*/
	inline void clear()
	{
		ReleaseCache();
		if (_str != nullptr)
		{
			delete[] _str;
			_str = nullptr;
		}
		if (_wstr != nullptr)
		{
			delete[] _wstr;
			_wstr = nullptr;
		}
		length = 0;
	}

	inline void empty()
	{
		this->assign("");
	}

	inline bool IsEmpty()
	{
		return this->length <= 0;
	}

	//字符串初始化
	HString()
	{
		clear();
		this->_str = new char[1];
		this->_str[0] = '\0';
		this->length = 0;
	}
	HString(const char* str)
	{
		if (str == nullptr)
			return;
		clear();
		this->length = strlen(str);
		this->_str = new char[this->length + 1];
		strcpy_s(this->_str, this->length + 1, str);
		_str[length] = '\0';
	}
	HString(const char str)
	{
		clear();
		char strTemp[2] = { str , '\0' };
		this->length = strlen(strTemp);
		this->_str = new char[this->length + 1];
		strcpy_s(this->_str, this->length + 1, strTemp);
	}
	HString(const wchar_t* str)
	{
		if (str == nullptr)
			return;
		clear();
		const char* result = pws2s(str);
		this->length = strlen(result);
		this->_str = new char[this->length + 1];
		strcpy_s(this->_str, this->length + 1, result);
		_str[length] = '\0';
	}
	HString(const wchar_t str)
	{
		clear();
		wchar_t strTemp[2] = { str , L'\0' };
		const char* result = pws2s(strTemp);
		this->length = strlen(result);
		this->_str = new char[this->length + 1];
		strcpy_s(this->_str, this->length + 1, result);
	}
	HString(const HString& obj)
	{
		if (obj.Length() > 0)
		{
			clear();
			this->length = obj.Length();
			this->_str = new char[this->length + 1];
			strcpy_s(this->_str, this->length + 1, obj._str);
		}
		else
		{
			clear();
			this->_str = new char[1];
			this->_str[0] = '\0';
			this->length = 0;
		}
	}
	HString(const std::string str)
	{
		clear();
		this->length = str.size();
		this->_str = new char[str.size()+1];
		strcpy_s(this->_str, this->length + 1, str.c_str());
		this->_str[this->length] = '\0';
	}
	~HString()
	{
		clear();
	}

	void operator=(const HString& obj)
	{
		clear();
		this->length = obj.Length();
		this->_str = new char[this->length + 1];
		strcpy_s(this->_str, this->length + 1, obj._str);
		_str[length] = '\0';
	}

	void operator/(const HString& obj)
	{
		HString newAppend = GetSeparate() + obj;
		this->append(newAppend);
	}

	HString& operator/=(const HString& obj)
	{
		HString newAppend = GetSeparate() + obj;
		this->append(newAppend);
		return *this;
	}

	HString& operator+=(const HString& obj)
	{
		if (obj._str == nullptr && obj._wstr == nullptr)
			return *this;
		char* temp = new char[length + 1];
		strcpy_s(temp, length + 1, _str);//先把字符串复制到temp
		ReleaseCache();
		if (_str != nullptr)
		{
			delete[] _str;
			_str = nullptr;
		}
		//申请新的大小
		length = length + obj.length;
		_str = new char[length + 1];
		//复制回去
		strcpy_s(_str, strlen(temp) + 1, temp);
		//并且把新的字符串连接上去
		strcat_s(_str, length + 1, obj._str);
		//释放temp
		delete[] temp;
		temp = nullptr;
		length = strlen(_str);
		return *this;
	}

	friend HString operator+(const HString& obj, const HString& obj1)
	{
		if (obj1._str == nullptr || obj1._str == nullptr)
			return obj._str;
		size_t len = obj.length + obj1.length;
		char* temp = new char[len + 1];//新建字符数组
		strcpy_s(temp, obj.length + 1, obj._str);//复制第一个
		strcat_s(temp, len + 1, obj1._str);	//把新的字符串连接上去
		HString result(temp);//赋值到新的变量
		//释放temp
		delete[] temp;
		temp = nullptr;
		return result;
	}

	//下标
	char operator[](size_t index)
	{
		return _str[index];
	}

	//根据ascii码排序
	bool operator<(const HString& obj)const
	{
		if (obj._str == nullptr)
		{
			//DE_ASSERT(obj._str == nullptr,"HString Error");
			return false;
		}
		if (obj.length <= 0)
			return false;
		else if (this->length <= 0)
			return true;
		else
		{
			if (strcmp(_str, obj.c_strC()) < 0)
			{
				return true;
			}
		}
		return false;
	}

	bool operator>(const HString& obj)const
	{
		if (obj.length <= 0)
			return false;
		else if (this->length <= 0)
			return true;
		else
		{
			if (strcmp(_str, obj.c_strC()) > 0)
			{
				return true;
			}
		}
		return false;
	}

	bool operator==(const HString& c) const
	{
		return strcmp(this->_str, c._str) == 0;
	}

	bool operator!=(const HString& c) const
	{
		return strcmp(this->_str, c._str) != 0;
	}

	HBBR_INLINE HString Left(size_t index)
	{
		if (index >= this->length || index <= 0)
		{
			return *this;
		}
		else
		{
			this->Remove(index, this->length);
		}
		return *this;
	}

	HBBR_INLINE HString Right(size_t index)
	{
		if (index >= this->length || index <= 0)
		{
			return *this;
		}
		else
		{
			this->Remove(0, index);
		}
		return *this;
	}

	//根据下标删除某段字符串
	void	Remove(size_t begin, size_t strLength)
	{
		if (strLength < begin)
			return;
		else if (strLength > length)
			strLength = length;
		std::string resultStr = _str;
		resultStr.erase(resultStr.begin() + begin , resultStr.begin() + strLength);
		this->assign(resultStr.c_str());
	}

	//根据某段字符串删除
	void Remove(const char* removeStr)
	{
		size_t newLen = strlen(removeStr);
		if (newLen <= 0)
			return;
		std::string resultStr = _str;
		std::string::size_type beginPos = resultStr.find(removeStr);
		if(beginPos != std::string::npos)
		{
			resultStr.erase(resultStr.begin() + beginPos , resultStr.begin() + beginPos + newLen);
		}
		this->assign(resultStr.c_str());
	}

	HBBR_INLINE void assign(const char* str)
	{
		clear();
		length = strlen(str);
		_str = new char[length + 1];//预留最后一个'/0'空字符的位置
		strcpy_s(_str, length + 1, str);
		_str[length] = '\0';
	}

	HBBR_INLINE void assign(const wchar_t* str)
	{
		if (str == nullptr)
			return;
		clear();
		const char* result = pws2s(str);
		this->length = strlen(result);
		this->_str = new char[this->length + 1];
		strcpy_s(this->_str, this->length + 1, result);
		_str[length] = '\0';
	}

	HBBR_INLINE void assign(HString str)
	{
		clear();
		length = str.length;
		_str = new char[length + 1];//预留最后一个'/0'空字符的位置
		strcpy_s(_str, length + 1, str._str);
		_str[length] = '\0';
	}

	template<typename ...Arg>
	HBBR_INLINE static HString printf(HString in, Arg...args)
	{
		char* formattedString = nullptr;
		SDL_asprintf(&formattedString, in.c_str(), args...);
		return HString(formattedString);
	}

	/* char* length!!! not wchar_t*  */
	HBBR_INLINE const size_t	Length()const { return length; }

	/* wchar_t* length */
	HBBR_INLINE const size_t	WLength() { return wcslen(ps2ws(_str)); }

	HBBR_INLINE const wchar_t GetWChar(size_t i) { return ps2ws(_str)[i]; }

	HBBR_INLINE const char* c_str() { return _str; }

	HBBR_INLINE const wchar_t* c_wstr() { 
		return ps2ws(_str); 
	}

	HBBR_INLINE const char* c_strC()const { return _str; }

	inline void append(const char* str)
	{
		size_t len = length + strlen(str);
		char* temp = new char[len + 1];
		strcpy_s(temp, length + 1, _str);
		strcat_s(temp, len + 1, str);
		clear();
		_str = temp;
		length = strlen(_str);
		_str[length] = '\0';
	}

	HBBR_INLINE void append(const wchar_t* str)
	{
		pws2s(str);
		this->append(m_char);
	}

	inline void append(HString& str)
	{
		size_t len = length + str.length;
		char* temp = new char[len + 1];
		strcpy_s(temp, length + 1, _str);
		strcat_s(temp, len + 1, str._str);
		HString result(temp);
		this->assign(result);
		//释放temp
		delete[] temp;
		temp = nullptr;
	}

	//判断字符是否相同，第二个参数作用 是否区分大小
	inline bool IsSame(HString c, bool strict = true) const
	{
#ifdef _WIN32
		if (this == nullptr)
			return false;
#endif
		if (strict == true)
		{
			if (this->length == c.length)
			{
				if (strcmp(_str, c.c_str()) == 0)
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
			if (strcasecmp(_str, c.c_str()) == 0)
			{
				return true;
			}
		}
		return false;
	}

	inline std::vector<HString> Split(const char* pattern)
	{
		if(strstr(this->_str , pattern) == nullptr)
		{
			return{ this->_str };
		}
		size_t patternLeng = strlen(pattern);
		size_t charLen = strlen(this->_str); 
		std::vector<HString> resultVec;
		if (patternLeng > charLen)
			return { this->_str };
		//const char* convert to char*
		char* strc = new char[charLen + 1];
		strcpy_s(strc, charLen + 1, this->_str);
		strc[charLen] = '\0';
		char* temp = nullptr;
		char* tmpStr = strtok_s(strc, pattern, &temp);
		while (tmpStr != nullptr)
		{
			const size_t vecSize = resultVec.size();
			if (vecSize >= resultVec.capacity())
			{
				resultVec.reserve(vecSize + 5);
			}
			resultVec.push_back(tmpStr);
			tmpStr = strtok_s(nullptr, pattern, &temp);
		}
		delete[] strc;
		strc = nullptr;
		return resultVec;
	}

	//获取文件名+后缀
	inline HString GetFileName()
	{
		size_t index = 0;
		//找到最后一个 '\'或者'/'
		for (size_t i = length - 1; i > 0; i--)
		{
			if (_str[i] == '/' || _str[i] == '\\')
			{
				break;
			}
			index++;
		}
		HString out = *this;
		out.Remove(0, out.length - index);
		return out;
	}

	//获取文件名
	HString GetBaseName()
	{
		size_t index = 0;
		HString out = *this;
		bool bFound = false;
		//找到最后一个 '\'或者'/'
		for (size_t i = length - 1; i > 0; i--)
		{
			if (_str[i] == '/' || _str[i] == '\\')
			{
				bFound = true;
				break;
			}
			index++;
		}
		if(bFound)
			out.Remove(0, out.length - index);
		//找到最后一个 '.'
		index = 0;
		bFound = false;
		for (size_t i = out.length - 1; i > 0; i--)
		{
			if (out[i] == '.')
			{
				bFound = true;
				break;
			}
			index++;
		}
		if (bFound)
			out.Remove(out.length - index - 1 , out.length);
		return out;
	}

	//返回一个没有空格或者Tab的字符串
	inline HString ClearSpace()
	{
		if (length > 0)
		{
			std::string s = _str;
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
		return _str;
	}

	/* 获取文件后缀 */
	inline HString GetSuffix()
	{
#if 0

#else
		size_t index = 0 ;
		bool bFound = false;
		HString out = *this;
		//找到最后一个 '.'
		const auto lenStr = strlen(_str);
		if (lenStr <= 0)
		{
			return out;
		}
		for (size_t i = lenStr - 1; i > 0; i--)
		{
			if (out[i] == '.')
			{
				bFound = true;
				break;
			}
			index++;
		}
		if (bFound)//查找到再删，查不到，就代表没有。
			out.Remove(0, lenStr - index);
		return out;
#endif
	}

	/*  获取文件路径 */
	inline HString GetFilePath()
	{
		size_t index = 0;
		bool bFound = false;
		auto len = strlen(_str);
		//找到最后一个 '\'或者'/'
		for (size_t i = len - 1 ; i > 0; i--)
		{
			if (_str[i] == '/' || _str[i] == '\\')
			{
				bFound = true;
				break;
			}
			index++;
		}
		HString out = *this;
		if (bFound)
			out.Remove(len - index, len);
		out.CorrectionPath();
		return out;
	}

	/*  路径斜杠格式纠正,目前window和Linux都支持“/”，不过window还是用“\\”吧 */
	HBBR_INLINE  void  CorrectionPath()
	{
#ifdef WIN32
		this->Replace("/", "\\");
#else
		this->Replace("\\", "/");
#endif
	}

	/* 转换成纯字符串形式的路径 */
	HBBR_INLINE void ToPathString()
	{
#ifdef WIN32
		this->Replace("\\", "\\\\");
#else
		//this->Replace("/", "/");
#endif
	}

	HBBR_INLINE static HString GetSeparate()
	{
#ifdef WIN32
		return "\\";
#else
		return "/";
#endif
	}

	/*
		替换字符，原理是先分离，再插入
	*/
	inline void Replace(const char* whatStr, const char* newStr)
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
		std::string str = this->_str;
		std::string old_str = whatStr;
		std::string new_str = newStr;
		size_t pos = 0;
		while ((pos = str.find(old_str, pos)) != std::string::npos) {
			str.replace(pos, old_str.length(), new_str);
			pos += new_str.length();
		}
		this->assign(str.c_str());
	}

	/* 字符串包含 */
	HBBR_INLINE bool Contains(HString whatStr, bool strict = true)
	{
		std::string str1(_str);
		std::string str2(whatStr.c_str());
		if (!strict)
		{
			// 将两个字符串转换为小写
			std::transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
			std::transform(str2.begin(), str2.end(), str2.begin(), ::tolower);
		}
		return str1.find(str2) != std::string::npos;
	}

	HBBR_INLINE char* ToStr()
	{
		return const_cast<char*>(c_str());
	}

	static HBBR_INLINE HString	FromFloat(double f, int precise = 6)
	{
		HString out;
		HString format = "%.";
		format += HString::FromInt(precise) + "f";
		char str[128];
		sprintf_s(str, 128, format.c_str(), f);
		out = str;
		return out;
	}

	static HBBR_INLINE HString	FromVec2(glm::vec2 f)
	{
		HString out;
		char str[128];
		sprintf_s(str, 128, "%f,%f", f.x, f.y);
		out = str;
		return out;
	}

	static HBBR_INLINE HString	FromVec3(glm::vec3 f)
	{
		HString out;
		char str[128];
		sprintf_s(str, 128, "%f,%f,%f" , f.x, f.y, f.z);
		out = str;
		return out;
	}

	static HBBR_INLINE HString	FromVec4(glm::vec4 f)
	{
		HString out;
		char str[128];
		sprintf_s(str, 128, "%f,%f,%f,%f", f.x, f.y, f.z, f.w);
		out = str;
		return out;
	}

	static HBBR_INLINE HString	FromInt(int i)
	{
		HString out;
		char str[128];
		sprintf_s(str, 128, "%d", i);
		out = str;
		return out;
	}

	static HBBR_INLINE HString	FromUInt(unsigned int i)
	{
		HString out;
		char str[128];
		sprintf_s(str, 128, "%u", i);
		out = str;
		return out;
	}

	static HBBR_INLINE HString	FromSize_t(size_t i)
	{
		HString out;
		char str[256];
		sprintf_s(str, 256, "%zu", i);
		out = str;
		return out;
	}

	static HBBR_INLINE HString	FromBool(bool i)
	{
		return i == true ? "true": "false";
	}

	template<class T>
	static HBBR_INLINE HString ToString(T value)
	{
		if (typeid(value) == typeid(float) || typeid(value) == typeid(double))
		{
			return HString::FromFloat(value);
		}
		else if (typeid(value) == typeid(int))
		{
			return HString::FromInt(value);
		}
		else if (typeid(value) == typeid(HString))
		{
			return value;
		}
		else if (typeid(value) == typeid(unsigned int))
		{
			return HString::FromUInt(value);
		}
		else if (typeid(value) == typeid(bool))
		{
			return HString::FromBool(value);
		}
		else if (typeid(value) == typeid(glm::vec2))
		{
			return HString::FromVec2(value);
		}
		else if (typeid(value) == typeid(size_t))
		{
			return HString::FromSize_t(value);
		}
		else
		{
			return value;
		}
	}

	static HBBR_INLINE bool ToBool(const char* str)
	{
		return  str[0]== 't' || str[0] == 'T' || atoi(str) > 0 ? true : false;
	}

	static HBBR_INLINE bool ToBool(HString str)
	{
		return  str.IsSame("true",false) ? true : false;
	}

	static HBBR_INLINE int ToInt(const char* str)
	{
		return atoi(str);
	}

	static HBBR_INLINE int ToInt(HString str)
	{
		return atoi(str.c_str());
	}

	static HBBR_INLINE unsigned long ToULong(const char* str)
	{
		char* end;
		return std::strtoul(str, &end, 10);
	}

	static HBBR_INLINE unsigned long ToULong(HString str)
	{
		char* end;
		return std::strtoul(str.c_str(), &end, 10);
	}

	static HBBR_INLINE double ToDouble(HString str)
	{
		return atof(str.c_str());
	}

	static HBBR_INLINE double ToDouble(const char* str)
	{
		return atof(str);
	}

	static HBBR_INLINE long ToLong(HString str)
	{
		return atol(str.c_str());
	}

	static HBBR_INLINE long ToLong(const char* str)
	{
		return atol(str);
	}

	static HBBR_INLINE long long ToLongLong(const char* str)
	{
		return atoll(str);
	}

	static HBBR_INLINE long long ToLongLong(HString str)
	{
		return atoll(str.c_str());
	}

	static HBBR_INLINE glm::vec2 ToVec2(HString str)
	{
		auto vecStr = str.Split(",");
		if (vecStr.size() == 2)
			return glm::vec2(atof(vecStr[0].c_str()), atof(vecStr[1].c_str()));
		else if (vecStr.size() == 1)
			return glm::vec2(atof(vecStr[0].c_str()), 0);
		return glm::vec2(0, 0);
	}

	static HBBR_INLINE glm::vec3 ToVec3(HString str)
	{
		auto vecStr = str.Split(",");
		if (vecStr.size() == 3)
			return glm::vec3(atof(vecStr[0].c_str()), atof(vecStr[1].c_str()), atof(vecStr[2].c_str()));
		else if (vecStr.size() == 2)
			return glm::vec3(atof(vecStr[0].c_str()), atof(vecStr[1].c_str()), 0);
		else if (vecStr.size() == 1)
			return glm::vec3(atof(vecStr[0].c_str()), 0, 0);
		return glm::vec3(0,0,0);
	}

	static HBBR_INLINE glm::vec4 ToVec4(HString str)
	{
		auto vecStr = str.Split(",");
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

	static HBBR_INLINE const bool IsNumber(HString str)
	{
		std::string stdstr = str.c_str();
		return std::all_of(stdstr.begin(), stdstr.end(), ::isdigit);
	}

	HBBR_INLINE const bool IsNumber()
	{
		std::string stdstr = _str;
		return std::all_of(stdstr.begin(), stdstr.end(), ::isdigit);
	}

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
	wchar_t* ps2ws(const char* src)
	{
		std::setlocale(LC_ALL, ""); // 设置本地化
		size_t wstr_size = std::mbsrtowcs(nullptr, &src, 0, nullptr);
		if (wstr_size == static_cast<size_t>(-1)) {
			//throw std::runtime_error("Invalid multibyte character sequence.");
			SDL_Log("Invalid multibyte character sequence.");
		}
		m_wchar = new wchar_t[wstr_size + 1];
		std::mbsrtowcs(m_wchar, &src, wstr_size + 1, nullptr);
		m_wchar[wstr_size] = L'\0';
		return m_wchar;
	}

	// 将 wchar_t* 转换为 char*
	char* pws2s(const wchar_t* src) {
		std::setlocale(LC_ALL, ""); // 设置本地化

		size_t str_size = std::wcsrtombs(nullptr, &src, 0, nullptr);
		if (str_size == static_cast<size_t>(-1)) {
			//throw std::runtime_error("Invalid wide character sequence.");
			SDL_Log("Invalid wide character sequence.");
		}
		m_char = new char[str_size + 1];
		std::wcsrtombs(m_char, &src, str_size + 1, nullptr);
		m_char[str_size] = L'\0';
		return m_char;
	}

#endif
	//std
	std::string ToStdString()const
	{
		return std::string(_str);
	}

private:

	//Cache
	void ReleaseCache()
	{
		if (m_char != nullptr)
		{
			delete[] m_char;
			m_char = nullptr;
		}
		if (m_wchar != nullptr)
		{
			delete[] m_wchar;
			m_wchar = nullptr;
		}
	}
};

//hash
namespace std
{
	struct EqualKey
	{
		bool operator() (const HString& s, const HString& q) const noexcept
		{
			return s==q ;
		}
	}; 
	template<>
	struct hash<HString>
	{
		size_t operator() (const HString& s) const noexcept
		{
			return  hash<std::string>()(s.c_strC());
		}
	};
}