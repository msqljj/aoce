#include "Aoce.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

#include "Aoce.hpp"
#include "Module/ModuleManager.hpp"
#if WIN32
#include <Shlwapi.h>
#include <Windows.h>

#include <iomanip>
#pragma comment(lib, "shlwapi.lib")
#elif __ANDROID__
#include <stdlib.h>
#endif

using namespace aoce;

static logEventHandle logHandle = nullptr;

void setLogAction(logEventAction action) { logHandle = action; }
void setLogHandle(logEventHandle action) { logHandle = action; }

void logMessage(AOCE_LOG_LEVEL level, const char* message) {
#if !DEBUG
    if (level == AOCE_LOG_DEBUG) {
        return;
    }
#endif
    if (logHandle != nullptr) {
        logHandle(level, message);
    } else if (message) {
#if WIN32
        auto now = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        struct tm t;
        localtime_s(&t, &now);
        // 用std::cout有可能会导致UE4烘陪失败,记录下
        std::wcout << std::put_time(&t, L"%Y-%m-%d %X") << " Level: " << level
                   << L" " << message << std::endl;
        // << "\" in " << __FILE__ << " at line " << __LINE__
#elif __ANDROID__
        switch (level) {
            case AOCE_LOG_INFO:
                LOGI(message, __FILE__, __LINE__);
                break;
            case AOCE_LOG_WARN:
                LOGW(message, __FILE__, __LINE__);
                break;
            case AOCE_LOG_ERROR:
                LOGE(message, __FILE__, __LINE__);
                break;
            case AOCE_LOG_DEBUG:
                LOGD(message, __FILE__, __LINE__);
                break;
            default:
                break;
        }
#endif
    }
}

void logMessage(aoce::LogLevel level, const std::string& message) {
    if (message.empty()) {
        return;
    }
    logMessage((AOCE_LOG_LEVEL)level, message.c_str());
}

void logAssert(bool expression, const std::string& message) {
    if (!expression) {
        logMessage(LogLevel::error, message);
    }
}

long long getNowTimeStamp() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto timeStamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    return timeStamp;
}

// ansi用char表示多字节编码(中国ansi对应GB2312),unicode(UTF-16)用wchar表示编码(多国家同一编码)
// uft8在unicode基础上,变长1-4Byte表示,用于传输与保存节约空间
// https://stackoverflow.com/questions/6693010/how-do-i-use-multibytetowidechar
// https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string
std::wstring utf8TWstring(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }
#if WIN32
    size_t len = str.length() + 1;
    std::wstring ret = std::wstring(len, 0);
    int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, &str[0],
                                   str.size(), &ret[0], len);
    ret.resize(size);
#else
    size_t len = str.length();
    std::vector<wchar_t> dest(len, 0);
    int dest_len = 0;
    for (int i = 0; i < len; i++, dest_len++) {
        // ansi
        if (str[i] <= 127) {
            dest[dest_len] = str[i];
        }
        // 2byte
        else if ((str[i] & 0xF0) == 0xC0) {
            dest[dest_len] = ((str[i] & 0x1F) << 6) + (str[i + 1] & 0x3F);
            i += 1;
        }
        // 3byte
        else if ((str[i] & 0xF0) == 0xE0) {
            dest[dest_len] = ((str[i] & 0x0F) << 12) +
                             ((str[i + 1] & 0x3F) << 6) + (str[i + 2] & 0x3F);
            i += 2;
        }
        // ignore 4byte
        else {
            logMessage(LogLevel::warn, "Can't change utf8 4byte characters");
            return L"";
        }
    }
    std::wstring ret;
    ret.assign(dest.data(), dest_len);
#endif
    return ret;
}

std::string utf8TString(const std::wstring& wstr) {
    if (wstr.empty()) {
        return std::string();
    }
#if WIN32
    int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0],
                                   wstr.size(), NULL, 0, NULL, NULL);
    std::string ret = std::string(size, 0);
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], wstr.size(),
                        &ret[0], size, NULL, NULL);
