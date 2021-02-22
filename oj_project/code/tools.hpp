#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <sys/time.h>
#include <boost/algorithm/string.hpp>

using namespace std;

class FileUtil
{
  public:
    //读文件接口
    //file_name:文件名称
    //content:读到的内容，输出参数，返回给调用者
    static bool ReadFile(const string& file_name, string* content)
    {
      //1、清空content当中的内容
      content->clear();
      ifstream file(file_name.c_str());
      if (!file.is_open())
        return false;

      //文件被打开
      string line;
      while(getline(file, line))
        (*content) += line + "\n";
      file.close();
      return true;
    }
    
    static bool WriteFile(const string& filename, const string data)
    {
      ofstream file(filename.c_str());
      if (!file.is_open())
        return false;

      file.write(data.c_str(), data.size());
      file.close();
      return true;
    }
};

class StringUtil
{
  public:
    static void Split(const string& input, const string& split_char, vector<string>* output)
    {
      boost::split(*output, input, boost::is_any_of(split_char), boost::token_compress_off);
    }
};

class UrlUtil
{
public:

	//body从httplib.h当中的request类的成员变量获得
	//  body:
	//     key=value&key1=value1   ===> 并且是进行过编码
	//  1.先切割
	//  2.在对切割之后的结果进行转码
	static void PraseBody(const std::string& body, std::unordered_map<std::string, std::string>* body_kv)
	{
		std::vector<std::string> kv_vec;
		StringUtil::Split(body, "&", &kv_vec);

		for (const auto& t : kv_vec)
		{
			std::vector<std::string> sig_kv;
			StringUtil::Split(t, "=", &sig_kv);

			if (sig_kv.size() != 2)
			{
				continue;
			}

			//在保存的时候， 针对value在进行转码
			(*body_kv)[sig_kv[0]] = UrlDecode(sig_kv[1]);
		}
	}

	static unsigned char ToHex(unsigned char x)
	{
		return  x > 9 ? x + 55 : x + 48;
	}

	static unsigned char FromHex(unsigned char x)
	{
		unsigned char y;
		if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
		else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
		else if (x >= '0' && x <= '9') y = x - '0';
		else assert(0);
		return y;
	}

	static std::string UrlEncode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (isalnum((unsigned char)str[i]) ||
				(str[i] == '-') ||
				(str[i] == '_') ||
				(str[i] == '.') ||
				(str[i] == '~'))
				strTemp += str[i];
			else if (str[i] == ' ')
				strTemp += "+";
			else
			{
				strTemp += '%';
				strTemp += ToHex((unsigned char)str[i] >> 4);
				strTemp += ToHex((unsigned char)str[i] % 16);
			}
		}
		return strTemp;
	}

	static std::string UrlDecode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (str[i] == '+') strTemp += ' ';
			else if (str[i] == '%')
			{
				assert(i + 2 < length);
				unsigned char high = FromHex((unsigned char)str[++i]);
				unsigned char low = FromHex((unsigned char)str[++i]);
				strTemp += high * 16 + low;
			}
			else strTemp += str[i];
		}
		return strTemp;
	}
};

//获取时间戳
class TimeUtil
{
  public:
	  static int64_t GetTimeStampMs()
	  {
		  struct timeval tv;
		  gettimeofday(&tv, NULL);
		  return tv.tv_sec + tv.tv_usec / 1000;
	  }

    //年月日，时分秒
    static void GetTimeStamp(string* TimeStamp)
    {
      time_t st;
      time(&st);

      struct tm* t = localtime(&st);
      
      char buf[30] = { 0 };
      snprintf(buf, sizeof(buf) - 1, "%04d-%02d-%02d %02d:%02d:%02d",t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
      TimeStamp->assign(buf, strlen(buf));
    }

};

enum LogLevel
{
  INFO = 0,
  WARNING,
  ERROR,
  FATAL,
  DEBUG
};

const char* Level[]=
{
  "INFO",
  "WARNING",
  "ERROR",
  "FATAL",
  "DEBUG"
};

/*
 *    lev: 日志等级
 *    file: 文件名称
 *    line: 行号
 *    logmsg: 想要记录的日志内容
 * */
inline ostream& Log(LogLevel lev, const char* file, int line, const string& logmsg)
{
  string level_info = Level[lev];
  string TimeStamp;
  TimeUtil::GetTimeStamp(&TimeStamp);

  cout << "[" << TimeStamp << " " << level_info << " " << file << ":" << line << "]" << " " << logmsg << endl;
  return cout;
}

#define LOG(lev, msg) Log(lev, __FILE__, __LINE__, msg)
