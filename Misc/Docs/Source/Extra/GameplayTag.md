# Gameplay Tag Serialization/Deserialization

[GameplayTags][1] is a built-in runtime module that implements hierarchical tags. 

In this example we have roundtrip handlers for `FGameplayTag` and `FGameplayTagContainer`.

* [DcSerDeGameplayTags.h]({{SrcRoot}}DataConfigEngineExtra/Public/DataConfig/EngineExtra/SerDe/DcSerDeGameplayTags.h)
* [DcSerDeGameplayTags.cpp]({{SrcRoot}}DataConfigEngineExtra/Private/DataConfig/EngineExtra/SerDe/DcSerDeGameplayTags.cpp)

```c++
// DataConfigEngineExtra/Public/DataConfig/EngineExtra/SerDe/DcSerDeGameplayTags.h
USTRUCT()
struct FDcEngineExtraTestStructWithGameplayTag1
{
    GENERATED_BODY()

    UPROPERTY() FGameplayTag TagField1;
    UPROPERTY() FGameplayTag TagField2;
};

// DataConfigEngineExtra/Private/DataConfig/EngineExtra/SerDe/DcSerDeGameplayTags.cpp
FString Str = TEXT(R"(
    {
        "TagField1" : null,
        "TagField2" : "DataConfig.Foo.Bar"
    }
)");
```

 `FGameplayTagContainer` converts to a list of strings:

```c++
// DataConfigEngineExtra/Public/DataConfig/EngineExtra/Deserialize/DcDeserializeGameplayTags.h
USTRUCT()
struct FDcEngineExtraTestStructWithGameplayTag2
{
    GENERATED_BODY()

    UPROPERTY() FGameplayTagContainer TagContainerField1;
    UPROPERTY() FGameplayTagContainer TagContainerField2;
};

// DataConfigEngineExtra/Private/DataConfig/EngineExtra/Deserialize/DcDeserializeGameplayTags.cpp
FString Str = TEXT(R"(
    {
        "TagContainerField1" : [],
        "TagContainerField2" : [
            "DataConfig.Foo.Bar",
            "DataConfig.Foo.Bar.Baz",
            "DataConfig.Tar.Taz",
        ]
    }
)");
```

Note that gameplay tag parsing has error reporting built-in. In this case we can pipe it into our diagnostic:

```c++
// DataConfigEngineExtra/Private/DataConfig/EngineExtra/Deserialize/DcDeserializeGameplayTags.cpp
static FDcResult _StringToGameplayTag(FDcDeserializeContext& Ctx, const FString& Str, FGameplayTag* OutTagPtr)
{
    FString FixedString;
    FText Err;
    if (!FGameplayTag::IsValidGameplayTagString(Str, &Err, &FixedString))
    {
        return DC_FAIL(DcDEngineExtra, InvalidGameplayTagStringFixErr)
            << Str << FixedString << Err;
    }
    //...
}
```

In case of a invalid tag it would report the reason and fixed string:

```
# DataConfig Error: Invalid Gameplay Tag String, Actual: 'DataConfig.Invalid.Tag.', Fixed: 'DataConfig.Invalid.Tag', Error: 'Tag ends with .'
- [JsonReader] --> <in-memory>5:4
   3 |        { 
   4 |            "TagField1" : null, 
   5 |            "TagField2" : "DataConfig.Invalid.Tag." 
     |                          ^^^^^^^^^^^^^^^^^^^^^^^^^
   6 |        } 
   7 |    
- [PropertyWriter] Writing property: (FDcEngineExtraTestStructWithGameplayTag1)$root.(FGameplayTag)TagField2
 [C:\DevUE\UnrealEngine\Engine\Source\Developer\MessageLog\Private\Model\MessageLogListingModel.cpp(73)]
```

[1]: https://docs.unrealengine.com/en-US/ProgrammingAndScripting/Tags/index.html "Gameplay Tags"