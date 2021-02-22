#include <iostream>
#include <cstdio>
#include "compile.hpp"
#include "tools.hpp"
#include "json/json.h" 
#include"oj_model.hpp"
#include "httplib.h"
#include "oj_view.hpp"
using namespace httplib;
using namespace std;

int main()
{
  //1、初始化httplib库的server对象
  Server svr;
  Oj_Model model;

  //2、提供三个接口，分别处理三个请求
  //2.1 获取整个实体列表，get
  svr.Get("/all_questions", [&model](const Request& req, Response& resp){
        //返回试题列表
        vector<Question> questions;
        model.GetAllQuestions(&questions);

        for(int i = 0; i < questions.size(); i++)
        {
          cout << questions[i]._id << "  "<< questions[i]._title << " "<< questions[i]._star 
          << "  "<< questions[i]._path << endl;
        }

        string html;
        OjView::DrawAllQuestions(questions, &html);
        
        resp.set_content(html, "text/html");
      });

  //2.2 获取单个试题
  //如何标识浏览器想要获取的是哪一个试题？？?
  //   \d: 表示一个数字
  //   \d+: 表示多位数字
  svr.Get(R"(/single_question/(\d+))", [&model](const Request& req, Response& resp){
      //1、获取url当中关于试题的数字
      //cout << req.matches[0] << " " << req.matches[1] << endl;
      
      Question ques;
      model.GetOneQuestion(req.matches[1].str(), &ques);

      //2、渲染模板html
      string html;
      OjView::DrawOneQuestion(ques, &html);      

      resp.set_content(html, "text/html");
      });

  //2.3 编译运行
  // 区分提交的哪一个试题？？？
  svr.Post(R"(/compile/(\d+))", [&model](const Request& req, Response& resp){
      //1、获取试题编号
      Question ques;
      model.GetOneQuestion(req.matches[1].str(), &ques);
      //ques._tail_cpp -->main函数调用+测试用例
      
      //post 方法在提交代码的时候，是经过encode编码的
      //于是要获取到浏览器提交的代码，需要进行decode解码
      //使用decode完成的代码和tail.cpp进行合并， 产生待编译的源码
      cout << req.body << endl;
      cout << UrlUtil::UrlDecode(req.body) << endl; 

      unordered_map<string,string> body_kv;
      UrlUtil::PraseBody(req.body, &body_kv);

      cout << body_kv["code"] << endl;
      
      //构造json对象，交给编译运行模块
      Json::Value req_json;
      req_json["code"] = body_kv["code"] + ques._tail_cpp;
      cout << req_json["code"].asString() <<endl;
  
      Json::Value resp_json;
      Compiler::CompileAndRun(req_json, &resp_json);

      //获取的返回结果都在resp_json中
      string errorno = resp_json["errorno"].asString();
      string case_result = resp_json["stdout"].asString();
      string reason = resp_json["reason"].asString();

      string html;
      OjView::DrawCaseResult(errorno, case_result, reason, &html);
      
      resp.set_content(html, "text/html");

      });

  Log(INFO, __FILE__, __LINE__, "17878");

  svr.set_base_dir("./www");
  svr.listen("0.0.0.0", 17878);
  return 0;
}