#else
    int source_len = wstr.length();
    std::vector<char> dest(source_len * 3 + 1, 0);
    const wchar_t* source = wstr.c_str();
    int dest_len = 0;
    for (int i = 0; i < source_len; i++) {
        if (wstr[i] <= 0x7F) {
            dest[dest_len] = wstr[i];
            dest_len++;
        } else if (wstr[i] >= 0x80 && wstr[i] <= 0x7FF) {
            wchar_t tmp = wstr[i];
            char first = 0, second = 0, third = 0;
            for (int j = 0; j < 3; j++) {
                wchar_t tmp_quota = tmp % 16;
                switch (j) {
                    case 0:
                        third = tmp_quota;
                        break;
                    case 1:
                        second = tmp_quota;
                        break;
                    case 2:
                        first = tmp_quota;
                        break;
                }
                tmp /= 16;
            }

            dest[dest_len] = 0xC0 + (first << 2) + (second >> 2);
            dest[dest_len + 1] = 0x80 + (((second % 8) % 4) << 4) + third;
            dest_len += 2;
        } else if (wstr[i] >= 0x800 && wstr[i] <= 0xFFFF) {
            wchar_t tmp = wstr[i];
            char first = 0, second = 0, third = 0, fourth = 0;
            for (int j = 0; j < 4; j++) {
                wchar_t tmp_quota = tmp % 16;
                switch (j) {
                    case 0:
                        fourth = tmp_quota;
                        break;
                    case 1:
                        third = tmp_quota;
                        break;
                    case 2:
                        second = tmp_quota;
                        break;
                    case 3:
                        first = tmp_quota;
                        break;
                }
                tmp /= 16;
            }
            dest[dest_len] = 0xE0 + first;
            dest[dest_len + 1] = 0x80 + second << 2 + third >> 2;
            dest[dest_len + 2] = 0x80 + (((third % 8) % 4) << 4) + fourth;
            dest_len += 3;
        } else {
        }
    }
    dest[dest_len++] = '\0';
    std::string ret;
    ret.assign(dest.data(), dest_len);
#endif
    return ret;
}

void copywcharstr(wchar_t* dest, const wchar_t* source, int32_t maxlength) {
    int length = sizeof(wchar_t) * (wcslen(source) + 1);
    memcpy(dest, source, std::min(length, maxlength));
}

void copycharstr(char* dest, const char* source, int32_t maxlength) {
    int length = sizeof(char) * (strlen(source) + 1);
    memcpy(dest, source, std::min(length, maxlength));
}

uint32_t divUp(int32_t x, int32_t y) { return (x + y - 1) / y; }

aoce::ImageType videoType2ImageType(const aoce::VideoType& videoType) {
    switch (videoType) {
        // 带P的格式全部转成r8
        case VideoType::nv12:
        case VideoType::yuv420P:
        case VideoType::yuy2P:
            return ImageType::r8;
        // 交叉格式,rgba8
        case VideoType::yuv2I:
        case VideoType::yvyuI:
        case VideoType::uyvyI:
        case VideoType::mjpg:
        case VideoType::rgba8:
            return ImageType::rgba8;
        // 这几个信息需要在InputLayer输出中转化成rgba
        case VideoType::argb8:
        case VideoType::rgb8:
        case VideoType::bgra8:
            return ImageType::rgba8;
        case VideoType::depth16u:
            return ImageType::r16;
        case VideoType::other:
        default:
            return ImageType::other;
    }
}

int32_t getYuvIndex(const aoce::VideoType& videoType) {
    switch (videoType) {
        case VideoType::nv12:
            return 1;
        case VideoType::yuv420P:
            return 2;
        case VideoType::yuy2P:
            return 3;
        case VideoType::yuv2I:
            return 4;
        case VideoType::yvyuI:
            return 5;
        case VideoType::uyvyI:
            return 6;
        default:
            return -1;
    }
}

ImageFormat videoFormat2ImageFormat(const aoce::VideoFormat& videoFormat) {
    ImageFormat imageFormat = {};
    // 带SP/P的格式转化成r8,否则转化成rgba8
    imageFormat.imageType = videoType2ImageType(videoFormat.videoType);
    imageFormat.width = videoFormat.width;
    imageFormat.height = videoFormat.height;
    if (videoFormat.videoType == VideoType::nv12 ||
        videoFormat.videoType == VideoType::yuv420P) {
        imageFormat.height = videoFormat.height * 3 / 2;
    } else if (videoFormat.videoType == VideoType::yuy2P) {
        imageFormat.height = videoFormat.height * 2;
    } else if (videoFormat.videoType == VideoType::uyvyI ||
               videoFormat.videoType == VideoType::yuv2I ||
               videoFormat.videoType == VideoType::yvyuI) {
        imageFormat.width = videoFormat.width / 2;
    }
    return imageFormat;
}

int32_t getImageTypeSize(const aoce::ImageType& imageType) {
    switch (imageType) {
        case ImageType::bgra8:
            return 4;
        case ImageType::r16:
            return 2;
        case ImageType::r8:
            return 1;
        case ImageType::rgba8:
            return 4;
        default:
            return 0;
    }
}

