#include <fstream>
#include <streambuf>
#include <iostream>

#include <stdlib.h>
#include <time.h>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

const char* ss_conf = "/root/confs/shadowsocks/config.json";
//const char* ss_conf = "config.json";
const char chs[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789~!@#$%^&*()_+";
const int key_len = 8;
const char* cmd = "/usr/bin/supervisorctl restart shadowsocks";

int info()
{
    std::ifstream ifs(ss_conf);
    std::string str((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());

    using namespace rapidjson;
    Document doc;
    if (doc.Parse(str.c_str()).HasParseError())
        return -1;

    {
        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        doc.Accept(writer);
        std::cout << sb.GetString();
    }


    Value adoc(kObjectType);
    adoc.AddMember("server_port", doc["server_port"], doc.GetAllocator());
    adoc.AddMember("password", doc["password"], doc.GetAllocator());

    {
        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        adoc.Accept(writer);
        std::cout << sb.GetString();
    }

    return 0;
}

bool check_port(int port)
{
    return true;
}

int reset()
{
    std::ifstream ifs(ss_conf);
    std::string str((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());
    ifs.close();

    using namespace rapidjson;
    Document conf;
    if (conf.Parse(str.c_str()).HasParseError())
        return -1;

    srand(time(NULL));
    int port = 4500;
    do {
        port = rand() % 10000 + 500;
    } while (!check_port(port));

    std::string passwd;
    for (int i = 0; i < key_len; i++)
    {
        passwd.push_back(chs[rand() % sizeof(chs)]);
    }

    Document new_conf(kObjectType);
    new_conf.AddMember("server_port", port, new_conf.GetAllocator());
    new_conf.AddMember("password", Value(passwd.c_str(), new_conf.GetAllocator()), new_conf.GetAllocator());

    {
        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        new_conf.Accept(writer);
        std::cout << sb.GetString();
    }

    conf["server_port"] = new_conf["server_port"];
    conf["password"] = new_conf["password"];

    {
        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        conf.Accept(writer);

        std::ofstream ofs(ss_conf);
        ofs << sb.GetString();
    }

    system(cmd);

    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        return 1;
    }

    if (memcmp(argv[1], "info", 5) == 0) {
       return info();
    } else if (memcmp(argv[1], "reset", 6) == 0) {
       return reset();
    } else {
        return 2;
    }
    
    return 0;
}