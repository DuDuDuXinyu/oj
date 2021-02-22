#include "Base64.h"

#include <iostream>
using namespace std;

#include "httplib.h"

#include "json/json.h"

#pragma comment(lib,".\\Debug\\json_vc71_libmtd.lib")

void TextJson()
{
	Json::Value value;

	value["name"] = "peter";
	value["gender"] = "ÄÐ";
	value["age"] = 18;

	int scores[] = { 70,80,90 };

	for (auto e : scores)
	{
		value["score"].append(e);
	}

	Json::StyledWriter sw;
	string strJsonData = sw.write(value);
	cout << strJsonData << endl;

	Json::FastWriter fw;
	strJsonData = fw.write(value);
	cout << strJsonData << endl;

	Json::Reader rd;
	Json::Value rdValue;
	rd.parse(strJsonData, rdValue);
	cout << rdValue["name"] << endl;
	cout << rdValue["gender"] << endl;
	cout << rdValue["age"] << endl;

	for (auto e : rdValue["score"])
		cout << e;
}

void helloworld(const httplib::Request& req, httplib::Response& res)
{
	res.set_content("<html><head><title>hello</title></head><body><h2>helloworld</h2></body></html>", "text/html");
	res.status = 200;
}

void Str_to_Base64(const httplib::Request& req, httplib::Response& res)
{
	Json::Value value;
	Json::Reader rd;
	if (!rd.parse(req.body, value))
	{
		res.set_content("", "text/html");
		res.status = 500;
		return;
	}
	
	Base64 base64;
	string strRst = base64.Code(value["strtextdata"].asString());

	Json::StyledWriter sw;
	Json::Value wvalue;
	wvalue["base64Data"] = strRst;
	strRst = sw.write(wvalue);

	res.set_content(strRst, "text/html");
	res.status = 200;
}

void Base64_to_Str(const httplib::Request& req, httplib::Response& res)
{
	Json::Value value;
	Json::Reader rd;
	if (!rd.parse(req.body, value))
	{
		res.set_content("", "text/html");
		res.status = 500;
		return;
	}

	Base64 base64;
	string strRst = base64.DeCode(value["base64textdata"].asString());

	Json::StyledWriter sw;
	Json::Value wvalue;
	wvalue["strtext"] = strRst;
	strRst = sw.write(wvalue);

	res.set_content(strRst, "text/html");
	res.status = 200;
}

void Pic_to_Base64(const httplib::Request& req, httplib::Response& res)
{
	auto picFile = req.files;
	auto formData = picFile.find("strPicData")->second;
	auto picData = formData.content;

	Base64 base64;
	string strRst;
	strRst += "<img src='data:img/jpg;base64,";
	strRst += base64.Code(picData);
	strRst += "'/>";
	
	Json::Value value;
	value["picbase64Data"] = strRst;

	Json::StyledWriter sw;
	strRst = sw.write(value);

	res.set_content(strRst, "text/html");
	res.status = 200;
}

int main()
{
	httplib::Server s;
	s.set_base_dir(".\\Debug");
	//s.Get("/", helloworld);

	s.Post("/str_to_base64", Str_to_Base64);
	s.Post("/base64_to_str", Base64_to_Str);
	s.Post("/pic_to_base64", Pic_to_Base64);

	s.listen("127.0.0.1", 9000);

	return 0;
}