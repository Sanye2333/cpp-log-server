#include "config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <windows.h>

// 声明服务端的宽字符输出函数
extern void print_wchar(const wchar_t *wstr);

ServerConfig load_config(const std::string &config_path)
{
    ServerConfig cfg;
    // 默认值
    cfg.server_ip = "0.0.0.0";
    cfg.server_port = 11451;
    cfg.log_dir = "./logs/";
    cfg.max_file_size = 100;
    cfg.log_keep_days = 7;
    cfg.max_clients = 1000;
    cfg.recv_buf_size = 4096;

    // 读取配置文件
    std::ifstream file(config_path);
    if (!file.is_open())
    {
        print_wchar(L"配置文件不存在，使用默认配置！\n");
        return cfg;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        size_t pos = line.find('=');
        if (pos == std::string::npos)
            continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));

        if (key == "server_ip")
            cfg.server_ip = value;
        else if (key == "server_port")
            cfg.server_port = std::stoi(value);
        else if (key == "log_dir")
            cfg.log_dir = value;
        else if (key == "max_file_size")
            cfg.max_file_size = std::stoi(value);
        else if (key == "log_keep_days")
            cfg.log_keep_days = std::stoi(value);
        else if (key == "max_clients")
            cfg.max_clients = std::stoi(value);
        else if (key == "recv_buf_size")
            cfg.recv_buf_size = std::stoi(value);
    }
    file.close();
    return cfg;
}