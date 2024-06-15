// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "GoalBox.generated.h"

/**
 * 
 */
UCLASS()
class SKATEGAME_API AGoalBox : public ATriggerBox
{
	GENERATED_BODY()
	
public:
	AGoalBox();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Score;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AActor* ObstacleActor;

protected:
	UFUNCTION()
	void OnGoalBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
