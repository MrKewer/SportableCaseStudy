#include "GBSNatsSubsystem.h"


UGBSNatsSubsystem::UGBSNatsSubsystem()
	: UEngineSubsystem()
{
}


void UGBSNatsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGBSNatsSubsystem::Deinitialize()
{
	// Do nothing for now
	Super::Deinitialize();
}

UGBSNatsConnectionSettings* UGBSNatsSubsystem::CreateConnectionSettings()
{
  UGBSNatsConnectionSettings* NewValue = NewObject<UGBSNatsConnectionSettings>(this);
  return NewValue;
}

UGBSNatsConnection* UGBSNatsSubsystem::CreateConnection(UGBSNatsConnectionSettings* ConnectionSettings, bool &Connected)
{
	UGBSNatsConnection* NewValue = NewObject<UGBSNatsConnection>(this);
	NewValue->SetSettings(ConnectionSettings);
	Connected = NewValue->Connect();
	return NewValue;
}

void UGBSNatsSubsystem::DestroyConnection(UGBSNatsConnection* Connection)
{
	UE_LOG(LogTemp, Log, TEXT("UGBSNatsSubsystem::DestroyConnection"));
	Connection->DestroyConnection();
}
