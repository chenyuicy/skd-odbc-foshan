#pragma once

#include <iostream>

#ifndef NDEBUG
#include <fstream>
#define LOG(message) \
    do { std::cerr << __FILE__ << ":" << __LINE__ << " " << message << std::endl; } while (false)
#define LOGQUERY(message)\
     do { std::ofstream log_file("D:\\log_file.txt", std::ios_base::out | std::ios_base::app); log_file << message << std::endl;log_file.close(); } while (false)

#else 
#include <fstream>
#define LOG(message)
#define LOGQUERY(message)\
     do { std::ofstream log_file("D:\\log_file.txt", std::ios_base::out | std::ios_base::app); log_file << message << std::endl; } while (false)
#endif
