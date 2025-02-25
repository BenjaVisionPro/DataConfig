#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointerInternals.h"
#include "Runtime/Launch/Resources/Version.h"
#include "DcAnyStruct.generated.h"

///	A struct that contains a heap stored struct of any type.
///	 - has value semantic and behaves like `TSharedRef`
///	 - can be safely passed around in BP as arguments

USTRUCT(BlueprintType)
struct DATACONFIGEXTRA_API FDcAnyStruct
{
	GENERATED_BODY()

#if ENGINE_MAJOR_VERSION == 5
	using FReferenceControllerBase = SharedPointerInternals::TReferenceControllerBase<ESPMode::NotThreadSafe>;
	using FSharedReferencer = SharedPointerInternals::FSharedReferencer<ESPMode::NotThreadSafe>;
#else
	using FReferenceControllerBase = SharedPointerInternals::FReferenceControllerBase;
	using FSharedReferencer = SharedPointerInternals::FSharedReferencer<ESPMode::Fast>;
#endif

	struct DATACONFIGEXTRA_API AnyStructReferenceController : public FReferenceControllerBase
	{
		AnyStructReferenceController(FDcAnyStruct* InAnyStruct)
		{
			DataPtr = InAnyStruct->DataPtr;
			StructClass = InAnyStruct->StructClass;
		}

		void DestroyObject() override;

		AnyStructReferenceController(const AnyStructReferenceController&) = delete;
		AnyStructReferenceController& operator=(const AnyStructReferenceController&) = delete;

		void* DataPtr;
		UScriptStruct* StructClass;
	};

	FDcAnyStruct(SharedPointerInternals::FNullTag* = nullptr)
		: DataPtr(nullptr)
		, StructClass(nullptr)
		, SharedReferenceCount(nullptr)
	{}

	FDcAnyStruct(void* InDataPtr, UScriptStruct* InStructClass)
		: DataPtr(InDataPtr)
		, StructClass(InStructClass)
		, SharedReferenceCount(new AnyStructReferenceController(this))
	{}

	FDcAnyStruct(FDcAnyStruct const& InOther)
		: DataPtr(InOther.DataPtr)
		, StructClass(InOther.StructClass)
		, SharedReferenceCount(InOther.SharedReferenceCount)
	{}

	FDcAnyStruct(FDcAnyStruct&& InOther)
		: DataPtr(InOther.DataPtr)
		, StructClass(InOther.StructClass)
		, SharedReferenceCount(MoveTemp(InOther.SharedReferenceCount))
	{
		InOther.DataPtr = nullptr;
		InOther.StructClass = nullptr;
	}

	void Reset()
	{
		*this = FDcAnyStruct();
		check(DataPtr == nullptr && StructClass == nullptr);
	}

	FDcAnyStruct& operator=(SharedPointerInternals::FNullTag*)
	{
		Reset();
		return *this;
	}

	FDcAnyStruct& operator=(FDcAnyStruct const& InOther)
	{
		FDcAnyStruct Temp = InOther;
		Swap(Temp, *this);
		return *this;
	}

	FDcAnyStruct& operator=(FDcAnyStruct&& InOther)
	{
		if (this != &InOther)
		{
			DataPtr = InOther.DataPtr;
			StructClass = InOther.StructClass;

			InOther.DataPtr = nullptr;
			InOther.StructClass = nullptr;
			SharedReferenceCount = MoveTemp(InOther.SharedReferenceCount);
		}

		return *this;
	}

	int32 GetSharedReferenceCount() const
	{
		return SharedReferenceCount.GetSharedReferenceCount();
	}

	bool IsValid() const
	{
		return SharedReferenceCount.IsValid();
	}

	template<class T>
	FDcAnyStruct(T* StructPtr)
		: DataPtr((void*)StructPtr)
		, StructClass(TBaseStructure<T>::Get())
		, SharedReferenceCount(new AnyStructReferenceController(this))
	{}

	template<class T>
	T* GetChecked() const
	{
		check(TBaseStructure<T>::Get() == StructClass);
		return (T*)(DataPtr);
	}

	void* DataPtr = nullptr;
	UScriptStruct* StructClass = nullptr;
	FSharedReferencer SharedReferenceCount;

	///	Dump data to output. Intended to be called in debugger immediate.
	void DebugDump();
};








