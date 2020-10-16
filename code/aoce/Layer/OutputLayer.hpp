#pragma once

#include "BaseLayer.hpp"

namespace aoce {

struct OutputParamet {
    int32_t bCpu = true;
    int32_t bGpu = false;
};

class ACOE_EXPORT OutputLayer : public ITLayer<OutputParamet> {
   protected:
    imageProcessHandle onImageProcessEvent;

   public:
    inline void setImageProcessHandle(imageProcessHandle handle) {
        onImageProcessEvent = handle;
    };

   protected:
    inline void onImageProcessHandle(uint8_t* data, int32_t width,
                                     int32_t height, int32_t outIndex = 0) {
        if (onImageProcessEvent) {
            onImageProcessEvent(data, width, height, outIndex);
        }
    }
};
}  // namespace aoce