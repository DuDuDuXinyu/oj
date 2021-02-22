#pragma once

#include <iostream>
#include <vector>
#include "ctemplate/template.h"
#include "oj_model.hpp"

using namespace std;

class OjView
{
  public:
    static void DrawAllQuestions(vector<Question>& questions, string* html)
    {
      //1、创建template字典
      ctemplate::TemplateDictionary dict("all_questions");

      //2、遍历vector，遍历vector就相当于遍历多个试题，每一个试题构造一个子字典
      for (const auto& ques : questions)
      {
        //TemplateDictionary* AddSectionDictionary(const TemplateString section_name);
        ctemplate::TemplateDictionary* sub_dict = dict.AddSectionDictionary("question");
        //void SetValue(const TemplateString variable, const TemplateString value);
        /*
         *    variable: 指定的是在预定义的html当中的变量名称
         *    value: 替换的量 
         * */
        sub_dict->SetValue("id", ques._id);
        sub_dict->SetValue("id", ques._id);
        sub_dict->SetValue("title", ques._title);
        sub_dict->SetValue("star", ques._star);
      }

      //3、填充
      ctemplate::Template* t1 = ctemplate::Template::GetTemplate("./template/all_questions.html", ctemplate::DO_NOT_STRIP);
      //渲染
      t1->Expand(html, &dict);
    }

    static void DrawOneQuestion(const Question& ques, string* html)
    {
      ctemplate::TemplateDictionary dict("single_question");
      dict.SetValue("id", ques._id);
      dict.SetValue("title", ques._title);
      dict.SetValue("star", ques._star);
      dict.SetValue("desc", ques._desc);
      dict.SetValue("id", ques._id);
      dict.SetValue("code", ques._header_cpp);
  
      //3、填充
      ctemplate::Template* t1 = ctemplate::Template::GetTemplate("./template/single_question.html", ctemplate::DO_NOT_STRIP);
      
      //渲染
      t1->Expand(html, &dict);
    } 

    static void DrawCaseResult(const string& errorno, const string& q_result,const string& reason, string* html)
    {
      ctemplate::TemplateDictionary dict("single_question");
      dict.SetValue("errorno", errorno);
      dict.SetValue("compile_result", reason);
      dict.SetValue("case_result", q_result);

      //3、填充
      ctemplate::Template* t1 = ctemplate::Template::GetTemplate("./template/case_result.html", ctemplate::DO_NOT_STRIP);
      
      //渲染
      t1->Expand(html, &dict);
    }

};
