#pragma once
#include <extensions/PxJoint.h>

#include "Container/String.h"
#include "UObject/ObjectMacros.h"

enum class ELinearConstraintMotion : int
{
    LCM_Free,
    LCM_Limited,
    LCM_Locked,
    LCM_Max,
};

enum class EAngularConstraintMotion
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
};

struct FConstraintInstance {
    DECLARE_STRUCT(FConstraintInstance)
    FConstraintInstance();
    
    // 기본 정보
    FString JointName;                          // 조인트 이름
    FString ConstraintBone1;                    // 첫 번째 본 이름
    FString ConstraintBone2;                    // 두 번째 본 이름
    
    /** 선형 제약 설정 (위치/이동 제한) */
    UPROPERTY_WITH_FLAGS(EditAnywhere, FLinearConstraint, LinearLimit)
    
    /** 원뿔 제약 설정 (Swing 회전 제한) */
    UPROPERTY_WITH_FLAGS(EditAnywhere, FConeConstraint, ConeLimit)
    
    /** 비틀림 제약 설정 (Twist 회전 제한) */
    UPROPERTY_WITH_FLAGS(EditAnywhere, FTwistConstraint, TwistLimit)
    
    // 모터 설정 (PhysX 4.1+)
    uint8 bLinearPositionDrive:1;               // 선형 위치 모터
    uint8 bLinearVelocityDrive:1;               // 선형 속도 모터
    uint8 bAngularOrientationDrive:1;           // 각도 위치 모터
    uint8 bAngularVelocityDrive:1;              // 각속도 모터
    
    // PhysX 제약 객체
    physx::PxJoint* ConstraintData = nullptr;             // PhysX 조인트 참조
};
