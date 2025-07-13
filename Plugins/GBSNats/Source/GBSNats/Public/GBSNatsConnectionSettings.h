#pragma once

#include "nats/nats.h"

#include "GBSNatsConnectionSettings.generated.h"

// Our object definition
typedef struct __Errors
{
	int count;
} Errors;



UCLASS(BlueprintType, Blueprintable)
class GBSNATS_API UGBSNatsConnectionSettings : public UObject
{
  GENERATED_UCLASS_BODY()

public:

	virtual ~UGBSNatsConnectionSettings();

  /**
   * Add a server to the connection.
   * 
   * This should be fully qualified, such as "nats://user:password:x.x.x.x:xxxx"
   */
  UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|Connection")
  void AddServer(const FString& Server);

  /**
   * Sets the expected host name
   * 
   * Use this if the server name to which you're connecting is different from
   * that server's internal host name.
   */
  UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|Connection")
  void SetExpectedHostName(const FString& HostName);

	/**
	   * Sets the user credential file
	   * 
	   * Use this if the server you are connection to requiers authentication
	   */
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|Connection")
	bool SetCredentialsFile(const FString& FilePath);
	
  /**
   * Skip the TLS verification
   */
  UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|Connection")
  void SkipServerVerification(const bool Skip);

	/**
	 * Set The Secure Flag
	 */
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|Connection")
	void SetSecure(const bool Secure);

	/**
	* Set The Host Name
	*/
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|Connection")
	void SetHostName(const FString HostName);

	/**
	* Set The Connection Name
	*/
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "GBSNats|Connection")
	void SetConnectionName(const FString ConnectionName);
	
  /////////////////////////////////////////////////////
  // Internal C++ stuff, not used by blueprints
public:
  FString GetConnectionString();
  natsOptions* GetOptions();

private:

  const char** ConvertServers();


  static void connectedCB(natsConnection*, void*);

  static void errorCB(natsConnection* conn, natsSubscription* sub, natsStatus s, void* closure);

  static void disconnectedCB(natsConnection* nc, void* closure);

  static void reconnectedCB(natsConnection* nc, void* closure);

  static void closedCB(natsConnection* nc, void* closure);
	  

private:
  TArray<FString> Servers;
  natsOptions* opts;


  // Declare an 'Errors' object on the stack.
  Errors			_asyncErrors;
  volatile int64_t	_dropped = 0;

};
