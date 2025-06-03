#pragma once
#include "GameFramework/Actor.h"

class UCameraComponent;
class ACamera : public AActor
{
    DECLARE_CLASS(ACamera, AActor);

public:
    ACamera();
    virtual ~ACamera() = default;

    UCameraComponent* CameraComponent;
};

