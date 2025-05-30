
#pragma once
#include "ShapeComponent.h"

class UCapsuleComponent : public UShapeComponent
{
    DECLARE_CLASS(UCapsuleComponent, UShapeComponent)

public:
    UCapsuleComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;

    /** 반지름과 반높이를 동시에 설정하고, 내부적으로 클램프를 적용합니다. */
    void InitCapsuleSize(float InRadius, float InHalfHeight);

    float GetHalfHeight() const { return CapsuleHalfHeight; }
    float GetRadius() const { return CapsuleRadius; }
    void GetEndPoints(FVector& OutStart, FVector& OutEnd) const;
    
private:
    float CapsuleHalfHeight = 0.88f;
    float CapsuleRadius = 0.34f;
};
