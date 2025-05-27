#pragma once
#include <extensions/PxJoint.h>

#include "Container/String.h"
#include "UObject/ObjectMacros.h"

enum class ELinearConstraintMotion : uint8
{
    LCM_Free,
    LCM_Limited,
    LCM_Locked,
    LCM_Max,
};

enum class EAngularConstraintMotion : uint8
{
    EAC_Free,
    EAC_Limited,
    EAC_Locked,
    EAC_Max,
};

struct FLinearConstraint
{
    DECLARE_STRUCT(FLinearConstraint)
    FLinearConstraint()
        : Limit(0.0f)
        , XMotion(ELinearConstraintMotion::LCM_Locked)
        , YMotion(ELinearConstraintMotion::LCM_Locked)
        , ZMotion(ELinearConstraintMotion::LCM_Locked)
    {}
    
    /** The distance allowed between the two joint reference frames. Distance applies on all axes enabled (one axis means line, two axes implies circle, three axes implies sphere) */
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, Limit)

    /** Indicates the linear constraint applied along the X-axis. Free implies no constraint at all. Locked implies no movement along X is allowed. Limited implies the distance in the joint along all active axes must be less than the Distance provided. */
    UPROPERTY_WITH_FLAGS(EditAnywhere, ELinearConstraintMotion, XMotion)

    /** Indicates the linear constraint applied along the Y-axis. Free implies no constraint at all. Locked implies no movement along Y is allowed. Limited implies the distance in the joint along all active axes must be less than the Distance provided. */
    UPROPERTY_WITH_FLAGS(EditAnywhere, ELinearConstraintMotion, YMotion)

    /** Indicates the linear constraint applied along theZX-axis. Free implies no constraint at all. Locked implies no movement along Z is allowed. Limited implies the distance in the joint along all active axes must be less than the Distance provided. */
    UPROPERTY_WITH_FLAGS(EditAnywhere, ELinearConstraintMotion, ZMotion)

    friend FArchive& operator<<(FArchive& Ar, FLinearConstraint& LinearConstraint)
    {
        uint8 X = static_cast<uint8>(LinearConstraint.XMotion);
        uint8 Y = static_cast<uint8>(LinearConstraint.YMotion);
        uint8 Z = static_cast<uint8>(LinearConstraint.ZMotion);

        Ar << LinearConstraint.Limit << X << Y << Z;

        if (Ar.IsLoading())
        {
            LinearConstraint.XMotion = static_cast<ELinearConstraintMotion>(X);
            LinearConstraint.YMotion = static_cast<ELinearConstraintMotion>(Y);
            LinearConstraint.ZMotion = static_cast<ELinearConstraintMotion>(Z);
        }

        return Ar;
    }
};

struct FConeConstraint
{
    DECLARE_STRUCT(FConeConstraint)
    FConeConstraint()
        : Swing1LimitDegrees(45.0f)
        , Swing2LimitDegrees(45.0f)
        , Swing1Motion(EAngularConstraintMotion::EAC_Limited)
        , Swing2Motion(EAngularConstraintMotion::EAC_Limited)
    {}

    /** Angle of movement along the XY plane. This defines the first symmetric angle of the cone. */
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, Swing1LimitDegrees)

    /** Angle of movement along the XZ plane. This defines the second symmetric angle of the cone. */
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, Swing2LimitDegrees)

    /** Indicates whether the Swing1 limit is used.*/
    UPROPERTY_WITH_FLAGS(EditAnywhere, EAngularConstraintMotion, Swing1Motion)

    /** Indicates whether the Swing2 limit is used.*/
    UPROPERTY_WITH_FLAGS(EditAnywhere, EAngularConstraintMotion, Swing2Motion)

    friend FArchive& operator<<(FArchive& Ar, FConeConstraint& ConeConstraint)
    {
        uint8 S1 = static_cast<uint8>(ConeConstraint.Swing1Motion);
        uint8 S2 = static_cast<uint8>(ConeConstraint.Swing2Motion);

        Ar << ConeConstraint.Swing1LimitDegrees << ConeConstraint.Swing2LimitDegrees
           << S1 << S2;

        if (Ar.IsLoading())
        {
            ConeConstraint.Swing1Motion = static_cast<EAngularConstraintMotion>(S1);
            ConeConstraint.Swing2Motion = static_cast<EAngularConstraintMotion>(S2);
        }

        return Ar;
    }
};

/** Angular roll constraint */
struct FTwistConstraint
{
    DECLARE_STRUCT(FTwistConstraint)
    FTwistConstraint()
        : TwistLimitDegrees(45.0f)
        , TwistMotion(EAngularConstraintMotion::EAC_Limited)
    {}

    /** Symmetric angle of roll along the X-axis. */
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, TwistLimitDegrees)

    /** Indicates whether the Twist limit is used.*/
    UPROPERTY_WITH_FLAGS(EditAnywhere, EAngularConstraintMotion, TwistMotion)

    friend FArchive& operator<<(FArchive& Ar, FTwistConstraint& TwistConstraint)
    {
        uint8 T = static_cast<uint8>(TwistConstraint.TwistMotion);

        Ar << TwistConstraint.TwistLimitDegrees << T;

        if (Ar.IsLoading())
        {
            TwistConstraint.TwistMotion = static_cast<EAngularConstraintMotion>(T);
        }

        return Ar;
    }
};

struct FConstraintInstanceBase
{
    DECLARE_STRUCT(FConstraintInstanceBase)
    FConstraintInstanceBase() = default;

    physx::PxScene* PhysScene = nullptr;

    void SerializeAsset(FArchive& Ar);
};

struct FConstraintInstance : public FConstraintInstanceBase
{
    DECLARE_STRUCT(FConstraintInstance)
    FConstraintInstance();
    
    // PhysX 제약 객체
    physx::PxJoint* ConstraintData = nullptr;             // PhysX 조인트 참조

    void SerializeAsset(FArchive& Ar);
};

struct FConstraintSetup
{
    DECLARE_STRUCT(FConstraintSetup)
    FConstraintSetup() = default;

    // 기본 정보
    UPROPERTY_WITH_FLAGS(EditAnywhere, FString, JointName)                          // 조인트 이름
    UPROPERTY_WITH_FLAGS(EditAnywhere, FString, ConstraintBone1)                    // 첫 번째 본 이름
    UPROPERTY_WITH_FLAGS(EditAnywhere, FString, ConstraintBone2)                    // 두 번째 본 이름

    /** 선형 제약 설정 (위치/이동 제한) */
    UPROPERTY_WITH_FLAGS(EditAnywhere, FLinearConstraint, LinearLimit)

    /** 원뿔 제약 설정 (Swing 회전 제한) */
    UPROPERTY_WITH_FLAGS(EditAnywhere, FConeConstraint, ConeLimit)

    /** 비틀림 제약 설정 (Twist 회전 제한) */
    UPROPERTY_WITH_FLAGS(EditAnywhere, FTwistConstraint, TwistLimit)

    // 모터 설정 (PhysX 4.1+)
    UPROPERTY(EditAnywhere, uint8, bLinearPositionDrive, = 1)               // 선형 위치 모터
    UPROPERTY(EditAnywhere, uint8, bLinearVelocityDrive, = 1)               // 선형 속도 모터
    UPROPERTY(EditAnywhere, uint8, bAngularOrientationDrive, = 1)           // 각도 위치 모터
    UPROPERTY(EditAnywhere, uint8, bAngularVelocityDrive, = 1)              // 각속도 모터

    void SerializeAsset(FArchive& Ar);
};
