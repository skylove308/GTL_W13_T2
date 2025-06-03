#include "Camera.h"
#include "Camera/CameraComponent.h"

ACamera::ACamera()
{
    CameraComponent = AddComponent<UCameraComponent>("Camera");
    RootComponent = CameraComponent;
}
