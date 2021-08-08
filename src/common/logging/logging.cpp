#include "logging.h"

using namespace common;

void _replace_all(logging::info &str, const logging::info &find_str, const logging::info &replace_str)
{
    if (find_str.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(find_str, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, find_str.length(), replace_str);
        start_pos += replace_str.length();
    }
}

void logging::log(out &stream, info type, info line_info /* LINE_INFORMATION */, info msg)
{
    if (!msg.empty())
    {
        if (msg.back() == '\n')
            msg.pop_back();
    }

    info w_type(type.begin(), type.end());
    info w_line_info(line_info.begin(), line_info.end());

    info type_str = info("[") + w_type + info("] ");
    info str = w_line_info + msg;

    _replace_all(str, info("\n"), info("\n") + type_str);

    stream << type_str << str << "\n-\n";
}