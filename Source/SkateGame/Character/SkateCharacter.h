// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SkateCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AGoalBox;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScoreSignature, int, ScoreAdded, bool, HalfScore);

UCLASS()
class SKATEGAME_API ASkateCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate")
	float SkateMaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate")
	float SkateAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate")
	float SkateBreakingStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate")
	float SkateFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate")
	float SkateJumpingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate")
	float SkateTurningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate")
	float SkateCollisionCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate")
	float SkateCrashSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* SkateMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* LosingBalanceMontage;

	UFUNCTION(BlueprintPure, Category = "Skate")
	float GetSkateSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Skate")
	bool IsPushing() const;

	UFUNCTION(BlueprintPure, Category = "Skate")
	bool IsJumping() const;

	UFUNCTION(BlueprintPure, Category = "Skate")
	bool IsCrashing() const;

	UFUNCTION(BlueprintPure, Category = "Skate")
	bool IsLosingBalance() const;

	UFUNCTION(BlueprintCallable, Category = "Skate")
	void Push();

	UFUNCTION(BlueprintPure, Category = "Score")
	int GetScore() const;

	UPROPERTY(BlueprintAssignable, Category = "Score")
	FScoreSignature OnScore;

	ASkateCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void StartJumping();
	void StopJumping() override;

	void SetCurrentGoal(AGoalBox* GoalBox);

protected:
	UPROPERTY()
	int Score;

	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void OnSkateCollision(FVector Normal);

	void OnCrashStart();

	UFUNCTION()
	void OnCrashEnd();

	void OnEndMove(const FInputActionValue& InputActionValue);
	void Move(const FInputActionValue& InputActionValue);
	void Look(const FInputActionValue& InputActionValue);

	float SkateSpeed;
	bool bBreaking;
	bool bPushing;
	bool bJumping;

	AGoalBox* CurrentGoal;

	FTimerHandle CollisionTimer;
	FTimerHandle CrashTimer;
};
