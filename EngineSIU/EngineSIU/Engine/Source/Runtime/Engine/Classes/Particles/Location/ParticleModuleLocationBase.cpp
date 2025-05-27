#include "ParticleModuleLocationBase.h"

void UParticleModuleLocationBase::DisplayProperty()
{
    Super::DisplayProperty();
    for (const auto& Property : StaticClass()->GetProperties())
    {
        Property->DisplayInImGui(this);
    }
}

void UParticleModuleLocationBase::SerializeAsset(FArchive& Ar)
{
    Super::SerializeAsset(Ar);

    Ar << bInWorldSpace << bApplyEmitterLocation;
}
