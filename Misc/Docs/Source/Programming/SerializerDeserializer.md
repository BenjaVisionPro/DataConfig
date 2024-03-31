# Serializer/Deserializer

Serializer/Deserializer are built on top of the data model to convert between external format and Unreal Engine property system.

## Context

A company class to the deserializer is `FDcDeserializeContext`:

```c++
// DataConfigCore/Public/DataConfig/Deserialize/DcDeserializeTypes.h
struct DATACONFIGCORE_API FDcDeserializeContext
{
    //...
    FDcDeserializer* Deserializer;
    FDcReader* Reader;
    FDcPropertyWriter* Writer;
    //...
};
```

Note how it takes an `FDcReader` and a `FPropertyWriter` - we're deserializing arbitrary format into the property system.

The mirrored version for serializer is `FDcSerializeContext`.

```c++
// DataConfigCore/Public/DataConfig/Serialize/DcSerializeTypes.h
struct DATACONFIGCORE_API FDcSerializeContext
{
    //...
    FDcSerializer* Serializer = nullptr;
    FDcPropertyReader* Reader = nullptr;
    FDcWriter* Writer = nullptr;
    //...
};
```

Note how it takes an `FDcWriter` and a `FDcPropertyReader` - we're serializing data from the property system to arbitrary format.

Since serializer and deserializer have extremely similar APIs, we're showing examples using deserializer below from here. 

## Handlers

Say that we're deserializing a JSON object into a `USTRUCT` instance. The `FDcJsonReader` implements `ReadMapRoot()/ReadMapEnd()` but doesn't have `ReadStructRoot()/ReadStructEnd()`. To make the conversion we basically want to map `ReadMap()` and calls into `WriteStruct()` calls. This is where **handlers** come into play:

```c++
// DataConfigCore/Public/DataConfig/SerDe/DcDeserializeCommon.inl
FDcResult DcHandlerDeserializeMapToStruct(FDcDeserializeContext& Ctx)
{
    FDcStructAccess Access;
    DC_TRY(Ctx.Reader->ReadMapRoot());
    DC_TRY(Ctx.Writer->WriteStructRootAccess(Access));

    EDcDataEntry CurPeek;
    while (true)
    {
        DC_TRY(Ctx.Reader->PeekRead(&CurPeek));
        if (CurPeek == EDcDataEntry::MapEnd)
            break;

        FName FieldName;
        DC_TRY(Ctx.Reader->ReadName(&FieldName));
        DC_TRY(Ctx.Writer->WriteName(FieldName));

        DC_TRY(DcDeserializeUtils::RecursiveDeserialize(Ctx));
    }

    DC_TRY(Ctx.Reader->ReadMapEnd());
    DC_TRY(Ctx.Writer->WriteStructEnd());

    return DcOk();
}

// DataConfigCore/Private/DataConfig/Deserialize/Handlers/Json/DcJsonCommonDeserializers.cpp
FDcResult HandlerStructRootDeserialize(FDcDeserializeContext& Ctx)
{
    return DcHandlerDeserializeMapToStruct(Ctx);
}
```

Note that `Ctx.Reader` is a `FDcReader` that can be any derived class, while `Ctx.Writer` is always a `FDcPropertyWriter`. Deserialize handlers have an uniform signature:

```c++
using FDcDeserializeDelegateSignature = FDcResult(*)(FDcDeserializeContext& Ctx);
```

## Deserializer Setup

Note how deserialize handler above doesn't specify when it should be invoked. 
These info are described in `FDcDeserializer`:

```c++
// DataConfigCore/Public/DataConfig/Deserialize/DcDeserializer.h
struct DATACONFIGCORE_API FDcDeserializer : public FNoncopyable
{
    //...
    FDcResult Deserialize(FDcDeserializeContext& Ctx);

    void AddDirectHandler(FFieldClass* PropertyClass, FDcDeserializeDelegate&& Delegate);
    void AddDirectHandler(UClass* PropertyClass, FDcDeserializeDelegate&& Delegate);
    void AddPredicatedHandler(FDcDeserializePredicate&& Predicate, FDcDeserializeDelegate&& Delegate);
    //...
};
```

