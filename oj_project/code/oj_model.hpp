#pragma once


#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include "tools.hpp"

using namespace std;

struct Question
{
  string _id; //题目id
  string _title;  //题目名称
  string _star; //题目的难易程度
  string _path; //题目路径

  string _desc; //题目的描述
  string _header_cpp;  //题目预定义的头
  string _tail_cpp;  //题目的尾，包含测试用例以及调用逻辑 
};

class Oj_Model
{
  public:
    Oj_Model()
    {
      Load("./oj_data/oj_config.cfg");
    }

    ~Oj_Model()
    {

    }

    //从文件当中获取题目信息
    bool Load(const string& filename)
    {
      ifstream file(filename.c_str());
      if (!file.is_open())
      {
        cout << "config file open failed" << endl;
        return false;
      }
      //文件打开成功
      //1、从文件当中获取每一行信息
      //  1.1 对于每一行信息，还需要获取题号，题目名称，题目难易程度，题目路径
      //  1.2 保存在结构体当中
      //2、把多个question组织在map当中
      
      string line;
      while(getline(file, line))
      {
        //boost::spilt
        vector<string> vec;
        StringUtil::Split(line, " ", &vec);
        //is_any_of:支持多个字符作为分隔符
        //token_compress_off:是否将多个分割字符看作是一个
        //boost::split(vec, line, boost::is_any_of(" "), boost::token_compress_off);

        Question ques;
        ques._id = vec[0];
        ques._title = vec[1];
        ques._star = vec[2];
        ques._path = vec[3];

        string dir = vec[3];
        FileUtil::ReadFile(dir + "/desc.txt", &ques._desc);
        FileUtil::ReadFile(dir + "/header.cpp", &ques._header_cpp);
        FileUtil::ReadFile(dir + "/tail.cpp", &ques._tail_cpp);

        _question_map[ques._id] = ques;
      }
      file.close();
      return true;
    }

    //提供给上层调用者一个获取所有试题的接口
    bool GetAllQuestions(vector<Question>* questions)
    {
      //1、遍历无序的map，将试题信息返回给调用者
      //对于每一个试题，都是一个Question结构体对象
      for (const auto& kv : _question_map)
        questions->push_back(kv.second);
      //排序
      sort(questions->begin(), questions->end(), [](const Question& l, const Question r){
          //比较Question中的题目编号，按照升序规则
          return stoi(l._id) < stoi(r._id);
          });
    }

    //提供给上层调用者一个获取单个试题的接口
    /*
     *    id: 输入条件，查找题目的id
     *    ques: 输出参数，将查到的结果返回给调用者
     * */
    bool GetOneQuestion(const string& id, Question* ques)
    {
      auto it = _question_map.find(id);
      if (it == _question_map.end())
      {
        return false;
      }
      *ques = it->second;
      return true;
    }
    
  private:
    unordered_map<string, Question> _question_map;
};
