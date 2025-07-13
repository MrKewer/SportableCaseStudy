#include "GBSNatsConnectionSettings.h"

#include <vector>
#include "GBSNats.h"



void UGBSNatsConnectionSettings::connectedCB(natsConnection* nc, void*)
{

	char buffer[256] = { 0 };
	natsConnection_GetConnectedUrl(nc, buffer, sizeof(buffer));
	const FString message = ANSI_TO_TCHAR(buffer);
	UE_LOG(LogGBSNats, Log, TEXT("UGBSNatsConnectionSettings::disconnectedCB Url: %s"), *message)

	// Only called if initial connection did not work
	UE_LOG(LogGBSNats, Warning, TEXT("UGBSNatsConnectionSettings::connectedCB Nats connected"));
}

void UGBSNatsConnectionSettings::errorCB(natsConnection* conn, natsSubscription* sub, natsStatus s, void* closure)
{
	UGBSNatsConnectionSettings* _this = static_cast<UGBSNatsConnectionSettings*>(closure);
	int errorCount = 0;
	int droppedCount = 0;
	if (_this) {
		errorCount = _this->_asyncErrors.count++;
		natsSubscription_GetDropped(sub, (int64_t*)&_this->_dropped);
		droppedCount = _this->_dropped;
	}

	const FString ErrorMessage = ANSI_TO_TCHAR(natsStatus_GetText(s)); // nats_GetLastError
	UE_LOG(LogGBSNats, Error, TEXT("UGBSNatsConnectionSettings::errorCB Async errors#: %d dropped#: %d Status Error: %d - %s"), 
		errorCount, droppedCount, s, *ErrorMessage);
}

// This callback could be used to see who we are connected to on reconnect
void UGBSNatsConnectionSettings::disconnectedCB(natsConnection* nc, void* closure)
{
	const char* errStr;
	natsStatus	retStatus = natsConnection_GetLastError(nc, &errStr);
	const FString messageError = ANSI_TO_TCHAR(errStr);
	UE_LOG(LogGBSNats, Log, TEXT("UGBSNatsConnectionSettings::disconnectedCB Last error: %d, messageError: %s"), retStatus,  *messageError);
}

void UGBSNatsConnectionSettings::reconnectedCB(natsConnection* nc, void* closure)
{
	// Define a buffer to receive the url
	char buffer[256] = { 0 };
	natsConnection_GetConnectedUrl(nc, buffer, sizeof(buffer));
	const FString message = ANSI_TO_TCHAR(buffer);
	UE_LOG(LogGBSNats, Warning, TEXT("UGBSNatsConnectionSettings::reconnectedCB Url: %s"), *message);
}
void UGBSNatsConnectionSettings::closedCB(natsConnection* nc, void* closure)
{
	// Define a buffer to receive the url
	char buffer[256] = { 0 };
	natsConnection_GetConnectedUrl(nc, buffer, sizeof(buffer));
	const FString message = ANSI_TO_TCHAR(buffer);
	UE_LOG(LogGBSNats, Warning, TEXT("UGBSNatsConnectionSettings::closedCB Url: %s"), *message);
}



