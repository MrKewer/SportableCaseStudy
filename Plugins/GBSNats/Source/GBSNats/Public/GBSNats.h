#pragma once

#include "Modules/ModuleManager.h"
#include <UObject/ObjectMacros.h>

#define USE_LATEST_NATS_IMPROVEMENTS		// Chris

DECLARE_LOG_CATEGORY_EXTERN(LogGBSNats, Log, All);

class UGBSNatsSettings;

UENUM(BlueprintType)
enum ENatsStatus {
	ENATS_OK                            UMETA(DisplayName = "NatsOk"),
	ENATS_ERR                           UMETA(DisplayName = "Error"),
	ENATS_PROTOCOL_ERROR                UMETA(DisplayName = "Protocol Error"),
    ENATS_IO_ERROR                      UMETA(DisplayName = "IO Error"),
    ENATS_LINE_TOO_LONG                 UMETA(DisplayName = "Line too long"),
    ENATS_CONNECTION_CLOSED             UMETA(DisplayName = "Connection Closed"),
    ENATS_NO_SERVER                     UMETA(DisplayName = "No Server"),
    ENATS_STALE_CONNECTION              UMETA(DisplayName = "Stale Connection"),
    ENATS_SECURE_CONNECTION_WANTED      UMETA(DisplayName = "Secure Connection Wanted"),
    ENATS_SECURE_CONNECTION_REQUIRED    UMETA(DisplayName = "Secure Connection Required"),
    ENATS_CONNECTION_DISCONNECTED       UMETA(DisplayName = "Connection Disconnected"),
    ENATS_CONNECTION_AUTH_FAILED        UMETA(DisplayName = "Connection Auth Failed"),
    ENATS_NOT_PERMITTED                 UMETA(DisplayName = "Not Permitted"),
    ENATS_NOT_FOUND                     UMETA(DisplayName = "Not Found"),
    ENATS_ADDRESS_MISSING               UMETA(DisplayName = "Address Missing"),
    ENATS_INVALID_SUBJECT               UMETA(DisplayName = "Invalid Subject"),
    ENATS_INVALID_ARG                   UMETA(DisplayName = "Invalid Argument"),
    ENATS_INVALID_SUBSCRIPTION          UMETA(DisplayName = "Invalid Subscription"),
    ENATS_INVALID_TIMEOUT               UMETA(DisplayName = "Invalid Timeout"),
    ENATS_ILLEGAL_STATE                 UMETA(DisplayName = "Illegal State"),
    ENATS_SLOW_CONSUMER                 UMETA(DisplayName = "Slow Consumer"),
    ENATS_MAX_PAYLOAD                   UMETA(DisplayName = "Max Payload"),
    ENATS_MAX_DELIVERED_MSGS            UMETA(DisplayName = "Max Deleivered Messages"),
    ENATS_INSUFFICIENT_BUFFER           UMETA(DisplayName = "Insufficient Buffer"),
    ENATS_NO_MEMORY                     UMETA(DisplayName = "No Memory"),
    ENATS_SYS_ERROR                     UMETA(DisplayName = "System Error"),
    ENATS_TIMEOUT                       UMETA(DisplayName = "Timeout"),
    ENATS_FAILED_TO_INITIALIZE          UMETA(DisplayName = "Failed to Initialize"),
    ENATS_NOT_INITIALIZED               UMETA(DisplayName = "Not Initialized"),
    ENATS_SSL_ERROR                     UMETA(DisplayName = "SSL Error"),
    ENATS_NO_SERVER_SUPPORT             UMETA(DisplayName = "No Server Support"),
    ENATS_NOT_YET_CONNECTED             UMETA(DisplayName = "Not Yet Connected"),
    ENATS_DRAINING                      UMETA(DisplayName = "Draining"),
    ENATS_INVALID_QUEUE_NAME            UMETA(DisplayName = "Invalid Queue Name"),
    ENATS_NO_RESPONDERS                 UMETA(DisplayName = "No Responders"),
    ENATS_MISMATCH                      UMETA(DisplayName = "Mismatch"),
    ENATS_MISSED_HEARTBEAT              UMETA(DisplayName = "Missed Heartbeat"),
};

class FGBSNatsModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FGBSNatsModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FGBSNatsModule>("GBSNats");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("GBSNats");
	}

	/** Getter for internal settings object to support runtime configuration changes */
	// UGBSNatsSettings* GetSettings() const;

protected:
	/** Module settings */
	// UGBSNatsSettings* ModuleSettings;
};