Comparing to `FDcDeserializeContext`, which describes data needed for a single run, 
`FDcDeserializer` contains info on what handlers to execute. Deserializer can also be reused across multiple
runs.

We use "direct handlers" to cover common cases:

```c++
// DataConfigCore/Private/DataConfig/Deserialize/DcDeserializerSetup.cpp
Deserializer.AddDirectHandler(FArrayProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerArrayDeserialize));
Deserializer.AddDirectHandler(FSetProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerSetDeserialize));
Deserializer.AddDirectHandler(FMapProperty::StaticClass(), FDcDeserializeDelegate::CreateStatic(HandlerMapDeserialize));
```

These basically says that "when running into array, set, map properties, use these provided handlers". 

Then there's "struct handler" that uses a `UStruct` that maps a specific class/struct
to a handler:

```c++
// DataConfigCore/Private/DataConfig/Deserialize/DcDeserializerSetup.cpp
Deserializer.AddStructHandler(TBaseStructure<FGuid>::Get(), FDcDeserializeDelegate::CreateStatic(HandlerGuidDeserialize));
Deserializer.AddStructHandler(TBaseStructure<FColor>::Get(), FDcDeserializeDelegate::CreateStatic(HandlerColorDeserialize));
Deserializer.AddStructHandler(TBaseStructure<FDateTime>::Get(), FDcDeserializeDelegate::CreateStatic(HandlerDateTimeDeserialize));
```

This means "when running into a `FGuid`, use these attached handlers". This is run before direct handlers.

Then we have "predicated handler" that get tested very early. This is how we allow custom conversion logic
setup for very specific class:

```c++
// DataConfigCore/Private/DataConfig/Deserialize/DcDeserializerSetup.cpp
EDcDeserializePredicateResult PredicateIsScalarArrayProperty(FDcDeserializeContext& Ctx)
{
    FProperty* Prop = CastField<FProperty>(Ctx.TopProperty().ToField());
    return Prop && Prop->ArrayDim > 1 && !Ctx.Writer->IsWritingScalarArrayItem()
        ? EDcDeserializePredicateResult::Process
        : EDcDeserializePredicateResult::Pass;
}

// ...
Deserializer.AddPredicatedHandler(
    FDcDeserializePredicate::CreateStatic(PredicateIsScalarArrayProperty),
    FDcDeserializeDelegate::CreateStatic(HandlerArrayDeserialize)
);
```

By convention the current deserializing property can be retrieved with `Ctx.TopProperty()`. `PredicateIsScalarArrayProperty` here
checks if it's wring a scalar array with non 1 dimension, if that's the case it would need to treat it like an array.

Note that all registered predicate handler is iterated through on every property, then proceed to handler on first success match or
fall through to struct/direct handlers when no match. Use it only when struct/direct handlers doesn't fit.

To recap:


| Handler Type      | Order  | Usage              | Execution                                     |
| :---------------- | ------ | ------------------ | --------------------------------------------- |
| Predicate handler | First  | Most flexible      | Iteration through all and match first success |
| Struct handler    | Second | "Is `FColor`? "    | Direct match                                  |
| Direct handler    | Last   | "Is `Map/Array`? " | Direct match                                  |

## Serializer Setup

Serializer has exactly the same API as [deserializer](#deserializer-setup) and the semantics are all the same.

## Sum Up

Serializer/Deserializer are built on top of Reader/Writer, to convert between Unreal Engine 
property system and external data formats.

- `FDcSerializeContext/FDcDeserializeContext` contains data.
- `FDcSerializer/FDcDeserializer` contains setup.
- Implement `FDcDeserializeDelegate/FDcSerializeDelegate` and `FDcDeserializePredicate/FDcSerializePredicate`
  pair for custom conversion logic.
