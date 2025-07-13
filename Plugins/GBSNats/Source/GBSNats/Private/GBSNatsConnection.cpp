#include "GBSNatsConnection.h"

#include "GBSNatsSubscription.h"

#include "nats/nats.h"
#include <string>






UGBSNatsConnection::UGBSNatsConnection(const class FObjectInitializer &PCIP)  : Super(PCIP)
{
}

UGBSNatsConnection::~UGBSNatsConnection()
{
#ifdef USE_NATS

    if (this->natsConn) {
        UE_LOG(LogGBSNats, Log, TEXT("UGBSNatsConnection::~UGBSNatsConnection() natsConnection_Destroy"));
        natsConnection_Destroy(this->natsConn);
    }

    // To silence reports of memory still in used with valgrind
    nats_Close();

#endif
}

void UGBSNatsConnection::SetSettings(UGBSNatsConnectionSettings *Settings)
{
  this->ConnectionSettings = Settings;
}

bool UGBSNatsConnection::Connect()
{
  natsStatus s = NATS_OK;
  s = natsConnection_Connect(&this->natsConn, this->ConnectionSettings->GetOptions());

  int value = static_cast<int>(s);
  status = static_cast<ENatsStatus>(value);

  if (s != NATS_OK)
  {
      FString ErrorMessage = ANSI_TO_TCHAR(natsStatus_GetText(s));
      UE_LOG(LogGBSNats, Warning, TEXT("UGBSNatsConnection::Connect() Error connecting to nats server: %s"), *ErrorMessage);
      return false;
  }

  char buffer[256] = { 0 };
  natsConnection_GetConnectedUrl(this->natsConn, buffer, sizeof(buffer));
  const FString message = ANSI_TO_TCHAR(buffer);
  UE_LOG(LogGBSNats, Log, TEXT("UGBSNatsConnection::Connect() natsConnection_GetConnectedUrl Url: %s"), *message);

  return true;
}

void UGBSNatsConnection::DestroyConnection()
{
  
  if (this->natsConn != nullptr)
  {
    natsConnection_Destroy(natsConn);
    natsConn = nullptr;
    UE_LOG(LogTemp, Log, TEXT("Disconnected from NATS"));
  }
}

ENatsStatus UGBSNatsConnection::GetStatus()
{
    return status;
}

UGBSNatsSubscription *UGBSNatsConnection::Subscribe(const FString& Subject, const FOnMessage& OnMessage, bool& FoundSubject, FString &ReturnEnumValue)
{
  UGBSNatsSubscription* sub = NewObject<UGBSNatsSubscription>(this);

  sub->SetSubject(Subject);
  sub->SetDelegate(OnMessage);
  sub->SetConnection(this->natsConn);

  FoundSubject = sub->DoSubscription(Subject, ReturnEnumValue);

  return sub;
}

UGBSNatsSubscription* UGBSNatsConnection::SubscribeByte(const FString& Subject, const FOnByteMessage& OnMessage,
  bool& FoundSubject, FString &ReturnEnumValue)
{
  UGBSNatsSubscription* sub = NewObject<UGBSNatsSubscription>(this);

  sub->SetSubject(Subject);
  sub->SetDelegate(OnMessage);
  sub->SetConnection(this->natsConn);

  FoundSubject = sub->DoSubscription(Subject, ReturnEnumValue);

  return sub;
}

void UGBSNatsConnection::Publish(const FString& Subject, const FString& Message)
{
  natsConnection_PublishString(this->natsConn, TCHAR_TO_ANSI(*Subject), TCHAR_TO_ANSI(*Message));
}

void UGBSNatsConnection::PublishData(const FString& Subject, const TArray<uint8>& Data)
{
    natsConnection_Publish(this->natsConn, TCHAR_TO_ANSI(*Subject), static_cast<const void*>(Data.GetData()), Data.Num());
}

FString UGBSNatsConnection::RequestString(const FString& Subject, const FString& Request)
{
  FString reply;

  natsMsg* msg;
  // TODO Don't hard-code the timeout.
  int64_t timeout = 1000;

  auto s = natsConnection_RequestString(&msg, this->natsConn, TCHAR_TO_ANSI(*Subject), TCHAR_TO_ANSI(*Request), timeout);
  if (s == NATS_OK)
  {
    std::string cstr(reinterpret_cast<const char*>(natsMsg_GetData(msg)), natsMsg_GetDataLength(msg));
    reply = (cstr.c_str());
    natsMsg_Destroy(msg);
  }

  return reply;
}

UGBSNatsSubscription* UGBSNatsConnection::PublishRequestString(const FString& Subject, const FString& ReplySubject, const FString& Request)
{
  UGBSNatsSubscription* sub = NewObject<UGBSNatsSubscription>(this);

  sub->SetSubject(ReplySubject);

  natsConnection_PublishRequestString(this->natsConn, TCHAR_TO_ANSI(*Subject), TCHAR_TO_ANSI(*ReplySubject), TCHAR_TO_ANSI(*Request));

  return sub;
}

ENatsStatus UGBSNatsConnection::CheckConnectionStatus()
{
    return status;
}
