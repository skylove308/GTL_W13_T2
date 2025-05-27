#include "ParticleSystem.h"

#include "Particles/ParticleEmitter.h"
#include "Container/ArrayHelper.h"

void UParticleSystem::SerializeAsset(FArchive& Ar)
{
    FArrayHelper::SerializePtrAsset(Ar, Emitters);
}
