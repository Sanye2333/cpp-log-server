#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <unordered_map>

struct ServerConfig
{
    std::string server_ip;
    int server_port;
    std::string log_dir;
    int max_file_size;
    int log_keep_days;
    int max_clients;
    int recv_buf_size;
};

// 加载配置文件
ServerConfig load_config(const std::string &config_path);

#endif
