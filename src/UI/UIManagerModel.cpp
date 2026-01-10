#include "UIManagerModel.h"

std::string UIManagerModel::GetId() const
{
    return "UIManagerModel";
}

void UIManagerModel::Reset()
{
    PreviewTextures.ResetTexture();

    GenerateUnrealORM = false;
    GenerateUnityORM = false;
    SelectedChannel = ORMChannel::AllRGB;

    AOResolutionIndex = 0;
    RoughResolutionIndex = 0;
    MetalResolutionIndex = 0;

    ORMProgress = 0.0f;
    NeedsPreviewUpdate = false;
    IsGeneratingORM = false;
    IsLoadingTexture = false;

    OutputUnreal = "orm_unreal.png";
    OutputUnity = "orm_unity.png";
}
