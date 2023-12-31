#ifndef MY_EXCEPTION_HEADER
#define MY_EXCEPTION_HEADER

#include <sstream>
#include <string>
#include <string_view>

class MyException {
public:
    MyException(const char* func, const char* file, int line)
        : line_(line), function_(func), file_(file){};

    MyException(const char* func, const char* file, int line, const char* reason)
        : line_(line), function_(func), file_(file), reason_(reason){};

    MyException(const char* func, const char* file, int line, const std::string& reason)
        : line_(line), function_(func), file_(file), reason_(std::move(reason)){};

    int line_;
    const char* function_;
    const char* file_;
    std::string reason_;

    std::stringstream what_;

    std::string what() {
        what_ << "file: ";
        std::string_view file_sv(file_);
#ifdef WIN32
        size_t index = file_sv.find_last_of(R"(\)") + 1;
#else
        size_t index = file_sv.find_last_of('/') + 1;
#endif
        what_ << &file_[index];
        what_ << " | line: ";
        what_ << std::to_string(line_);
        what_ << " | function: ";
        what_ << function_;
        what_ << "\n";
        what_ << reason_;
        what_ << "\n";
        return what_.str();
    }
};

#define MyEx(reason) MyException(__FUNCTION__, __FILE__, __LINE__, reason)

#endif // my_exception.hh