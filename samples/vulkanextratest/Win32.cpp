#if WIN32
#include <memory>
#include <vector>

#include "VkExtraBaseView.hpp"
#include "aoce_vulkan_extra/VkExtraExport.hpp"

static VkExtraBaseView* view = nullptr;

// box模糊
static ITLayer<KernelSizeParamet>* boxFilterLayer = nullptr;
static ITLayer<GaussianBlurParamet>* gaussianLayer = nullptr;
static ITLayer<ChromKeyParamet>* chromKeyLayer = nullptr;
static ITLayer<AdaptiveThresholdParamet>* adaptiveLayer = nullptr;
static ITLayer<GuidedParamet>* guidedLayer = nullptr;
static BaseLayer* convertLayer = nullptr;
static BaseLayer* alphaShowLayer = nullptr;
static BaseLayer* alphaShow2Layer = nullptr;
static BaseLayer* luminanceLayer = nullptr;
static ITLayer<ReSizeParamet>* resizeLayer = nullptr;
static ITLayer<ReSizeParamet>* resizeLayer2 = nullptr;
static ITLayer<KernelSizeParamet>* box1Layer = nullptr;
static ITLayer<HarrisCornerDetectionParamet>* hcdLayer = nullptr;
static ITLayer<NobleCornerDetectionParamet>* ncdLayer = nullptr;
static ITLayer<KernelSizeParamet>* boxFilterLayer1 = nullptr;
// 亮度平均阈值
static ITLayer<float>* averageLT = nullptr;
static ITLayer<BilateralParamet>* bilateralLayer = nullptr;
static ITLayer<DistortionParamet>* bdLayer = nullptr;
static ITLayer<CannyEdgeDetectionParamet>* cedLayer = nullptr;
static BaseLayer* cgaLayer = nullptr;
static ITLayer<FASTFeatureParamet>* fastLayer = nullptr;
static LookupLayer* lutLayer = nullptr;
static ITLayer<int>* dilationLayer = nullptr;
static ITLayer<int>* erosionLayer = nullptr;
static ITLayer<int>* closingLayer = nullptr;
static ITLayer<BlurSelectiveParamet>* blurSelectiveLayer = nullptr;
static ITLayer<BulrPositionParamet>* blurPositionLayer = nullptr;
static ITLayer<SphereRefractionParamet>* srLayer = nullptr;
static ITLayer<PixellateParamet>* ppLayer = nullptr;
static ITLayer<PixellateParamet>* halftoneLayer = nullptr;
static ITLayer<float>* lowPassLayer = nullptr;
static ITLayer<float>* highPassLayer = nullptr;
static BaseLayer* histogramLayer = nullptr;
static BaseLayer* histogramLayer2 = nullptr;
static ITLayer<IOSBlurParamet>* iosBlurLayer = nullptr;
static ITLayer<uint32_t>* kuwaharaLayer = nullptr;
static BaseLayer* lapLayer = nullptr;
static ITLayer<uint32_t>* medianLayer = nullptr;
static BaseLayer* medianK3Layer = nullptr;
static ITLayer<MotionBlurParamet>* motionBlurLayer = nullptr;
static MotionDetectorLayer* motionDetectorLayer = nullptr;
static ITLayer<PoissonParamet>* poissonLayer = nullptr;
static BaseLayer* linerBlendLayer = nullptr;
static PerlinNoiseLayer* noiseLayer = nullptr;

