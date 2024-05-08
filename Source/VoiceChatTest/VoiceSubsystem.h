// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VivoxCoreCommon.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VoiceSubsystem.generated.h"

enum class LoginState : uint8;
class ILoginSession;
class IClient;

DECLARE_LOG_CATEGORY_EXTERN(LogVoiceSubsystem, Log, All)

/**
 * 
 */
UCLASS()
class VOICECHATTEST_API UVoiceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

private:
	void OnInitialPlayerCreated();

	void OnBeginLoginCompleted(VivoxCoreError Error);

	void OnLoginSessionStateChanged(LoginState State);

	void OnBeginConnectCompleted(VivoxCoreError Error);

	FString UniqueNetIdToVivoxUserName(FUniqueNetIdRepl UniqueNetId);

private:
	IClient* VoiceClient;
	ILoginSession* LoginSession;
};
