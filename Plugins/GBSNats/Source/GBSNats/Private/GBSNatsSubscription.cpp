#include "GBSNatsSubscription.h"
#include <string>
#include "Async/TaskGraphInterfaces.h"
#include "Async/Async.h"
#include <chrono>

#include "GBSNats.h"



#define STATS_IN        0x1
#define STATS_OUT       0x2
#define STATS_COUNT     0x4

static natsStatus printStats(int mode, natsConnection* conn, natsSubscription* sub, natsStatistics* stats)
{
	natsStatus  s = NATS_OK;
	uint64_t    inMsgs = 0;
	uint64_t    inBytes = 0;
	uint64_t    outMsgs = 0;
	uint64_t    outBytes = 0;
	uint64_t    reconnected = 0;
	int         pending = 0;
	int64_t     delivered = 0;
	int64_t     sdropped = 0;

	s = natsConnection_GetStats(conn, stats);
	if (s == NATS_OK)
		s = natsStatistics_GetCounts(stats, &inMsgs, &inBytes,
			&outMsgs, &outBytes, &reconnected);

	if ((s == NATS_OK) && (sub != NULL))
	{
		s = natsSubscription_GetStats(sub, &pending, NULL, NULL, NULL,
			&delivered, &sdropped);

		// Since we use AutoUnsubscribe(), when the max has been reached,
		// the subscription is automatically closed, so this call would
		// return "Invalid Subscription". Ignore this error.
		if (s == NATS_INVALID_SUBSCRIPTION)
		{
			s = NATS_OK;
			pending = 0;
		}
	}
	if (s == NATS_OK)
	{
		if (mode & STATS_IN)
		{
			UE_LOG(LogGBSNats, VeryVerbose, TEXT("printStats In Msgs: %llu - In Bytes: %llu"), inMsgs, inBytes);
		}
		if (mode & STATS_OUT)
		{
			UE_LOG(LogGBSNats, VeryVerbose, TEXT("printStats Out Msgs: %llu - Out Bytes: %llu"), outMsgs, outBytes);
		}
		if (mode & STATS_COUNT)
		{
			UE_LOG(LogGBSNats, VeryVerbose, TEXT("printStats Delivered: %llu - Dropped: %llu, Pending: %d"), delivered, sdropped, pending);
		}
		UE_LOG(LogGBSNats, VeryVerbose, TEXT("printStats Reconnected: %llu"), reconnected);
	}

	return s;
}

static void printPerf(const FString& perfTxt, int64_t start, int64_t elapsed, int64_t count)
{
	if ((start > 0) && (elapsed == 0))
		elapsed = nats_Now() - start;

	if (elapsed <= 0) {
		UE_LOG(LogGBSNats, VeryVerbose, TEXT("printPerf %s, Not enough messages or too fast to report performance!"),
			*perfTxt, count);
	}
	else {
		UE_LOG(LogGBSNats, VeryVerbose, TEXT("printPerf %s, %lld messages in %lld milliseconds (%d msgs/sec)"),
			*perfTxt, count, elapsed, (int)((count * 1000) / elapsed));
	}
}



UGBSNatsSubscription::UGBSNatsSubscription(const class FObjectInitializer &PCIP)
    : Super(PCIP)
{
}

UGBSNatsSubscription::~UGBSNatsSubscription()
{
#ifdef USE_NATS
  this->Unsubscribe();

  // Destroy all our objects to avoid report of memory leak
  if (this->_stats)
	natsStatistics_Destroy(this->_stats);


#endif
}

void UGBSNatsSubscription::SetSubject(const FString &Subject)
{
  this->m_Subject = Subject;
  this->currentTime = this->currentTime.Now();
}

void UGBSNatsSubscription::SetDelegate(const FOnMessage &Delegate)
{
  this->OnMessage = Delegate;
}

void UGBSNatsSubscription::SetDelegate(const FOnByteMessage& Delegate)
{
	this->OnByteMessage = Delegate;
}

void UGBSNatsSubscription::Unsubscribe()
{
#ifdef USE_NATS
  if (this->natsSub != nullptr)
  {
      natsSubscription_Unsubscribe(this->natsSub);
	  UE_LOG(LogGBSNats, VeryVerbose, TEXT("UGBSNatsSubscription::~Unsubscribe() natsSubscription_Destroy"));
	  natsSubscription_Destroy(this->natsSub);
      this->natsSub = nullptr;
  }
#endif
}

