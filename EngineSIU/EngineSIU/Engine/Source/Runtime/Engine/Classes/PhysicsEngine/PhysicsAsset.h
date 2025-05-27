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
    TArray<physx::PxShape*> SphereElems;
    TArray<physx::PxShape*> BoxElems;
    TArray<physx::PxShape*> CapsuleElems;
};

enum class EGeomType
{
    ESphere,
    EBox,
    ECapsule,
};

enum class ERigidBodyType {
    STATIC,
    DYNAMIC, 
    KINEMATIC
};

struct AggregateGeomAttributes
{
    DECLARE_STRUCT(AggregateGeomAttributes)
    AggregateGeomAttributes() = default;

    UPROPERTY_WITH_FLAGS(EditAnywhere, EGeomType, GeomType, = EGeomType::EBox)
    UPROPERTY_WITH_FLAGS(EditAnywhere, FVector, Offset)
    UPROPERTY_WITH_FLAGS(EditAnywhere, FRotator, Rotation)
    UPROPERTY_WITH_FLAGS(EditAnywhere, FVector, Extent, = FVector(1,1,1))
};

class UBodySetupCore : public UObject
{
    DECLARE_CLASS(UBodySetupCore, UObject)
public:
    UBodySetupCore() = default;

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
