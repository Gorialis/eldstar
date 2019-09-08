#pragma once

#include <exception>
#include <stdexcept>

#ifdef _WIN32

#include <windows.h>
#include <winuser.h>

#elif __linux__

#include <iostream>

#endif


namespace eldstar {

class load_failure : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;

        void report() {
            #ifdef _WIN32

            std::string text = "Eldstar has run into a fatal load error and cannot continue:\n\n" + std::string(what());

            MessageBoxA(
                NULL, text.c_str(), "Load failure",
                MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_TASKMODAL
            );

            #elif __linux__

            std::cerr << "Eldstar has run into a fatal load error and cannot continue:\n" << what() << std::endl;

            #endif
        }
};

class socket_timeout : public std::runtime_error
{
    public:
        socket_timeout() : std::runtime_error("Socket timed out") {};
};

class not_implemented : public std::logic_error
{
    public:
        not_implemented() : std::logic_error("A non-implemented function was used (erroneous call to base class?)") {};
};

} // eldstar
