#include "ParticleModuleAccelerationBase.h"

void UParticleModuleAccelerationBase::DisplayProperty()
{
    Super::DisplayProperty();
    for (const auto& Property : StaticClass()->GetProperties())
    {
        Property->DisplayInImGui(this);
    }
}

void UParticleModuleAccelerationBase::SerializeAsset(FArchive& Ar)
{
    Super::SerializeAsset(Ar);

    Ar << bInWorldSpace << bApplyOwnerScale;
}
