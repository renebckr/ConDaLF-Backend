/**
 * @file parser.cpp
 * @author René Pascal Becker (OneDenper@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "parser.h"

#include <fstream>
#include <common/logging/logging.h>

void common::config::parse(std::string file, std::function<void(const std::string&)> line_handler)
{
    std::ifstream in;
    in.open(file);

    if (!in.is_open())
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Parser could not open file.");
        throw std::invalid_argument("File could not be opened");
    }

    std::string line;
    while (std::getline(in, line))
        line_handler(line);
    
    if (in.bad())
        common::logging::log_warning(std::cout, LINE_INFORMATION, "Parser encountered an unknown error during parsing.");

    in.close();
    return;
}