#pragma once

#include "MVC/IModel.h"
#include "MVC/BoilerplateMacro.h"
#include "Utils/Types.h"
#include <array>
#include <string>
#include <atomic>

/**
 * @brief Groups all preview textures owned by the UI model.
 */
struct FPreviewTexture final
{
    PreviewTexture AOPreview;       ///< Ambient occlusion preview texture.
    PreviewTexture RoughPreview;    ///< Roughness preview texture.
    PreviewTexture MetallicPreview; ///< Metallic preview texture.
    PreviewTexture ORMPreview;      ///< Packed ORM preview texture.

    /**
     * @brief Resets every preview texture to its default empty state.
     */
    void ResetTexture()
    {
        AOPreview = {};
        RoughPreview = {};
        MetallicPreview = {};
        ORMPreview = {};
    }
};


/**
 * @brief Stores UI state for texture previews and ORM processing options.
 */
class UIManagerModel final : public IModel
{
private:
    FPreviewTexture PreviewTextures; ///< Preview texture data for the current UI session.

    bool GenerateUnrealORM = false;                 ///< Whether Unreal ORM output should be generated.
    bool GenerateUnityORM = false;                  ///< Whether Unity ORM output should be generated.
    ORMChannel SelectedChannel = ORMChannel::AllRGB; ///< Currently selected preview channel.

    int AOResolutionIndex = 0;    ///< Selected AO resolution index.
    int RoughResolutionIndex = 0; ///< Selected roughness resolution index.
    int MetalResolutionIndex = 0; ///< Selected metallic resolution index.

    std::atomic<float> ORMProgress{0.0f};       ///< Current ORM processing progress.
    std::atomic<bool> NeedsPreviewUpdate{false}; ///< Whether the preview should be refreshed.
    std::atomic<bool> IsGeneratingORM{false};    ///< Whether ORM processing is active.
    std::atomic<bool> IsLoadingTexture{false};   ///< Whether texture loading is active.

    std::string OutputUnreal = "orm_unreal.png"; ///< Unreal ORM output path.
    std::string OutputUnity = "orm_unity.png";   ///< Unity ORM output path.

    /** @brief Supported square texture sizes. */
    static constexpr std::array<int, 6> ResolutionValues = {128, 256, 512, 1024, 2048, 4096};

    /** @brief UI labels for supported texture sizes. */
    static constexpr std::array<const char*, 6> ResolutionOptions = {
        "128", "256", "512", "1024", "2048", "4096"
    };
public:
    /**
     * @brief Returns the stable model identifier.
     * @return Model id string.
     */
    virtual std::string GetId() const override;

    /**
     * @brief Restores model state to defaults.
     */
    virtual void Reset() override;


};
