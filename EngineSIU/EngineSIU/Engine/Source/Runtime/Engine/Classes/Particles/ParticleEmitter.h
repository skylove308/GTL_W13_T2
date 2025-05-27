#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "ParticleHelper.h"

class UParticleModule;
class UParticleLODLevel;

class UParticleEmitter : public UObject
{
    DECLARE_CLASS(UParticleEmitter, UObject)

public:
    UParticleEmitter();
    virtual ~UParticleEmitter() override = default;

    void CacheEmitterModuleInfo();

    virtual void DisplayProperty() override;

    TArray<UParticleLODLevel*> GetLODLevels() const { return LODLevels; }
    TMap<UParticleModule*, uint32> ModuleOffsetMap; // Not used
    UParticleLODLevel* GetLODLevel(int32 LODIndex) const;

    virtual void SerializeAsset(FArchive& Ar) override;

public:
    UPROPERTY_WITH_FLAGS(EditAnywhere, FName, EmitterName, = "Default")
    int32 PeakActiveParticles = 0;

    // Below is information updated by calling CacheEmitterModuleInfo
    
    int32 ParticleSize;

    EDynamicEmitterType EmitterType;

private:
    TArray<UParticleLODLevel*> LODLevels; // 현재는 Level 0만 사용
};
