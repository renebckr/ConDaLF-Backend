/**
 * @file parser.h
 * @author René Pascal Becker (OneDenper@gmail.com)
 * @brief Basic Config Parser
 * @version 0.1
 * @date 2021-06-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <string>
#include <functional>

namespace common::config
{
    /**
     * @brief Reads one line and passes it to the line_handler
     * 
     * @param file Filepath
     * @param line_handler Handler that handles every read line
     */
    void parse(std::string file, std::function<void(const std::string&)> line_handler);
}
