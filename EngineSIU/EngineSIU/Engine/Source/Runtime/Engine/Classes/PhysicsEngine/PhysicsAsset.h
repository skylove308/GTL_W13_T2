#pragma once
#include "Container/Array.h"

#include <PxPhysicsAPI.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

struct PxAggregateGeom
{
    TArray<physx::PxSphereGeometry> SphereElems;
    TArray<physx::PxBoxGeometry> BoxElems;
    TArray<physx::PxCapsuleGeometry> CapsuleElems;
};

class UBodySetupCore : public UObject
{
    DECLARE_CLASS(UBodySetupCore, UObject)
public:
    UBodySetupCore() = default;

private:
    FName BoneName;
};

class UBodySetup : public UBodySetupCore
{
    DECLARE_CLASS(UBodySetup, UBodySetupCore)
public:
    UBodySetup() = default;

    // DisplayName = Primitives
    PxAggregateGeom AggGeom;
};

class UPhysicsAsset : public UObject
{
public:
    TArray<UBodySetup*> BodySetups;
};
