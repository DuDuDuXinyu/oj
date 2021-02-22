#pragma once

#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/resource.h>
#include <atomic>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <json/json.h>
#include "tools.hpp"
using namespace std;

enum ErrorNo
{
  OK = 0,
  PRAM_ERROR,
  INTERNAL_ERROR,
  COMPILE_ERROR,
  RUN_ERROR
};

class Compiler
{
  public:
    /*
     *  Json::Value Req : 请求的json
     *  Json::Value* Resp : 出参，返回给调用者
     * */
    static void CompileAndRun(Json::Value Req, Json::Value* Resp)
    {
      //1、参数是否是错误的，json串中的code是否为空
      if (Req["code"].empty())
      {
        (*Resp)["errorno"] = PRAM_ERROR;
        (*Resp)["reason"] = "Ream error";
        return;
      }

      //2、将代码写到文件当中去
      string code = Req["code"].asString();
      //文件命名约定的时候为了区分不同的请求
      //使用 tmp_时间戳.id.cpp
      string file_nameheader = WriteTempFile(code);
      if (file_nameheader ==  "")
      {
        (*Resp)["errorno"] = INTERNAL_ERROR;
        (*Resp)["reason"] = "write file error";
        return;
      }

      //3、编译
     if (!Compile(file_nameheader))
     {
        (*Resp)["errorno"] = COMPILE_ERROR;
        string reason;
        FileUtil::ReadFile(CompileErrorPath(file_nameheader).c_str(), &reason);
        (*Resp)["reason"] = reason;
        return;
     }

      //4、运行
      if (int ret = Run(file_nameheader) != 0)
      {
        (*Resp)["errorno"] = RUN_ERROR;
        (*Resp)["reason"] = "program exit by sig:" + to_string(ret);
        return;
      }

      //5、构造响应
      (*Resp)["errorno"] = OK;
      (*Resp)["reason"] = "Compile and Run are ok";

      string stdout_str;
      FileUtil::ReadFile(StdoutPath(file_nameheader), &stdout_str);
      (*Resp)["stdout"] = stdout_str;

      string stderr_str;
      FileUtil::ReadFile(StderrPath(file_nameheader), &stderr_str);
      (*Resp)["stderr"] = stderr_str;

      //删除临时文件
      Clean(file_nameheader);
    }
  private:

    static void Clean(const string& filename)
    {
      unlink(SrcPath(filename).c_str());
      unlink(CompileErrorPath(filename).c_str());
      unlink(ExePath(filename).c_str());
      unlink(StdoutPath(filename).c_str());
      unlink(StderrPath(filename).c_str());
    }

    static int Run(const string& file_name)
    {
      //创建子进程
      //子进程程序替换
      int pid = fork();
      if (pid < 0)
        return -1;
      else if (pid > 0)
      {
        //father
        int status = 0;
        waitpid(pid, &status, 0);

        //终止信号
        return status & 0x7f;
      }
      else
      {
        //child
        
        //注册一个定时器，alarm
        alarm(1);

        //限制内存
        struct rlimit rlim;
        rlim.rlim_cur = 30000 * 1024; //3wkb
        rlim.rlim_max = RLIM_INFINITY;
        setrlimit(RLIMIT_AS, &rlim);

        //重定向
        int stdout_fd = open(StdoutPath(file_name).c_str(), O_CREAT | O_WRONLY, 0666);
        if (stdout_fd < 0)
          return -2;
        dup2(stdout_fd, 1);

        int stderr_fd = open(StderrPath(file_name).c_str(), O_CREAT | O_WRONLY, 0666);
        if (stderr_fd < 0)
          return -2;
        dup2(stderr_fd, 2);

        //进程替换
        execl(ExePath(file_name).c_str(), ExePath(file_name).c_str(), NULL);
        exit(0);
      }

      return 0;
    }

    static bool Compile(const string& file_name)
    {
      //1、创建子进程
      //  1.1、父进程进行进程等待
      //  1.2、子进程进行进程程序替换
      
      int pid = fork();
      if (pid > 0)
      {
        //father
        waitpid(pid, NULL, 0);
      }
      else if (pid == 0)
      {
        //child
        //进程程序替换--> g++ SrcPath(filename) -o ExePath(filename) "-std=c++11"
        int fd = open(CompileErrorPath(file_name).c_str(), O_CREAT | O_WRONLY, 0666);
        if (fd < 0)
          return false;

        //将标准错误（2）重定向为fd，标准错误的输出就会输出在文件当中
        dup2(fd, 2);

        execlp("g++", "g++", SrcPath(file_name).c_str(), "-o", ExePath(file_name).c_str(), "-std=c++11", "-D", "CompileOnline", NULL);

        //如果替换失败，子进程直接退出
        exit(0);
      }
      else
      {
        return false;
      }

      //判断g++是否执行成功
      struct stat st;
      int ret = stat(ExePath(file_name).c_str(), &st);
      if (ret < 0)
        return false;

      return true;
    }

    static string StdoutPath(const string& filename)
    {
      return "./tmp_file/" + filename + ".stdout";
    }
    
    static string StderrPath(const string& filename)
    {
      return "./tmp_file/" + filename + ".stderr";
    }
    
    static string CompileErrorPath(const string& filename)
    {
      return "./tmp_file/" + filename + ".Compileerror";
    }

    static string ExePath(const string& filename)
    {
      return "./tmp_file/" + filename + ".executable";
    }

    static string SrcPath(const string& filename)
    {
      return "./tmp_file/" + filename + ".cpp";
    }

    static string WriteTempFile(const string& code)
    {
      //1、组织文件名称，区分源码文件，以及后面生成的可执行程序文件
      static atomic_uint id(0);
      string tmp_filename = "tmp_" + to_string(TimeUtil::GetTimeStampMs()) + "." + to_string(id);
      id++;
      //2、code写到文件当中去
      FileUtil::WriteFile(SrcPath(tmp_filename), code);
      return tmp_filename;
    }
};
