# Breaking Changes

On this page we'll document breaking changes across versions.

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
