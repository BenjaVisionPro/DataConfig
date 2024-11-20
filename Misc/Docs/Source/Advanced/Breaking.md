# Breaking Changes

On this page we'll document breaking changes across versions.

# 1.7.1

- `FDcPropertyWriter` will now empty `TArray/TSet/TMap` on write.

   Previously when running into container property with default values DataConfig will thrash the defaults also leaving the container with incorrect elements count. Now the containers will be properly emptied on write.

   ```c++
   // Example of TArray property with default values.
   USTRUCT()
   struct FDcTestStructWithDefaults
   {
       // ...
       UPROPERTY() TArray<FString> StrArrFieldWithDefault = {"Foo", "Bar", "Baz"};
   };
   ```

   Though this is a behavior change but it should have minimal impact. See `DataConfig.Core.Property.Defaults` test for details.

# 1.7.0

- UE 5.5 deprecated `StructUtils` plugin, thus we'll need to setup multiple uplugins for UE5. See: [DataConfigXX.uplugin](../Integration.md#dataconfigxxuplugin)


# 1.6.0

-  Added a optional `FName` to `FDcSerializer/FDcDeserializer::AddPredicatedHandler` which can be used to identify an entry so one can replace it later like this:

   ```c++
   Ctx.Deserializer->PredicatedDeserializers.FindByPredicate([](auto& Entry){
   return Entry.Name == FName(TEXT("Enum"));
   })->Handler = FDcDeserializeDelegate::CreateStatic(DcEngineExtra::HandlerBPEnumDeserialize);
   ```

-  Removed `DcDeserializeUtils/DcSerializeUtils::PredicateIsUStruct`. Use `FDcSerializer/FDcDeserializer::AddStructHandler` instead. It's more accurate and overall gets better performance.

   For an concrete example, change this:

   ```c++
   Ctx.Deserializer->AddPredicatedHandler(
       FDcDeserializePredicate::CreateStatic(DcDeserializeUtils::PredicateIsUStruct<FMsgPackTestSuite>),
       FDcDeserializeDelegate::CreateStatic(HandlerMsgPackTestSuite)
   );
   ```

   to this:

   ```c++
   Ctx.Deserializer->AddStructHandler(
       TBaseStructure<FMsgPackTestSuite>::Get(),
       FDcDeserializeDelegate::CreateStatic(HandlerMsgPackTestSuite)
   );
   ```

   and that's done. For more details see: [Serializer Deserializer Setup](../Programming/SerializerDeserializer.md#deserializer-setup).

-  Move many `DataConfigEditorExtra` content to the new `DataConfigEngineExtra` runtime module so it can be used at runtime. This is mostly for the blueprint nodes to be available at engine runtime.


# 1.5.0

-  Renamed `EDcDataEntry::Nil` to `EDcDataEntry::None`. This is a fix for MacOS build.
