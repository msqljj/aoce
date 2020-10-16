#pragma once
#include <assert.h>

#include <functional>
#include <string>
#include <vector>

#include "Aoce.h"

// #include "Module/IModule.hpp"
// #include "VideoDevice/VideoManager.hpp"
// #include "PipeGraph/PipeGraph.hpp"

// 如果列表的顺序无关,可以尝试如下三种的快速删除,不需要move数据
template <typename T>
void removeItem(std::vector<T>& vect, const T& t) {
    if (vect.empty()) {
        return;
    }
    auto size = vect.size();
    if (vect[size - 1] == t) {
        vect.pop_back();
        return;
    }
    auto iter = std::find(vect.begin(), vect.end(), t);
    if (iter != vect.end()) {
        *iter = vect[size - 1];
        vect.pop_back();
    }
};

template <typename T>
void removeItem(std::vector<T>& vect, std::function<bool(T t)> pred) {
    if (vect.empty()) {
        return;
    }
    auto iter = std::find_if(vect.begin(), vect.end(), pred);
    if (iter != vect.end()) {
        *iter = vect[vect.size() - 1];
        vect.pop_back();
    }
};

template <typename T>
void removeItem(std::vector<T>& vect, typename std::vector<T>::iterator iter) {
    if (vect.empty()) {
        return;
    }
    if (iter != vect.end()) {
        *iter = vect[vect.size() - 1];
        vect.pop_back();
    }
};

// https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
template <typename... Args>
std::string string_format(const std::string& format, Args... args) {
    size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1;
    if (size <= 0) {
        return "";
    }
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(),
                       buf.get() + size - 1);  // We don't want the '\0' inside
}

ACOE_EXPORT void logMessage(aoce::LogLevel level, const std::string& message);

ACOE_EXPORT std::wstring utf8TWstring(const std::string& str);
ACOE_EXPORT std::string utf8TString(const std::wstring& str);

ACOE_EXPORT void copywcharstr(wchar_t* dest, const wchar_t* source,
                              int32_t maxlength);

ACOE_EXPORT void copycharstr(char* dest, const char* source, int32_t maxlength);

ACOE_EXPORT uint32_t divUp(int32_t x, int32_t y);

ACOE_EXPORT aoce::ImageType videoType2ImageType(
    const aoce::VideoType& videoType);

ACOE_EXPORT bool bYuv(const aoce::VideoType& videoType);

ACOE_EXPORT aoce::ImageFormat videoFormat2ImageFormat(
    const aoce::VideoFormat& videoFormat);

ACOE_EXPORT int32_t getImageTypeSize(const aoce::ImageType& imageType);