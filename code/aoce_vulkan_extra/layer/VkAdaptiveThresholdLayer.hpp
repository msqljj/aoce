#pragma once

#include "../VkExtraExport.hpp"
#include "VkLinearFilterLayer.hpp"
#include "VkLuminanceLayer.hpp"
#include "aoce_vulkan/layer/VkLayer.hpp"

namespace aoce {
namespace vulkan {
namespace layer {

class VkAdaptiveThresholdLayer : public VkLayer,
                                 public ITLayer<AdaptiveThresholdParamet> {
    AOCE_LAYER_QUERYINTERFACE(VkAdaptiveThresholdLayer)
   private:
    /* data */
    std::unique_ptr<VkLuminanceLayer> luminance;
    std::unique_ptr<VkBoxBlurLayer> boxBlur;

   public:
    VkAdaptiveThresholdLayer(/* args */);
    ~VkAdaptiveThresholdLayer();

   protected:
    virtual void onUpdateParamet() override;
    virtual void onInitGraph() override;
    virtual void onInitNode() override;
    // virtual void onInitPipe() override;
};

}  // namespace layer
}  // namespace vulkan
}  // namespace aoce