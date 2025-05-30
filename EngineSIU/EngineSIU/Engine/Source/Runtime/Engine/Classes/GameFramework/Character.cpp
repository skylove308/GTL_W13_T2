#include "Character.h"

UObject* ACharacter::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    return NewActor;
}
