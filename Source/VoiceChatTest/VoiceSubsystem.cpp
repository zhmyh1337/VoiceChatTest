// Fill out your copyright notice in the Description page of Project Settings.


#include "VoiceSubsystem.h"

#include "VivoxCore.h"

#define VIVOX_SERVER "https://unity.vivox.com/appconfig/79715-test-99539-udash"
#define VIVOX_DOMAIN "mtu1xp.vivox.com"
#define VIVOX_ISSUER "79715-test-99539-udash"
#define VIVOX_KEY "WHPOnU9QlP7f3I18cB7smnUKPfmRJIcy"

DEFINE_LOG_CATEGORY(LogVoiceSubsystem);

void UVoiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FVivoxCoreModule* MyVoiceModule = static_cast<FVivoxCoreModule*>(&FModuleManager::Get().LoadModuleChecked(TEXT("VivoxCore")));

	VoiceClient = &MyVoiceModule->VoiceClient();

	if (VivoxCoreError Error = VoiceClient->Initialize())
	{
		UE_LOG(LogVoiceSubsystem, Error, TEXT("VoiceClient->Initialize() error %d"), Error);
	}

	AccountId Account(VIVOX_ISSUER, "example_user", VIVOX_DOMAIN);
	LoginSession = &VoiceClient->GetLoginSession(Account);

	ILoginSession::FOnBeginLoginCompletedDelegate OnBeginLoginCompletedDelegate;
	OnBeginLoginCompletedDelegate.BindUObject(this, &ThisClass::OnBeginLoginCompleted);

	LoginSession->EventStateChanged.AddUObject(this, &ThisClass::OnLoginSessionStateChanged);

	FString LoginToken = LoginSession->GetLoginToken(VIVOX_KEY, FTimespan::FromSeconds(60));
	if (VivoxCoreError Error = LoginSession->BeginLogin(VIVOX_SERVER, LoginToken, OnBeginLoginCompletedDelegate))
	{
		UE_LOG(LogVoiceSubsystem, Error, TEXT("LoginSession->BeginLogin error %d"), Error);
	}

	UE_LOG(LogVoiceSubsystem, Log, TEXT("LoginSession->BeginLogin called"));
}

void UVoiceSubsystem::Deinitialize()
{
	Super::Deinitialize();

	LoginSession->Logout();
	VoiceClient->Uninitialize();
}

void UVoiceSubsystem::OnBeginLoginCompleted(VivoxCoreError Error)
{
	if (Error)
	{
		UE_LOG(LogVoiceSubsystem, Error, TEXT("OnBeginLoginCompleted error %d"), Error);
		return;
	}

	UE_LOG(LogVoiceSubsystem, Log, TEXT("OnBeginLoginCompleted success"));

	ChannelId Channel(VIVOX_ISSUER, "example_channel", VIVOX_DOMAIN, ChannelType::Echo);
	IChannelSession* ChannelSession = &LoginSession->GetChannelSession(Channel);

	IChannelSession::FOnBeginConnectCompletedDelegate OnBeginConnectCompletedDelegate;
	OnBeginConnectCompletedDelegate.BindUObject(this, &ThisClass::OnBeginConnectCompleted);

	FString ConnectToken = ChannelSession->GetConnectToken(VIVOX_KEY, FTimespan::FromSeconds(60));
	if (VivoxCoreError BeginConnectError = ChannelSession->BeginConnect(true, false, true, ConnectToken, OnBeginConnectCompletedDelegate))
	{
		UE_LOG(LogVoiceSubsystem, Error, TEXT("ChannelSession->BeginConnect error %d"), BeginConnectError);
	}

	UE_LOG(LogVoiceSubsystem, Log, TEXT("ChannelSession->BeginConnect called"));
}

void UVoiceSubsystem::OnLoginSessionStateChanged(LoginState State)
{
	UE_LOG(LogVoiceSubsystem, Log, TEXT("OnLoginSessionStateChanged (%s)"), *UEnum::GetDisplayValueAsText(State).ToString());

	// TODO: handle OnLoginSessionStateChanged LoggedOut (happens after 30 seconds of lacking internet connectivity)
}

void UVoiceSubsystem::OnBeginConnectCompleted(VivoxCoreError Error)
{
	if (Error)
	{
		UE_LOG(LogVoiceSubsystem, Error, TEXT("OnBeginConnectCompleted error %d"), Error);
		return;
	}

	UE_LOG(LogVoiceSubsystem, Log, TEXT("OnBeginConnectCompleted success"));
}
