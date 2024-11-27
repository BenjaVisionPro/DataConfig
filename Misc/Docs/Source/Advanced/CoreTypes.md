# UE Core Types Handlers


We bundled some custom serialization handlers for common Unreal Engine types (think `FText/FVector/FColor`) to improve developer experience. You need to explicitly set it up with your serializer/deserializers.

```c++
//  setup core types serializers
FDcSerializer Serializer;
DcSetupCoreTypesSerializeHandlers(Serializer);

//  setup core types deserializers
FDcDeserializer Deserializer;
DcSetupCoreTypesDeserializeHandlers(Deserializer);
```

These are also used in [DataConfig blueprint nodes][2]. 

Note that you always can custom these default behavior [by writing your own handlers][1].


## FText

`FText` is used to handle localization in UE and it's quite different from `FString`. When calling `ReadText()` on JSON/MsgPack reader
we pipe the string through a helper method `FTextStringHelper::CreateFromBuffer()` which parses and handles `NSLOCTEXT()/LOCTABLE()` style string.


```json
//  DataConfigTests/Private/DcTestProperty5.cpp
{
    "TextFieldAlpha" : "NSLOCTEXT(\"DataConfig.Tests\", \"BAR_TEST\", \"Localize String Bar\")",
    "TextFieldBeta" : "LOCTABLE(\"/Bogus/Path/To/Table\", \"FOO_TEST\")"
}
```

The escaping sequences in JSON makes it a bit difficult to use. With the core types handlers we also accept arrays:

```json
//  DataConfigTests/Private/DcTestRoundtrip2.cpp
{
    // [<namespace>, <text-id>, <source>], same as `NSLOCTEXT()`
    "TextFieldAlpha" : [
        "DataConfig.Tests",
        "BAR_TEST",
        "Localize String Bar"
    ],
    // [<string-table-id>, <text-id>], same as `LOCTABLE()`
    "TextFieldBeta" : [
        "/Bogus/Path/To/Table",
        "FOO_TEST"
    ]
}
```

## Compact Presentation

Without any special handling a `FVector` is serialized as a JSON object like this:


```JSON
{
    "X" : 1,
    "Y" : 2,
    "Z" : 3,
}
```

This is fine but sometimes we want it to be more compact. DataConfig now comes with 
a set of serialize/deserialize handlers that writes the commonly used types in compact form.

With a struct like this:

```c++
USTRUCT()
struct DATACONFIGEXTRA_API FDcExtraCoreTypesStruct
{
    GENERATED_BODY()

    UPROPERTY() FGuid GuidField1;
    UPROPERTY() FGuid GuidField2;

    UPROPERTY() FVector2D Vec2Field1;
    UPROPERTY() FVector2D Vec2Field2;

    UPROPERTY() FVector VecField1;
    UPROPERTY() FVector VecField2;

    UPROPERTY() FPlane PlaneField1;
    UPROPERTY() FPlane PlaneField2;

    UPROPERTY() FMatrix MatrixField1;
    UPROPERTY() FMatrix MatrixField2;

    UPROPERTY() FBox BoxField1;
    UPROPERTY() FBox BoxField2;

    UPROPERTY() FRotator RotatorField1;
    UPROPERTY() FRotator RotatorField2;

    UPROPERTY() FQuat QuatField1;
    UPROPERTY() FQuat QuatField2;

    UPROPERTY() FTransform TransformField1;
    UPROPERTY() FTransform TransformField2;

    UPROPERTY() FColor ColorField1;
    UPROPERTY() FColor ColorField2;

    UPROPERTY() FLinearColor LinearColorField1;
    UPROPERTY() FLinearColor LinearColorField2;

    UPROPERTY() FIntPoint IntPointField1;
    UPROPERTY() FIntPoint IntPointField2;

    UPROPERTY() FIntVector IntVectorField1;
    UPROPERTY() FIntVector IntVectorField2;

    UPROPERTY() FDateTime DateTimeField1;
    UPROPERTY() FDateTime DateTimeField2;

    UPROPERTY() FTimespan TimespanField1;
    UPROPERTY() FTimespan TimespanField2;

};
```

It would be serialized like this:

```json
{
    "GuidField1" : [       1,        2,        3,        4],
    "GuidField2" : [       0,        0,        0,        0],
    "Vec2Field1" : [        0,         1],
    "Vec2Field2" : [        0,         0],
    "VecField1" : [        0,         0,         1],
    "VecField2" : [        0,         0,         0],
    "PlaneField1" : [        1,         2,         3,         4],
    "PlaneField2" : [        0,         0,         0,         0],
    "MatrixField1" : [
        [        1,         0,         0,         0],
        [        0,         1,         0,         0],
        [        0,         0,         1,         0],
        [        0,         0,         0,         1]
    ],
    "MatrixField2" : [
        [        0,         0,         0,         0],
        [        0,         0,         0,         0],
        [        0,         0,         0,         0],
        [        0,         0,         0,         0]
    ],
    "BoxField1" : {
        "Min" : [        0,         0,         0],
        "Max" : [        0,         0,         1],
        "IsValid" : 1
    },
    "BoxField2" : {
        "Min" : [        1,         1,         1],
        "Max" : [        0,         0,        -1],
        "IsValid" : 1
    },
    "RotatorField1" : [        1,         2,         3],
    "RotatorField2" : [        0,         0,         0],
    "QuatField1" : [        1,         2,         3,         4],
    "QuatField2" : [        0,         0,         0,         0],
    "TransformField1" : {
        "Rotation" : [        0,         0,         0,         1],
        "Translation" : [        1,         2,         3],
        "Scale3D" : [        1,         1,         1]
    },
    "TransformField2" : {
        "Rotation" : [        0,         0,         0,         1],
        "Translation" : [        0,         0,         0],
        "Scale3D" : [        1,         1,         1]
    },
    "ColorField1" : "#000000FF",
    "ColorField2" : "#0000FFFF",
    "LinearColorField1" : [        0,         0,         0,         1],
    "LinearColorField2" : [        1,         1,         1,         1],
    "IntPointField1" : [       1,        2],
    "IntPointField2" : [       0,        0],
    "IntVectorField1" : [       1,        2,        3],
    "IntVectorField2" : [       0,        0,        0],
    "DateTimeField1" : "0001.01.01-00.00.00",
    "DateTimeField2" : "1988.07.23-00.00.00",
    "TimespanField1" : "+00:00:00.000",
    "TimespanField2" : "+5.06:07:08.000"
}
```

This would make large JSON dumps more readable. Deserialize also works and it supports both array and object form.


[1]:./WritingHandlers.md "Writing Handlers"
[2]:../Extra/BlueprintNodes.md "Blueprint Nodes"
