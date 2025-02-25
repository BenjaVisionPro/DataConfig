#include "DataConfig/Writer/DcPrettyPrintWriter.h"
#include "DataConfig/Property/DcPropertyUtils.h"
#include "DataConfig/Misc/DcTemplateUtils.h"
#include "DataConfig/Misc/DcTypeUtils.h"
#include "Misc/OutputDevice.h"
#include "Misc/Fnv.h"

static const FString _PER_INDENT = FString(TEXT("    "));

FDcPrettyPrintWriter::FDcPrettyPrintWriter()
	: Output((FOutputDevice&)*GWarn)
{}

FDcPrettyPrintWriter::FDcPrettyPrintWriter(FOutputDevice& InOutput)
	: Output(InOutput)
{}

FDcPrettyPrintWriter::~FDcPrettyPrintWriter()
{
	Output.Flush();
}

FDcResult FDcPrettyPrintWriter::PeekWrite(EDcDataEntry Next, bool* bOutOk)
{
	ReadOut(bOutOk, true);
	return DcOk();	// accepts anything
}

FDcResult FDcPrettyPrintWriter::WriteBool(bool Value)
{
	Output.Logf(TEXT("%s<Bool> '%s'"), *Indent, (Value
		? TEXT("true")
		: TEXT("false"))
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteName(const FName& Value)
{
	Output.Logf(TEXT("%s<Name> '%s'"), *Indent, *DcPropertyUtils::SafeNameToString(Value));
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteString(const FString& Value)
{
	Output.Logf(TEXT("%s<String> '%s'"), *Indent, *Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteText(const FText& Value)
{
	Output.Logf(TEXT("%s<Text> '%s'"), *Indent, *Value.ToString());
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteEnum(const FDcEnumData& Value)
{
	Output.Logf(TEXT("%s<Enum> '%s', '%s', IsUnsigned: '%d', '0x%X'"),
		*Indent,
		*DcPropertyUtils::SafeNameToString(Value.Type),
		*DcPropertyUtils::SafeNameToString(Value.Name),
		Value.bIsUnsigned,
		Value.Signed64
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteStructRootAccess(FDcStructAccess& Access)
{
	Output.Logf(TEXT("%s<StructRoot> '%s'"), *Indent, *DcPropertyUtils::SafeNameToString(Access.Name));
	SetIndentLevel(IndentLevel + 1);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteStructEndAccess(FDcStructAccess& Access)
{
	SetIndentLevel(IndentLevel - 1);
	Output.Logf(TEXT("%s<StructEnd> '%s'"), *Indent, *DcPropertyUtils::SafeNameToString(Access.Name));
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteClassRootAccess(FDcClassAccess& Access)
{
	Output.Logf(TEXT("%s<ClassRoot> '%s'"), *Indent, *DcPropertyUtils::SafeNameToString(Access.Name));
	SetIndentLevel(IndentLevel + 1);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteClassEndAccess(FDcClassAccess& Access)
{
	SetIndentLevel(IndentLevel - 1);
	Output.Logf(TEXT("%s<ClassEnd> '%s'"), *Indent, *DcPropertyUtils::SafeNameToString(Access.Name));
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteMapRoot()
{
	Output.Logf(TEXT("%s<MapBegin>"), *Indent);
	SetIndentLevel(IndentLevel + 1);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteMapEnd()
{
	SetIndentLevel(IndentLevel - 1);
	Output.Logf(TEXT("%s<MapEnd>"), *Indent);
	return DcOk();
}


FDcResult FDcPrettyPrintWriter::WriteArrayRoot()
{
	Output.Logf(TEXT("%s<ArrayRoot>"), *Indent);
	SetIndentLevel(IndentLevel + 1);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteArrayEnd()
{
	SetIndentLevel(IndentLevel - 1);
	Output.Logf(TEXT("%s<ArrayEnd>"), *Indent);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteSetRoot()
{
	Output.Logf(TEXT("%s<SetRoot>"), *Indent);
	SetIndentLevel(IndentLevel + 1);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteSetEnd()
{
	SetIndentLevel(IndentLevel - 1);
	Output.Logf(TEXT("%s<SetEnd>"), *Indent);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteOptionalRoot()
{
	Output.Logf(TEXT("%s<OptionalRoot>"), *Indent);
	SetIndentLevel(IndentLevel + 1);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteOptionalEnd()
{
	SetIndentLevel(IndentLevel - 1);
	Output.Logf(TEXT("%s<OptionalEnd>"), *Indent);
	return DcOk();
}



FDcResult FDcPrettyPrintWriter::WriteObjectReference(const UObject* Value)
{
	check(Value);	// note that this is guaranteed to be non-null
	DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(Value));
	Output.Logf(TEXT("%s<ObjectReference> '%d' '%s'"), *Indent, Value->GetUniqueID(), *GetNameSafe(Value));
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteClassReference(const UClass* Value)
{
	DC_TRY(DcPropertyUtils::HeuristicVerifyPointer(Value));
	Output.Logf(TEXT("%s<ClassReference> '%s'"), *Indent,  *GetNameSafe(Value));
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteWeakObjectReference(const FWeakObjectPtr& Value)
{
	struct FView
	{
		int32		ObjectIndex;
		int32		ObjectSerialNumber;
	};
	static_assert(DcTypeUtils::TIsSameSize<FView, FWeakObjectPtr>::Value, "should be same size");

	const FView& View = (const FView&)Value;
	Output.Logf(TEXT("%s<WeakObjectReference> Index: '%d', Gen: '%d'"), *Indent,
		View.ObjectIndex,
		View.ObjectSerialNumber
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteLazyObjectReference(const FLazyObjectPtr& Value)
{
	Output.Logf(TEXT("%s<LazyObjectReference> '%s'"), *Indent,
		*Value.GetUniqueID().ToString()
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteSoftObjectReference(const FSoftObjectPtr& Value)
{
	Output.Logf(TEXT("%s<SoftObjectReference> '%s'"), *Indent,
		*Value.ToString()
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteSoftClassReference(const FSoftObjectPtr& Value)
{
	Output.Logf(TEXT("%s<SoftClassReference> '%s'"), *Indent,
		*Value.ToString()
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteInterfaceReference(const FScriptInterface& Value)
{
	Output.Logf(TEXT("%s<InterfaceReference> '%s'"), *Indent, *GetNameSafe(Value.GetObject()));
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteFieldPath(const FFieldPath& Value)
{
	Output.Logf(TEXT("%s<FieldPath> '%s'"), *Indent,
		*Value.ToString()
	);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteDelegate(const FScriptDelegate& Value)
{
	Output.Logf(TEXT("%s<Delegate> '%s'"), *Indent, *Value.ToString<UObject>());
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteMulticastInlineDelegate(const FMulticastScriptDelegate& Value)
{
	Output.Logf(TEXT("%s<MulticastInlineDelegate> '%s'"), *Indent, *Value.ToString<UObject>());
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteMulticastSparseDelegate(const FMulticastScriptDelegate& Value)
{
	Output.Logf(TEXT("%s<MulticastSparseDelegate> '%s'"), *Indent, *Value.ToString<UObject>());
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteInt8(const int8& Value)
{
	Output.Logf(TEXT("%s<Int8> '%d'"), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteInt16(const int16& Value)
{
	Output.Logf(TEXT("%s<Int16> '%d'"), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteInt32(const int32& Value)
{
	Output.Logf(TEXT("%s<Int32> '%d'"), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteInt64(const int64& Value)
{
	Output.Logf(TEXT("%s<Int64> '%lld'"), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteUInt8(const uint8& Value)
{
	Output.Logf(TEXT("%s<UInt8> '%u'"), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteUInt16(const uint16& Value)
{
	Output.Logf(TEXT("%s<UInt16> '%u'"), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteUInt32(const uint32& Value)
{
	Output.Logf(TEXT("%s<UInt32> '%u'"), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteUInt64(const uint64& Value)
{
	Output.Logf(TEXT("%s<UInt64> '%llu'"), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteFloat(const float& Value)
{
	Output.Logf(TEXT("%s<Float> '%f'"), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteDouble(const double& Value)
{
	Output.Logf(TEXT("%s<Double> '%f'"), *Indent, Value);
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteBlob(const FDcBlobViewData& Value)
{
	Output.Logf(TEXT("%s<Blob>: Size: '%d', Hash: '%llu'"), *Indent, Value.Num, FFnv::MemFnv64(Value.DataPtr, Value.Num));
	return DcOk();
}

FDcResult FDcPrettyPrintWriter::WriteNone()
{
	Output.Logf(TEXT("%s<None>"), *Indent);
	return DcOk();
}

void FDcPrettyPrintWriter::SetIndentLevel(int InLevel)
{
	static const TCHAR* _WS_INDENT = TEXT("|   ");
	static const TCHAR* _LAST_INDENT = TEXT("|---");

	if (InLevel == 0)
	{
		Indent = TEXT("");
	}
	else
	{
		Indent.Empty();
		for (int Ix = 0; Ix < InLevel - 1; Ix++)
			Indent += _WS_INDENT;

		Indent += _LAST_INDENT;
	}

	IndentLevel = InLevel;
}

FName FDcPrettyPrintWriter::ClassId() { return FName(TEXT("DcPrettyPrintWriter")); }
FName FDcPrettyPrintWriter::GetId() { return ClassId(); }