void showMotion(vec4 motion) {
    std::cout << "x: " << motion.x << " y: " << motion.y << " z: " << motion.z
              << " w: " << motion.w << std::endl;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    loadAoce();

    view = new VkExtraBaseView();
    boxFilterLayer = createBoxFilterLayer();
    boxFilterLayer->updateParamet({4, 4});

    gaussianLayer = createGaussianBlurLayer();
    gaussianLayer->updateParamet({10, 20.0f});

    chromKeyLayer = createChromKeyLayer();
    ChromKeyParamet keyParamet = {};
    keyParamet.chromaColor = {0.15f, 0.6f, 0.0f};
    keyParamet.alphaScale = 20.0f;
    keyParamet.alphaExponent = 0.1f;
    keyParamet.alphaCutoffMin = 0.2f;
    keyParamet.lumaMask = 2.0f;
    keyParamet.ambientColor = {0.1f, 0.1f, 0.9f};
    keyParamet.despillScale = 0.0f;
    keyParamet.despillExponent = 0.1f;
    keyParamet.ambientScale = 1.0f;
    chromKeyLayer->updateParamet(keyParamet);

    adaptiveLayer = createAdaptiveThresholdLayer();
    AdaptiveThresholdParamet adaParamet = {};
    adaParamet.boxSize = 10;
    adaParamet.offset = 0.01f;
    adaptiveLayer->updateParamet(adaParamet);

    alphaShowLayer = createAlphaShowLayer();
    alphaShow2Layer = createAlphaShow2Layer();
    luminanceLayer = createLuminanceLayer();

    hcdLayer = createHarrisCornerDetectionLayer();
    HarrisCornerDetectionParamet hcdParamet = {};    
    hcdParamet.harris = 0.04f;
    hcdParamet.harrisBase.threshold = 0.1f;
    hcdParamet.harrisBase.edgeStrength = 1.0f;
    hcdParamet.harrisBase.blueParamet = {5, 0.0f};
    hcdLayer->updateParamet(hcdParamet);

    ncdLayer = createNobleCornerDetectionLayer();
    NobleCornerDetectionParamet ncdParamet = {};
    ncdParamet.harrisBase = hcdParamet.harrisBase;
    ncdLayer->updateParamet(ncdParamet);

    boxFilterLayer1 = createBoxFilterLayer(ImageType::r8);
    boxFilterLayer1->updateParamet({5, 5});

    resizeLayer = createResizeLayer(ImageType::rgba8);
    resizeLayer->updateParamet({true, 1920 / 8, 1080 / 8});
    resizeLayer2 = createResizeLayer(ImageType::rgba8);
    resizeLayer2->updateParamet({true, 1920, 1080});

    convertLayer = createConvertImageLayer();

    averageLT = createAverageLuminanceThresholdLayer();

    bilateralLayer = createBilateralLayer();
    bilateralLayer->updateParamet({10, 10.0f, 10.0f});

    bdLayer = createBulgeDistortionLayer();
    DistortionParamet bdParamet = {};
    bdParamet.aspectRatio = 1080.0 / 1920.0;
    bdLayer->updateParamet(bdParamet);

    guidedLayer = createGuidedLayer();
    guidedLayer->updateParamet({20, 0.000001f});

    cedLayer = createCannyEdgeDetectionLayer();
    CannyEdgeDetectionParamet cedParamet = {};
    cedLayer->updateParamet(cedParamet);

    cgaLayer = createCGAColorspaceLayer();

    dilationLayer = createDilationLayer(true);
    // dilationLayer->updateParamet(10);

    erosionLayer = createErosionLayer(true);
    // erosionLayer->updateParamet(20);

    closingLayer = createClosingLayer(true);

    blurSelectiveLayer = createBlurSelectiveLayer();
    BlurSelectiveParamet bsp = {};
    bsp.gaussian.blurRadius = 20;
    blurSelectiveLayer->updateParamet(bsp);

    blurPositionLayer = createBlurPositionLayer();
    BulrPositionParamet bpp = {};
    bpp.gaussian.blurRadius = 20;
    blurPositionLayer->updateParamet(bpp);

    srLayer = createSphereRefractionLayer();

    ppLayer = createPixellateLayer();

    halftoneLayer = createHalftoneLayer();

    lowPassLayer = createLowPassLayer();

    highPassLayer = createHighPassLayer();

    histogramLayer = createHistogramLayer(true);
    histogramLayer2 = createHistogramLayer(false);

    iosBlurLayer = createIOSBlurLayer();

    fastLayer = createColourFASTFeatureDetector();

    kuwaharaLayer = createKuwaharaLayer();
    kuwaharaLayer->updateParamet(10);

    lapLayer = createLaplacianLayer(false);

    medianLayer = createMedianLayer(true);
    medianLayer->updateParamet(10);

    medianK3Layer = createMedianK3Layer(false);

    motionBlurLayer = createMotionBlurLayer();
    motionBlurLayer->updateParamet({2.0f, 45.0});

    motionDetectorLayer = createMotionDetectorLayer();
    motionDetectorLayer->setMotionHandle(showMotion);

    poissonLayer = createPoissonBlendLayer();
    poissonLayer->updateParamet({0.5f, 10});

    linerBlendLayer = createLinearBurnBlendLayer();

    noiseLayer = createPerlinNoiseLayer();
    noiseLayer->setImageSize(1920, 1080);

    std::vector<uint8_t> lutData;
    std::vector<BaseLayer*> layers;
    // 如果为true,层需要二个输入,用原始图像做第二个输入
    bool bAutoIn = false;
    // ---高斯模糊
    // layers.push_back(gaussianLayer->getLayer());
    // 检测resize效果
    // layers.push_back(resizeLayer->getLayer());
    // layers.push_back(resizeLayer2->getLayer());
    // ---查看自适应阈值化效果
    // layers.push_back(adaptiveLayer->getLayer());
    // layers.push_back(alphaShowLayer);
    // ---查看Harris 角点检测
    // bAutoIn = true;
    // layers.push_back(luminanceLayer);
    // layers.push_back(hcdLayer->getLayer());//hcdLayer ncdLayer
    // layers.push_back(boxFilterLayer1->getLayer());
    // layers.push_back(alphaShow2Layer);
    // ---查看导向滤波效果
    // layers.push_back(chromKeyLayer->getLayer());
    // layers.push_back(guidedLayer->getLayer());
    // layers.push_back(alphaShowLayer);
    // ---平均亮度调整阈值
    // layers.push_back(averageLT->getLayer());
    // layers.push_back(alphaShowLayer);
    // --- LUT颜色表映射
    // std::string aocePath = getAocePath();
    // std::wstring spath =
    //     utf8TWstring(aocePath + "/images/lookup_amatorka.binary");
    // if (existsFile(spath.c_str())) {
    //     loadFileBinary(spath.c_str(), lutData);
    //     lutLayer = createLookupLayer();
    //     layers.push_back(lutLayer->getLayer());
    // }
    // ---双边滤波
    // layers.push_back(bilateralLayer->getLayer());
    // ---凸起失真，鱼眼效果
    // layers.push_back(bdLayer->getLayer());
    // canny边缘检测
    // layers.push_back(cedLayer->getLayer());
    // layers.push_back(alphaShowLayer);
    // ---CGAColorspace效果
    // layers.push_back(cgaLayer);
    // ---dilation/erosion
    // layers.push_back(cedLayer->getLayer());
    // // layers.push_back(dilationLayer->getLayer());
    // // layers.push_back(erosionLayer->getLayer());
    // layers.push_back(closingLayer->getLayer());
    // layers.push_back(alphaShowLayer);
    // ---高斯选择模糊1
    // layers.push_back(blurSelectiveLayer->getLayer());
    // ---高斯选择模糊2
    // layers.push_back(blurPositionLayer->getLayer());
    // ---球形映射,图像倒立
    // layers.push_back(srLayer->getLayer());
    // ---马赛克
    // layers.push_back(ppLayer->getLayer());
    // ---半色调效果，如新闻打印
    // layers.push_back(halftoneLayer->getLayer());
    // ---低通滤波器
    // layers.push_back(lowPassLayer->getLayer());
    // ---高通滤波器
    // bAutoIn = true;
    // layers.push_back(highPassLayer->getLayer());
    // ---直方图 1通道 luminanceLayer cedLayer->getLayer()
    // layers.push_back(luminanceLayer);
    // layers.push_back(histogramLayer);
    // layers.push_back(alphaShowLayer);
    // ---直方图 4通道
    // layers.push_back(histogramLayer2);
    // layers.push_back(alphaShowLayer);
    // ---IOS Blur
    // layers.push_back(iosBlurLayer->getLayer());
    // --- fast feature detection
    // layers.push_back(fastLayer->getLayer());
    // --- Kuwahara
    // layers.push_back(kuwaharaLayer->getLayer());
    // ---Laplacian
    // layers.push_back(lapLayer);
    // ---中值滤波
    // layers.push_back(luminanceLayer);
    // layers.push_back(medianLayer->getLayer());
    // layers.push_back(alphaShowLayer);
    // ---中值滤波K3
    // layers.push_back(medianK3Layer);
    // ---运动模糊
    // layers.push_back(motionBlurLayer->getLayer());
    // ---运动检测
    // layers.push_back(motionDetectorLayer->getLayer());
    // ---松泊融合 poissonLayer->getLayer() linerBlendLayer
    // bAutoIn = true;
    // layers.push_back(noiseLayer->getLayer());
    // layers.push_back(poissonLayer->getLayer());
    // ---柏林噪声
    // layers.push_back(noiseLayer->getLayer());

    view->initGraph(layers, hInstance, bAutoIn);
    // 如果有LUT,需要在initGraph后,加载Lut表格数据
    if (lutLayer != nullptr && lutData.size() > 0) {
        lutLayer->loadLookUp(lutData.data(), lutData.size());
    }
    view->openDevice();

    // std::thread trd([&]() {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    //     view->enableLayer(false);
    // });

    view->run();
    unloadAoce();
}

#endif