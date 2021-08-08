/**
 * @file python_integration.hpp
 * @author René Pascal Becker (OneDenper@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <string>
#include <vector>

// TODO: Make this clean

namespace condalf
{
    bool initialize_python(std::string file);
    void python_process_data(const std::vector<uint8_t> &data);
    void uninitialize_python();
}