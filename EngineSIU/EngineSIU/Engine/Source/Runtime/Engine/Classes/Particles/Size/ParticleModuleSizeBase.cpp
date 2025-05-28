#include "ParticleModuleSizeBase.h"

void UParticleModuleSizeBase::DisplayProperty()
{
    Super::DisplayProperty();
    for (const auto& Property : StaticClass()->GetProperties())
    {
        ImGui::PushID(Property);
        Property->DisplayInImGui(this);
        ImGui::PopID();
    }
}

void UParticleModuleSizeBase::SerializeAsset(FArchive& Ar)
{
    Super::SerializeAsset(Ar);

    Ar << bUseUniformSize;
}
