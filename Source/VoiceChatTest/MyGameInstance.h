// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MyGameInstance.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInitialPlayerCreated)

UCLASS()
class VOICECHATTEST_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	virtual void Init() override;

protected:
	IOnlineSessionPtr GetOnlineSession();

public:
	FOnInitialPlayerCreated OnInitialPlayerCreated;

	IOnlineSessionPtr OnlineSessionCachedPtr;

	UFUNCTION(Exec)
	void Create();

	UFUNCTION(Exec)
	void Find();

	UFUNCTION(Exec)
	void Join();

	void OnFindSessionsComplete(bool bWasSuccessful);

	void OnCreateSessionComplete(FName name, bool bWasSuccessful);

	void OnStartSessionComplete(FName name, bool bWasSuccessful);

	void OnJoinSessionComplete(FName name, EOnJoinSessionCompleteResult::Type type);

	void OnSessionParticipantsChange(FName name, const FUniqueNetId& netid, bool joined);

	virtual ULocalPlayer* CreateInitialPlayer(FString& OutError) override;

	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FOnSessionParticipantsChangeDelegate OnSessionParticipantsChangeDelegate;
};