void UGBSNatsSubscription::SetMessageLogLevel(const MessageLogLevel setting)
{
    this->logLevel = setting;
}

void UGBSNatsSubscription::SetMessageLogInterval(const FTimespan interval)
{
    this->m_Interval = interval;
    this->currentTime = this->currentTime.Now() - (interval * 10);

}

#ifdef USE_NATS
void UGBSNatsSubscription::SetConnection(natsConnection *nc)
{
  this->natsConn = nc;
}
#endif

void UGBSNatsSubscription::InternalOnMessage(const FString& Subject, const FString& Message)
{
	if (OnMessage.IsBound())
	{
		AsyncTask(ENamedThreads::GameThread, [this, Subject, Message] {

			if (OnMessage.IsBound())
			{
				OnMessage.Execute(Subject, Message, UnixTime);
			}
		});
	}
}

void UGBSNatsSubscription::InternalByteMessage(const FString& Subject, const TArray<uint8>& Message) const
{
    if (OnByteMessage.IsBound())
    {
        AsyncTask(ENamedThreads::GameThread, [this, Subject, Message]
		//AsyncTask(ENamedThreads::NormalThreadPriority, [this, Subject, Message]
        {
        	if (OnByteMessage.IsBound())
        	{
				static int messageCounterC = 0;
				messageCounterC++;
				UE_LOG(LogGBSNats, VeryVerbose, TEXT("UGBSNatsSubscription InternalByteMessage >>>>>>>>> TEST NATS MSG counter: %d, Subject: %s, Size: %d, threadId: %d"),
					messageCounterC, *Subject, Message.Num(), FPlatformTLS::GetCurrentThreadId());

        		OnByteMessage.Execute(Subject, Message, UnixTime);
        	}
        });
    }
}

void onMsgThread(natsConnection* nc, natsSubscription* sub, natsMsg* msg, void* closure)
{
	if (!msg) return;

	UGBSNatsSubscription* _this = static_cast<UGBSNatsSubscription*>(closure);
	if (!_this) return;

	_this->_messageCounterB++;
	UE_LOG(LogGBSNats, VeryVerbose, TEXT("UGBSNatsSubscription onMsgThread >>>>>>>>> TEST NATS MSG counter: %d, Subject: %s, Size: %d, threadId: %d"),
		_this->_messageCounterB, *FString(natsMsg_GetSubject(msg)), natsMsg_GetDataLength(msg), FPlatformTLS::GetCurrentThreadId());

	if (_this->_start == 0)
		_this->_start = nats_Now();

	if ((nats_Now() - _this->_last) >= 1000) // do every one Sec
	{
		natsStatus  s = printStats(STATS_IN | STATS_COUNT, nc, sub, _this->_stats);
		printPerf("Received", _this->_start, _this->_elapsed, _this->_count);
		_this->_last = nats_Now();
	}

	// TODO, we will need to create a proper callback here to avoid any blockers or delays returning, as it posible that it may create blocking!
	using namespace std::chrono;
	const int64 Unix = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
	_this->SetUnixTime(Unix);
	const FString subject = natsMsg_GetSubject(msg);

	if (_this->bByteMessage)
	{
		TArray<uint8> ByteMessage(reinterpret_cast<const TArray<unsigned char>::ElementType*>(natsMsg_GetData(msg)), natsMsg_GetDataLength(msg));
		_this->InternalByteMessage(subject, ByteMessage);
	}
	else
	{
		const std::string str(reinterpret_cast<const char*>(natsMsg_GetData(msg)), natsMsg_GetDataLength(msg));
		const FString message = (str.c_str());

		switch (_this->logLevel)
		{
		case MessageLogLevel::OFF: break;
		case MessageLogLevel::LIGHT:
			if ((_this->currentTime + _this->m_Interval) < _this->currentTime.Now())
			{
				//LogGBSNats changed to LogTemp
				UE_LOG(LogGBSNats, VeryVerbose, TEXT("%s : %s"), *subject, *message);
				_this->currentTime = _this->currentTime.Now();
			}
			break;
		case MessageLogLevel::VERBOSE: UE_LOG(LogGBSNats, VeryVerbose, TEXT("%s : %s"), *subject, *message); break;
		}

		_this->InternalOnMessage(subject, message);
	}

	if (++_this->_count == _this->_total)
		_this->_elapsed = nats_Now() - _this->_start;

	natsMsg_Destroy(msg);
}

