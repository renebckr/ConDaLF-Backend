/**
 * @file logging.h
 * @author René Pascal Becker (OneDenper@gmail.com)
 * @brief Basic logging functionality
 * @version 0.1
 * @date 2021-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <iostream>
#include <locale>
#include <codecvt>
#include <string>

// TODO: Print to file as well -> we do not need to use some cheesy hacks

namespace common::logging
{
#define LINE_INFORMATION std::string(__FILE__) + std::string(":") + std::to_string(__LINE__) + std::string("\n\t") + std::string(__PRETTY_FUNCTION__) + std::string(":\n")

    using info = std::string;
    using out = std::ostream;

    void log(out &stream, info type, info line_info /* LINE_INFORMATION */, info msg);

    static void log_error(out &stream, info line_info, info msg) { log(stream, "error", line_info, msg); }

    static void log_information(out &stream, info line_info, info msg) { log(stream, "info", line_info, msg); }

    static void log_warning(out &stream, info line_info, info msg) { log(stream, "warning", line_info, msg); }

} // namespace logging