int32_t getVideoFrame(const aoce::VideoFrame& frame, uint8_t* data) {
    if (frame.videoType == VideoType::yuv420P ||
        frame.videoType == VideoType::yuy2P) {
        int32_t ysize =
            (uint64_t)&frame.data[1][0] - (uint64_t)&frame.data[0][0];
        int32_t usize =
            (uint64_t)&frame.data[2][0] - (uint64_t)&frame.data[1][0];
        int32_t hscale = frame.videoType == VideoType::yuy2P ? 1 : 2;
        int32_t uweight = frame.width / 2;
        int32_t uheight = frame.height / hscale;
        // 按照YUV格式排列
        if (ysize == frame.width * frame.height &&
            ysize == usize * hscale * 2) {
            return 0;
        }
        // 如果data有数据,紧密排列
        if (data != nullptr) {
            // Y可以直接复制
            if (frame.dataAlign[0] == 0 || frame.dataAlign[0] == frame.width) {
                memcpy(data, frame.data[0], frame.width * frame.height);
                data += frame.width * frame.height;
            } else {
                for (int i = 0; i < frame.height; i++) {
                    memcpy(data, frame.data[0] + i * frame.dataAlign[0],
                           frame.width);
                    data += frame.width;
                }
            }
            // U
            if (frame.dataAlign[1] == 0 ||
                frame.dataAlign[1] == frame.width / 2) {
                memcpy(data, frame.data[1], uweight * uheight);
                data += uweight * uheight;
            } else {
                for (int i = 0; i < uheight; i++) {
                    memcpy(data, frame.data[1] + i * frame.dataAlign[1],
                           uweight);
                    data += uweight;
                }
            }
            // V
            if (frame.dataAlign[2] == 0 ||
                frame.dataAlign[2] == frame.width / 2) {
                memcpy(data, frame.data[2], uweight * uheight);
                data += uweight * uheight;
            } else {
                for (int i = 0; i < uheight; i++) {
                    memcpy(data, frame.data[2] + i * frame.dataAlign[2],
                           uweight);
                    data += uweight;
                }
            }
        }
        int32_t yuvsize = frame.width * (frame.height + uheight);
        return yuvsize;
    } else if (frame.videoType == VideoType::nv12) {
        if (frame.data[1] == nullptr) {
            return 0;
        }
        // anroid的摄像头的YUV_420_888可能是nv12,并且Y/UV不连续
        int32_t ysize =
            (uint64_t)&frame.data[1][0] - (uint64_t)&frame.data[0][0];
        // Y plan与uv plan按要求紧密排列
        if (ysize == frame.width * frame.height) {
            return 0;
        }
        if (data != nullptr) {
            // Y可以直接复制
            if (frame.dataAlign[0] == 0 || frame.dataAlign[0] == frame.width) {
                memcpy(data, frame.data[0], frame.width * frame.height);
                data += frame.width * frame.height;
            } else {
                for (int i = 0; i < frame.height; i++) {
                    memcpy(data, frame.data[0] + i * frame.dataAlign[0],
                           frame.width);
                    data += frame.width;
                }
            }
            // uv复制
            if (frame.dataAlign[1] == 0 || frame.dataAlign[1] == frame.width) {
                memcpy(data, frame.data[1], frame.width * frame.height / 2);
                data += frame.width * frame.height / 2;
            } else {
                for (int i = 0; i < frame.height / 2; i++) {
                    memcpy(data, frame.data[1] + i * frame.dataAlign[1],
                           frame.width);
                    data += frame.width;
                }
            }
        }
        return frame.width * frame.height * 3 / 2;
    }
    return 0;
}

std::string getAocePath() {
#if WIN32
    char sz[512] = {0};
    HMODULE ihdll = GetModuleHandleA("aoce.dll");
    ::GetModuleFileNameA(ihdll, sz, 512);
    ::PathRemoveFileSpecA(sz);
    std::string path = sz;
    return path;
#elif __ANDROID__
    return "";
#endif
}

static bool bLoad = false;

void loadAoce() {
    if (bLoad) {
        return;
    }
    bLoad = true;
    ModuleManager::Get().regAndLoad("aoce_vulkan");
#if WIN32
    ModuleManager::Get().regAndLoad("aoce_win_mf");
    ModuleManager::Get().regAndLoad("aoce_cuda");
#elif __ANDROID__
    ModuleManager::Get().regAndLoad("aoce_android");
#endif
#if defined(AOCE_INSTALL_AGORA)
    ModuleManager::Get().regAndLoad("aoce_agora");
#endif
#if defined(AOCE_INSTALL_FFMPEG)
    ModuleManager::Get().regAndLoad("aoce_ffmpeg");
#endif
#if defined(AOCE_INSTALL_TALKTO)
    ModuleManager::Get().regAndLoad("aoce_talkto");
#endif
}

void unloadAoce() {
    if (!bLoad) {
        return;
    }
    bLoad = false;
    ModuleManager::Get().unloadModule("aoce_vulkan");
#if WIN32
    ModuleManager::Get().unloadModule("aoce_win_mf");
    ModuleManager::Get().unloadModule("aoce_cuda");
#elif __ANDROID__
    ModuleManager::Get().unloadModule("aoce_android");
#endif
#if defined(AOCE_INSTALL_AGORA)
    ModuleManager::Get().unloadModule("aoce_agora");
#endif
#if defined(AOCE_INSTALL_FFMPEG)
    ModuleManager::Get().unloadModule("aoce_ffmpeg");
#endif
#if defined(AOCE_INSTALL_TALKTO)
    ModuleManager::Get().unloadModule("aoce_talkto");
#endif
}