#ifdef USE_NATS
void onMsg(natsConnection* nc, natsSubscription* sub, natsMsg* msg, void* closure)
{
	if (!msg) return;
	UGBSNatsSubscription* _this = static_cast<UGBSNatsSubscription*>(closure);
	if (!_this) {
		natsMsg_Destroy(msg);
		return;
	}

#ifdef USE_LATEST_NATS_IMPROVEMENTS

	_this->_messageCounter++;
	UE_LOG(LogGBSNats, VeryVerbose, TEXT("UGBSNatsSubscription onMsg >>>>>>>>> TEST NATS MSG counter: %d, Subject: %s, Size: %d, threadId: %d"),
		_this->_messageCounter, *FString(natsMsg_GetSubject(msg)), natsMsg_GetDataLength(msg), FPlatformTLS::GetCurrentThreadId());

	AsyncTask(ENamedThreads::NormalTaskPriority, [_this, nc, sub, msg]
	{
		onMsgThread(nc, sub, msg, _this);
	});
#else

	onMsgThread(nc, sub, msg, _this);

#endif

}
#endif

void UGBSNatsSubscription::onComplete(void* context) {

	UE_LOG(LogGBSNats, VeryVerbose, TEXT("UGBSNatsSubscription::onComplete CB"));

	//if (((UGBSNatsSubscription*)context)->natsSub != nullptr)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("UGBSNatsSubscription::onComplete natsSubscription_Destroy"));
	//	natsSubscription_Destroy(((UGBSNatsSubscription*)context)->natsSub);
	//}
}


bool UGBSNatsSubscription::DoSubscription(const FString& subject, FString& ResultValue)
{
#ifdef USE_NATS
	UE_LOG(LogGBSNats, VeryVerbose, TEXT("Subscribing to \"%s\""), *subject);

	natsStatus s = NATS_OK;
	s = natsConnection_Subscribe(&this->natsSub, this->natsConn, TCHAR_TO_ANSI(*subject), onMsg, (void*)this);
	if (s != NATS_OK)
	{
		const FString ErrorMessage = ANSI_TO_TCHAR(natsStatus_GetText(s)); // nats_GetLastError
		UE_LOG(LogGBSNats, Error, TEXT("Error subscribing to %s: %s"), *subject, *ErrorMessage);
		ResultValue = ErrorMessage;
		return false;
	}

#ifdef USE_LATEST_NATS_IMPROVEMENTS

	this->_start = 0;
	this->_last = 0;
	this->_total = 0;
	this->_messageCounter = 0;
	this->_messageCounterB = 0;
	//this->_messageCounterC = 0;
	this->_count = 0;
	this->_dropped = 0;
	this->_elapsed = 0;

	//if (s == NATS_OK)
	//	s = natsSubscription_NoDelay(this->natsSub);

	// For maximum performance, set no limit on the number of pending messages.
	if (s == NATS_OK)
		s = natsSubscription_SetPendingLimits(this->natsSub, -1, -1);
		//// Set limits of 100 messages or 1MB, whichever comes first
		//if (s == NATS_OK)
		//	s = natsSubscription_SetPendingLimits(this->natsSub, 100, 1 * 1024 * 1024);

	if (s == NATS_OK)
		s = natsSubscription_SetOnCompleteCB(this->natsSub, &onComplete, (void*)this);
		if (s != NATS_OK) {

			const FString ErrorMessage2 = ANSI_TO_TCHAR(natsStatus_GetText(s)); // nats_GetLastError
			UE_LOG(LogGBSNats, Error, TEXT("Error natsSubscription_SetOnCompleteCB to %s: %s"), *subject, *ErrorMessage2);
		}

	if (s == NATS_OK)
		s = natsStatistics_Create(&this->_stats);

#endif // !USE_LATEST_NATS_IMPROVEMENTS

	ResultValue = "NATS_OK";
	return true;
#endif
}

