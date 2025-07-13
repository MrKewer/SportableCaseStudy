#pragma once

#ifdef USE_NATS
#include "nats/nats.h"
#endif

#include "Misc/DateTime.h"
#include "GBSNatsSubscription.generated.h"

/** Generate a delegate for callback events */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnMessage, FString, Subject, FString, Message, const int64&, UnixTime);

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnByteMessage, const FString&, Subject, const TArray<uint8>&, Message, const int64&, UnixTime);

UENUM(BlueprintType)
enum class MessageLogLevel : uint8 {
	OFF = 0 UMETA(DisplayName = "OFF"),
	LIGHT = 1 UMETA(DisplayName = "LIGHT"),
	VERBOSE = 2 UMETA(DisplayName = "VERBOSE")
};

UCLASS(BlueprintType, Blueprintable)
class GBSNATS_API UGBSNatsSubscription : public UObject
{
  GENERATED_UCLASS_BODY()

  //////////////////////////////////////////////////////
  // The fun stuff goes here
public:
  UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|PubSub")
  void Unsubscribe();

  UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|Settings")
	void SetMessageLogLevel(const MessageLogLevel setting);

  UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|Settings")
	  void SetMessageLogInterval(const FTimespan interval);
  
  //////////////////////////////////////////////////////
  // Internal use only, not exposed to blueprints.
public:
#ifdef USE_NATS
  void SetConnection(natsConnection* nc);
#endif

  void SetSubject(const FString& Subject);
  void SetDelegate(const FOnMessage& Delegate);

  void SetDelegate(const FOnByteMessage& Delegate);
  
  bool DoSubscription(const FString& subject, FString& ResultEnumValue);
  void InternalOnMessage(const FString& Subject, const FString& Message);

  void InternalByteMessage(const FString& Subject, const TArray<uint8>& Message) const;

  void SetUnixTime(const int64 Time) { UnixTime = Time; }
  
  //////////////////////////////////////////////////////
  // 'Structors
  virtual ~UGBSNatsSubscription();

  MessageLogLevel logLevel;
  FDateTime currentTime;
  FTimespan m_Interval;

public:
  //UPROPERTY(BlueprintReadWrite, Category = "Variable")
  //bool bIsReceivingMessage = false;
  UPROPERTY(BlueprintReadWrite, Category = "Variable")
  bool bByteMessage = false;

public:

  int64_t			_start = 0;
  int64_t			_last = 0;
  int64_t			_total = 0;
  int64_t			_messageCounter = 0;
  int64_t			_messageCounterB = 0;
  //int64_t			_messageCounterC = 0;
  volatile int64_t	_count = 0;
  volatile int64_t	_dropped = 0;
  volatile int64_t	_elapsed = 0;

  natsStatistics*	_stats = NULL;

private:

  FString m_Subject;
  FOnMessage OnMessage;
  FOnByteMessage OnByteMessage;
  int64 UnixTime;

#ifdef USE_NATS
  natsConnection* natsConn;
  natsSubscription* natsSub;
#endif


  static void onComplete(void* context);


};
