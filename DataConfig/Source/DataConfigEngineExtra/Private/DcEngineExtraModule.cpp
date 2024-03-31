#include "DcEngineExtraModule.h"

#include "MessageLogModule.h"
#include "Logging/MessageLog.h"
#include "GameplayTagsManager.h"

#include "DataConfig/DcEnv.h"
#include "DataConfig/Extra/Diagnostic/DcDiagnosticExtra.h"
#include "DataConfig/EngineExtra/Diagnostic/DcDiagnosticEngineExtra.h"


struct FDcMessageLogDiagnosticConsumer : public IDcDiagnosticConsumer
{
	void HandleDiagnostic(FDcDiagnostic& Diag) override
	{
		FMessageLog MessageLog("DataConfig");
		MessageLog.Message(EMessageSeverity::Error, FText::FromString(DcDiagnosticToString(Diag)));
	}
};


static void _PopulateEngineExtraGameplayTagFixtures()
{
	UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("DataConfig.Foo.Bar"));
	UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("DataConfig.Foo.Bar.Baz"));
	UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("DataConfig.Tar.Taz"));
}

void FDcEngineExtraModule::StartupModule()
{
	UE_LOG(LogDataConfigCore, Log, TEXT("DcEngineExtraModule module starting up"));
	DcRegisterDiagnosticGroup(&DcDExtra::Details);
	DcRegisterDiagnosticGroup(&DcDEngineExtra::Details);

	_PopulateEngineExtraGameplayTagFixtures();

	DcStartUp(EDcInitializeAction::Minimal);
	DcEnv().DiagConsumer = MakeShareable(new FDcMessageLogDiagnosticConsumer());

	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions InitOptions;
	InitOptions.bAllowClear = true;
	MessageLogModule.RegisterLogListing("DataConfig", FText::FromString(TEXT("DataConfig")), InitOptions);
}

void FDcEngineExtraModule::ShutdownModule()
{
	UE_LOG(LogDataConfigCore, Log, TEXT("DcEngineExtraModule module shutting down"));

	DcShutDown();

	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	MessageLogModule.UnregisterLogListing("DataConfig");
}

IMPLEMENT_MODULE(FDcEngineExtraModule, DataConfigEngineExtra);
