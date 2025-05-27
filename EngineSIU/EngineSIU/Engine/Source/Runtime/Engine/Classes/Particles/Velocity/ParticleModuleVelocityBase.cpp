#include "ParticleModuleVelocityBase.h"

void UParticleModuleVelocityBase::DisplayProperty()
{
    Super::DisplayProperty();
    for (const auto& Property : StaticClass()->GetProperties())
    {
        Property->DisplayInImGui(this);
    }
}

void UParticleModuleVelocityBase::SerializeAsset(FArchive& Ar)
{
    Super::SerializeAsset(Ar);

    Ar << bInWorldSpace << bApplyOwnerScale;
}