UGBSNatsConnectionSettings::UGBSNatsConnectionSettings(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	natsStatus	retStatus = NATS_OK;
	// TODO this is being leaked; destroy it later, but when?
	retStatus = natsOptions_Create(&this->opts);
	UE_LOG(LogGBSNats, Log, TEXT("UGBSNatsConnectionSettings : %d"), (int)retStatus);


#ifdef USE_LATEST_NATS_IMPROVEMENTS

	if (retStatus == NATS_OK)
		retStatus = natsOptions_SetMaxReconnect(this->opts, -1); // Unlimited

	//if (retStatus == NATS_OK)
	//	retStatus = natsOptions_SetReconnectWait(this->opts, RECONNECT_WAIT_TIME_MS);

	if (retStatus == NATS_OK)
		retStatus = natsOptions_SetRetryOnFailedConnect(this->opts, true, &connectedCB, nullptr);

	// Initialize this object
	_asyncErrors = {0};

	// Set a callback for asynchronous errors. This is useful when having an asynchronous
	// subscriber, which would otherwise have no other way of reporting an error.
	if (retStatus == NATS_OK)
		retStatus = natsOptions_SetErrorHandler(this->opts, errorCB, (void*)this);

	// Setup a callback to be notified on disconnects...
	if (retStatus == NATS_OK)
		retStatus = natsOptions_SetDisconnectedCB(this->opts, disconnectedCB, nullptr);

	// And on reconncet
	if (retStatus == NATS_OK)
		retStatus = natsOptions_SetReconnectedCB(this->opts, reconnectedCB, nullptr);

	if (retStatus == NATS_OK)
		retStatus = natsOptions_SetClosedCB(this->opts, closedCB, nullptr);

	// You can do this, or just specify these on the command line
	// natsOptions_SetUserInfo(opts, "admin", "admin");

	// Causes timeouts but should reduce latency from req rep server...
	if (retStatus == NATS_OK)
		retStatus = natsOptions_SetSendAsap(this->opts, true); // reducing Latency

	if (retStatus == NATS_OK) {

		// H.265 with 4k (4196x2016) 100kb I frame + (4 x 50kb P/B Frames) ~300Kb
		// JPEG with 2k (1920x1080) 2x 200kb I frame ~400Kb
		int size = (int) ((float)1024 * 1024 * 0.4);
		UE_LOG(LogGBSNats, Log, TEXT("UGBSNatsConnectionSettings::UGBSNatsConnectionSettings natsOptions_SetIOBufSize: %d"), size);
		retStatus = natsOptions_SetIOBufSize(this->opts, size); // ~300Kb, default is 32KB.
	}

#endif // !USE_LATEST_NATS_IMPROVEMENTS

}

UGBSNatsConnectionSettings::~UGBSNatsConnectionSettings()
{
	if (this->opts)
		natsOptions_Destroy(this->opts);
}

void UGBSNatsConnectionSettings::AddServer(const FString& Server)
{
  Servers.Add(Server);
}

void UGBSNatsConnectionSettings::SetExpectedHostName(const FString& HostName)
{
  // TODO Handle errors
  natsOptions_SetExpectedHostname(this->opts, TCHAR_TO_ANSI(*HostName));
}

bool UGBSNatsConnectionSettings::SetCredentialsFile(const FString& FilePath)
{
	// TODO Handle errors
	auto s = natsOptions_SetUserCredentialsFromFiles(this->opts, TCHAR_TO_ANSI(*FilePath), NULL);
	return (s == natsStatus::NATS_OK);
}


void UGBSNatsConnectionSettings::SkipServerVerification(const bool Skip)
{
  // TODO Handle errors
  natsOptions_SkipServerVerification(this->opts, Skip);
}

void UGBSNatsConnectionSettings::SetSecure(const bool Secure)
{
	natsOptions_SetSecure(this->opts, Secure);
}

void UGBSNatsConnectionSettings::SetHostName(const FString HostName)
{
	natsOptions_SetExpectedHostname(this->opts, TCHAR_TO_ANSI(*HostName));
}

void UGBSNatsConnectionSettings::SetConnectionName(const FString ConnectionName)
{
	natsOptions_SetName(this->opts, TCHAR_TO_ANSI(*ConnectionName));
}

FString UGBSNatsConnectionSettings::GetConnectionString()
{
  FString connectionString = FString::Join(this->Servers, TEXT(", "));
  return connectionString;
}

natsOptions* UGBSNatsConnectionSettings::GetOptions()
{
  FString ConnectionString = *this->GetConnectionString();
  UE_LOG(LogGBSNats, Log, TEXT("GetOptions() Connecting to %s"), *ConnectionString);

  natsOptions_SetURL(this->opts, TCHAR_TO_ANSI(*ConnectionString));
  return this->opts;
}
