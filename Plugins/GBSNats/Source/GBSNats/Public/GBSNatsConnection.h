#pragma once

#if PLATFORM_WINDOWS
#ifdef USE_NATS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/prewindowsapi.h"
#endif // USE_NATS
#endif // PLATFORM_WINDOWS

#include "GBSNats.h"
#include "GBSNatsConnectionSettings.h"
#include "GBSNatsSubscription.h"

#ifdef USE_NATS
#include "nats/nats.h"
#endif

#if PLATFORM_WINDOWS
#ifdef USE_NATS
#include "Windows/HideWindowsPlatformTypes.h"
#include "Windows/PostWindowsApi.h"
#endif // USE_NATS
#endif // PLATFORM_WINDOWS

#include "GBSNatsConnection.generated.h"


UCLASS(BlueprintType, Blueprintable)
class GBSNATS_API UGBSNatsConnection : public UObject
{
  GENERATED_UCLASS_BODY()

  //////////////////////////////////////////////////////
  // The fun stuff goes here
public:

	virtual ~UGBSNatsConnection();

  UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|PubSub")
  UGBSNatsSubscription* Subscribe(const FString& Subject, const FOnMessage& OnMessage, bool &FoundSubject, FString &ReturnEnumValue);

  UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|PubSub")
  UGBSNatsSubscription* SubscribeByte(const FString& Subject, const FOnByteMessage& OnMessage, bool &FoundSubject, FString &ReturnEnumValue);

  UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|PubSub")
  void Publish(const FString& Subject, const FString& Message);

  UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|PubSub")
  void PublishData(const FString& Subject, const TArray<uint8>& Data);

  UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|ReqRep")
  FString RequestString(const FString& Subject, const FString& Request);

  UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|ReqRep")
  UGBSNatsSubscription* PublishRequestString(const FString& Subject, const FString& ReplySubject, const FString& Request);

  UFUNCTION(BlueprintPure, BlueprintCallable, Category = "GBSNats", meta=(DefaultToSelf=Target))
  ENatsStatus CheckConnectionStatus();

  //////////////////////////////////////////////////////
  // Internal use only, not exposed to blueprints.
public:
  void SetSettings(UGBSNatsConnectionSettings* ConnectionSettings);
  bool Connect();
  void DestroyConnection();
  ENatsStatus GetStatus();

protected:

  UPROPERTY()
  UGBSNatsConnectionSettings* ConnectionSettings;

private:

#ifdef USE_NATS
  natsConnection *natsConn;
#endif

  ENatsStatus status;





};
