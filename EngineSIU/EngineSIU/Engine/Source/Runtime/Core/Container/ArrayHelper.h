#pragma once
#include "Array.h"
#include "Serialization/Archive.h"
#include "UObject/ObjectFactory.h"

struct FArrayHelper
{
    template <typename T, typename AllocatorType>
    static void SerializePtrAsset(FArchive& Ar, TArray<T, AllocatorType>& Array)
    {
        static_assert(std::is_pointer_v<T>, "T must be a pointer type for SerializePtrAsset.");
    
        using PointerToType = std::remove_pointer_t<T>;
        using SizeType = typename TArray<T, AllocatorType>::SizeType;
    
        uint32 ArraySize = static_cast<uint32>(Array.Num());
        Ar << ArraySize;

        if (Ar.IsLoading())
        {
            if constexpr (std::is_base_of_v<UObject, PointerToType>)
            {
                for (SizeType Index = 0; Index < Array.Num(); ++Index)
                {
                    Array.ContainerPrivate[Index]->MarkAsGarbage();
                }
            }
            else
            {
                for (SizeType Index = 0; Index < Array.Num(); ++Index)
                {
                    delete Array.ContainerPrivate[Index];
                }
            }

            Array.Empty();
            Array.SetNum(ArraySize);
        }

        for (SizeType Index = 0; Index < ArraySize; ++Index)
        {
            if (Ar.IsLoading())
            {
                if constexpr (std::is_base_of_v<UObject, PointerToType>)
                {
                    Array.ContainerPrivate[Index] = FObjectFactory::ConstructObject<PointerToType>(nullptr);
                }
                else
                {
                    Array.ContainerPrivate[Index] = new PointerToType();
                }
            }
            Array.ContainerPrivate[Index]->SerializeAsset(Ar);
        }
    }
};
