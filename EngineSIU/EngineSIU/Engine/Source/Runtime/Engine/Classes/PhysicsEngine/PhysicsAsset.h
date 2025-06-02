#pragma once
#include "Container/Array.h"

#include <PxPhysicsAPI.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Math/Shapes.h"

class USkeletalMesh;
struct FConstraintSetup;

enum class EGeomType : uint8
{
    ESphere,
    EBox,
    ECapsule,
    MAX,
};

enum class ECollisionGroup : uint32
{
    GROUP_NONE = 0,
    GROUP_WORLD_STATIC = (1 << 0), // 정적 환경 (바닥, 벽 등)
    GROUP_WORLD_DYNAMIC = (1 << 1),
    GROUP_CHARACTER_BODY = (1 << 2), // 캐릭터 루트 캡슐
    GROUP_CHARACTER_RAGDOLL = (1 << 3), // 캐릭터 래그돌 본 Shape들
    // 필요에 따라 더 많은 그룹 추가
    GROUP_MAX = (1 << 4), // 모든 그룹을 포함하는 플래그
};

inline ECollisionGroup operator|(ECollisionGroup a, ECollisionGroup b) {
    return static_cast<ECollisionGroup>(static_cast<uint8>(a) | static_cast<uint8>(b));
}
inline ECollisionGroup operator&(ECollisionGroup a, ECollisionGroup b) {
    return static_cast<ECollisionGroup>(static_cast<uint8>(a) & static_cast<uint8>(b));
}
inline ECollisionGroup operator~(ECollisionGroup a) {
    return static_cast<ECollisionGroup>(~static_cast<uint8>(a));
}


template <>
struct magic_enum::customize::enum_range<ECollisionGroup> // <- 여기 T를 실제 타입으로 수정
{
    static constexpr bool is_flags = true;
};

struct FKAggregateGeom
{
    TArray<physx::PxShape*> SphereElems;
    TArray<physx::PxShape*> BoxElems;
    TArray<physx::PxShape*> CapsuleElems;
};

enum class ERigidBodyType : uint8
{
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
    UPROPERTY_WITH_FLAGS(EditAnywhere, FVector, Extent, = FVector(1,1,1)) // Half Extent
    UPROPERTY_WITH_FLAGS(EditAnywhere | BitMask, ECollisionGroup, CollisionGroup, = ECollisionGroup::GROUP_WORLD_STATIC)
    UPROPERTY_WITH_FLAGS(EditAnywhere | BitMask, ECollisionGroup, CollisionWithGroup, = static_cast<ECollisionGroup>(static_cast<uint32>(ECollisionGroup::GROUP_MAX) - 1))
    friend FArchive& operator<<(FArchive& Ar, AggregateGeomAttributes& Attributes);
};

class UBodySetupCore : public UObject
{
    DECLARE_CLASS(UBodySetupCore, UObject)
    
public:
    UBodySetupCore() = default;
    void SetBoneName(const FName& InBoneName) { BoneName = InBoneName; }

    FName BoneName;

    virtual void SerializeAsset(FArchive& Ar) override;
};

class UBodySetup : public UBodySetupCore
{
    DECLARE_CLASS(UBodySetup, UBodySetupCore)
    
public:
    UBodySetup() = default;

    // DisplayName = Primitives
    FKAggregateGeom AggGeom;
    
    TArray<AggregateGeomAttributes> GeomAttributes;

    virtual void SerializeAsset(FArchive& Ar) override;
};

class UPhysicsAsset : public UObject
{
    DECLARE_CLASS(UPhysicsAsset, UObject)
    
public:
    UPhysicsAsset() = default;

    USkeletalMesh* PreviewSkeletalMesh;
    
    TArray<UBodySetup*> BodySetups;

    /**
     * 언리얼 엔진에서는 UPhysicsConstraintTemplate의 배열을 가지고있고,
     * UPhysicsConstraintTemplate에는 FConstraintInstance타입인
     * DefaultInstance를 가지고있음.
     * UPhysicsConstraintTemplate의 DEPRECATED된 멤버 변수들을 보면
     * FConstraintInstance의 멤버 변수와 겹치기 때문에 사실상 같은 정보로 판단하여
     * FConstraintInstance를 사용.
     */
    TArray<FConstraintSetup*> ConstraintSetups;

    virtual bool SetPreviewMesh(USkeletalMesh* PreviewMesh);
    virtual USkeletalMesh* GetPreviewMesh() const;

    virtual void SerializeAsset(FArchive& Ar) override;
};
