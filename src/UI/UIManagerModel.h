#pragma once

#include "MVC/IModel.h"
#include "MVC/BoilerplateMacro.h"
#include "Utils/Types.h"
#include <array>
#include <string>
#include <atomic>

struct FPreviewTexture final
{
    PreviewTexture AOPreview;
    PreviewTexture RoughPreview;
    PreviewTexture MetallicPreview;
    PreviewTexture ORMPreview;

    void ResetTexture()
    {
        AOPreview = {};
        RoughPreview = {};
        MetallicPreview = {};
        ORMPreview = {};
    }
};


class UIManagerModel final : public IModel
{
private:
    FPreviewTexture PreviewTextures;

    bool GenerateUnrealORM = false;
    bool GenerateUnityORM = false;
    ORMChannel SelectedChannel = ORMChannel::AllRGB;

    int AOResolutionIndex = 0;
    int RoughResolutionIndex = 0;
    int MetalResolutionIndex = 0;

    std::atomic<float> ORMProgress{0.0f};
    std::atomic<bool> NeedsPreviewUpdate{false};
    std::atomic<bool> IsGeneratingORM{false};
    std::atomic<bool> IsLoadingTexture{false};

    std::string OutputUnreal = "orm_unreal.png";
    std::string OutputUnity = "orm_unity.png";

    static constexpr std::array<int, 6> ResolutionValues = {128, 256, 512, 1024, 2048, 4096};
    static constexpr std::array<const char*, 6> ResolutionOptions = {
        "128", "256", "512", "1024", "2048", "4096"
    };
public:
    virtual std::string GetId() const override;

    virtual void Reset() override;


};