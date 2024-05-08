// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"

void UMyGameInstance::Init()
{
	Super::Init();

	if (IOnlineSubsystem::Get()->GetSubsystemName() == "Steam")
	{
		//FOnlineSubsystemSteam* OssSteam = StaticCast<FOnlineSubsystemSteam*>(IOnlineSubsystem::Get());
		//uint32 AuthTokenHandle;
		//FString Token = OssSteam->GetAuthInterface()->GetAuthTicket(AuthTokenHandle);
	}

	FString MyString = FString::Printf(TEXT("FNetworkVersion::GetLocalNetworkVersion() = %u"), FNetworkVersion::GetLocalNetworkVersion());
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, MyString);

	auto OnlineSessionPtr = GetOnlineSession();
	OnlineSessionPtr->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMyGameInstance::OnCreateSessionComplete));
	OnlineSessionPtr->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionCompleteDelegate::CreateUObject(this, &UMyGameInstance::OnStartSessionComplete));
	OnlineSessionPtr->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMyGameInstance::OnFindSessionsComplete));
	OnlineSessionPtr->AddOnSessionParticipantsChangeDelegate_Handle(FOnSessionParticipantsChangeDelegate::CreateUObject(this, &UMyGameInstance::OnSessionParticipantsChange));
	OnlineSessionPtr->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMyGameInstance::OnJoinSessionComplete));
}

TSharedPtr<FOnlineSessionSearch> ptr;

IOnlineSessionPtr UMyGameInstance::GetOnlineSession()
{
	if (!OnlineSessionCachedPtr.IsValid())
	{
		IOnlineSubsystem* oss = IOnlineSubsystem::Get();
		{
			FString MyString = FString::Printf(TEXT("oss Valid = %d"), oss != nullptr);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, MyString);
		}

		if (oss == nullptr)
		{
			return {};
		}

		{
			FString MyString = FString::Printf(TEXT("oss Name = %s"), *oss->GetSubsystemName().ToString());
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, MyString);
		}

		OnlineSessionCachedPtr = oss->GetSessionInterface();
		{
			FString MyString = FString::Printf(TEXT("OnlineSession Valid = %d"), OnlineSessionCachedPtr.IsValid());
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, MyString);
		}
	}

	return OnlineSessionCachedPtr;
}

void UMyGameInstance::Create()
{
	auto OnlineSessionPtr = GetOnlineSession();

	EnableListenServer(true);

	FOnlineSessionSettings settings;
	settings.NumPublicConnections = 10;
	settings.bUsesPresence = true;
	settings.bUseLobbiesIfAvailable = true;
	settings.bShouldAdvertise = true;
	settings.bAllowJoinInProgress = true; // can be off
	settings.bAllowInvites = true; // can be off
	settings.bAllowJoinViaPresence = true; // can be off
	settings.Set(FName(TEXT("GAME_NAME")), FString(FApp::GetProjectName()), EOnlineDataAdvertisementType::ViaOnlineService);
	OnlineSessionPtr->CreateSession(0, NAME_GameSession, settings);
}

void UMyGameInstance::OnCreateSessionComplete(FName name, bool bWasSuccessful)
{
	UE_LOG(LogOnline, Display, TEXT("OnCreateSessionComplete success = %d name = %s"), bWasSuccessful, *name.ToString());
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("OnCreateSessionComplete success = %d name = %s"), bWasSuccessful, *name.ToString()));

	auto OnlineSessionPtr = GetOnlineSession();
	OnlineSessionPtr->StartSession(NAME_GameSession);
}

void UMyGameInstance::OnStartSessionComplete(FName name, bool bWasSuccessful)
{
	UE_LOG(LogOnline, Display, TEXT("OnStartSessionComplete success = %d name = %s"), bWasSuccessful, *name.ToString());
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("OnStartSessionComplete success = %d name = %s"), bWasSuccessful, *name.ToString()));
}

bool searching = false;
void TimerTick()
{
	if (!searching)
	{
		return;
	}

	GWorld->GetTimerManager().SetTimerForNextTick(TimerTick);
	UKismetSystemLibrary::PrintString(GWorld, FString::FromInt(ptr->SearchResults.Num()));
}

void UMyGameInstance::Find()
{
	auto OnlineSessionPtr = GetOnlineSession();

	ptr = MakeShared<FOnlineSessionSearch>();
	FOnlineSearchSettings settings;
	settings.Set(FName(TEXT("GAME_NAME")), FString(FApp::GetProjectName()), EOnlineComparisonOp::Equals);
	settings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	ptr->QuerySettings = settings;
	ptr->MaxSearchResults = 10;

	searching = true;
	TimerManager->SetTimerForNextTick(TimerTick);
	OnlineSessionPtr->FindSessions(0, ptr.ToSharedRef());
}

void UMyGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogOnline, Display, TEXT("OnFindSessionsComplete success = %d cnt = %d"), bWasSuccessful, ptr->SearchResults.Num());
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("OnFindSessionsComplete success = %d cnt = %d"), bWasSuccessful, ptr->SearchResults.Num()));
	searching = false;
}

void UMyGameInstance::Join()
{
	auto OnlineSessionPtr = GetOnlineSession();

	check(!ptr->SearchResults.IsEmpty());

	OnlineSessionPtr->JoinSession(0, "session1", ptr->SearchResults[0]);
}

void UMyGameInstance::OnJoinSessionComplete(FName name, EOnJoinSessionCompleteResult::Type type)
{
	auto OnlineSessionPtr = GetOnlineSession();
	FString ConnectInfo;
	OnlineSessionPtr->GetResolvedConnectString("session1", ConnectInfo);

	UE_LOG(LogOnline, Display, TEXT("OnJoinSessionComplete name = %s type = %s connect = %s"), *name.ToString(), LexToString(type), *ConnectInfo);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("OnJoinSessionComplete name = %s type = %s connect = %s"), *name.ToString(), LexToString(type), *ConnectInfo));

	GetFirstLocalPlayerController()->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
}

void UMyGameInstance::OnSessionParticipantsChange(FName name, const FUniqueNetId& netid, bool joined)
{
	UE_LOG(LogOnline, Display, TEXT("OnSessionParticipantsChange name = %s netid = %s joined = %d"), *name.ToString(), *netid.ToString(), joined);
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, FString::Printf(TEXT("OnSessionParticipantsChange name = %s netid = %s joined = %d"), *name.ToString(), *netid.ToString(), joined));
}

ULocalPlayer* UMyGameInstance::CreateInitialPlayer(FString& OutError)
{
	ULocalPlayer* Result = Super::CreateInitialPlayer(OutError);
	OnInitialPlayerCreated.Broadcast();
	return Result;
}
