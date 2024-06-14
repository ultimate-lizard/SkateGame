// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SkateCharacter.generated.h"

class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class SKATEGAME_API ASkateCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASkateCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

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

	UFUNCTION(BlueprintPure, Category = "Skate")
	float GetSkateSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Skate")
	bool IsPushing() const;

	UFUNCTION(BlueprintPure, Category = "Skate")
	bool IsJumping() const;

	UFUNCTION(BlueprintCallable, Category = "Skate")
	void Push();

	void StartJumping();
	void StopJumping() override;

protected:
	virtual void BeginPlay() override;

	void OnEndMove(const FInputActionValue& InputActionValue);
	void Move(const FInputActionValue& InputActionValue);

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
	UInputAction* JumpAction;

	float SkateSpeed;
	bool bBreaking;
	bool bPushing;
	bool bJumping;
};
