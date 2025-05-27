#include "ConstraintInstance.h"

FConstraintInstance::FConstraintInstance()
{
}

void FConstraintInstanceBase::SerializeAsset(FArchive& Ar)
{
}

void FConstraintSetup::SerializeAsset(FArchive& Ar)
{
    Ar << JointName
        << ConstraintBone1
        << ConstraintBone2
        << LinearLimit
        << ConeLimit
        << TwistLimit
        << bLinearPositionDrive
        << bLinearVelocityDrive
        << bAngularOrientationDrive
        << bAngularVelocityDrive;

    // TODO: ConstraintData는 어떻게 저장해야하는지 생각해보기.
}
