#include "Character/SkateCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "Actor/GoalBox.h"
#include "Components/CapsuleComponent.h"

ASkateCharacter::ASkateCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SkateMaxSpeed = 75'000.0f;
	SkateAcceleration = 1000.0f;
	SkateBreakingStrength = 30'000.0f;
	SkateFriction = 10'000.0f;
	SkateJumpingSpeed = 30'000.0f;
	SkateTurningSpeed = 10'000.0f;

	DefaultMappingContext = nullptr;
	MoveAction = nullptr;
	JumpAction = nullptr;
	LookAction = nullptr;

	LosingBalanceMontage = nullptr;

	SkateMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skate Mesh"));
	SkateMesh->SetupAttachment(RootComponent);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	SkateSpeed = 0.0f;
	bBreaking = false;
	bPushing = false;
	bJumping = false;

	CurrentGoal = nullptr;
	Score = 0;

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->OnComponentHit.AddDynamic(this, &ASkateCharacter::OnCapsuleHit);
	}
}

float ASkateCharacter::GetSkateSpeed() const
{
	return SkateSpeed;
}

bool ASkateCharacter::IsPushing() const
{
	return bPushing;
}

bool ASkateCharacter::IsJumping() const
{
	return bJumping;
}

bool ASkateCharacter::IsCrashing() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetTimerManager().IsTimerActive(CrashTimer);
	}

	return false;
}

bool ASkateCharacter::IsLosingBalance() const
{
	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		if (UAnimInstance* CharacterAnimInstance = CharacterMesh->GetAnimInstance())
		{
			return CharacterAnimInstance->Montage_IsPlaying(LosingBalanceMontage);
		}
	}

	return false;
}

void ASkateCharacter::OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	int ScoredPoints = 0;
	bool bHalfScore = false;
	bool bHasScored = false;

	const float HitDotProduct = FVector::DotProduct(GetActorUpVector(), Hit.Normal);
	const float HitDotProductThreshold = 0.5f;
	// If collision was vertical
	if (HitDotProduct >= HitDotProductThreshold && CurrentGoal)
	{
		if (CurrentGoal)
		{
			bHasScored = true;

			const bool bLandedOnObstacle = CurrentGoal->ObstacleActor == OtherActor;
			if (bLandedOnObstacle)
			{
				ScoredPoints = CurrentGoal->Score / 2;
				bHalfScore = true;
			}
			else
			{
				ScoredPoints = CurrentGoal->Score;
			}
		}
	}
	else
	{
		// Register hit if collision was horizontal
		OnSkateCollision(Hit.Normal);
	}

	Score += ScoredPoints;

	if (bHasScored && CurrentGoal)
	{
		OnScore.Broadcast(ScoredPoints, bHalfScore);
		CurrentGoal = nullptr;
	}
}

void ASkateCharacter::OnSkateCollision(FVector Normal)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();

	if (TimerManager.IsTimerActive(CollisionTimer))
	{
		return;
	}

	const float CollisionCooldown = 0.5f;
	TimerManager.SetTimer(CollisionTimer, CollisionCooldown, false);

	const float FatalSpeed = 60'000.0f;
	if (SkateSpeed >= FatalSpeed && GetCharacterMovement()->IsFalling())
	{
		OnCrashStart();
	}
	else
	{
		const float CollisionDotProduct = FMath::Abs(FVector::DotProduct(GetActorForwardVector(), Normal));
		const float CollisionDotProductThreshold = 0.8f;
		if (CollisionDotProduct > CollisionDotProductThreshold)
		{
			if (SkateSpeed >= FatalSpeed)
			{
				// Play hit montage
				if (USkeletalMeshComponent* CharacterMesh = GetMesh())
				{
					if (UAnimInstance* CharacterAnimInstance = CharacterMesh->GetAnimInstance())
					{
						CharacterAnimInstance->Montage_Play(LosingBalanceMontage);
					}
				}
			}

			SkateSpeed = -SkateSpeed / 2.0f;
		}
		else
		{
			SkateSpeed *= 0.5f;
		}
	}
}

void ASkateCharacter::OnCrashStart()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float FallLength = 3.0f;
	World->GetTimerManager().SetTimer(CrashTimer, this, &ASkateCharacter::OnCrashEnd, FallLength, false);

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		CharacterMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		CharacterMesh->SetAllBodiesSimulatePhysics(true);
		CharacterMesh->WakeAllRigidBodies();
		CharacterMesh->SetSimulatePhysics(true);
	}
}

void ASkateCharacter::OnCrashEnd()
{
	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		const FVector DefaultMeshPosition(0.0f, 0.0f, -78.5f);
		const FRotator DefaultMeshRotator(0.0f, -90.0f, 0.0f);

		CharacterMesh->SetSimulatePhysics(false);
		CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CharacterMesh->SetAllBodiesSimulatePhysics(false);
		CharacterMesh->PutRigidBodyToSleep();
		CharacterMesh->SetRelativeLocation(DefaultMeshPosition);
		CharacterMesh->SetRelativeRotation(DefaultMeshRotator);
	}
}

void ASkateCharacter::OnEndMove(const FInputActionValue& InputActionValue)
{
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	if (MovementVector.X == 0.0f)
	{
		bPushing = false;
	}
}

void ASkateCharacter::Move(const FInputActionValue& InputActionValue)
{
	if (IsCrashing())
	{
		return;
	}

	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	if (MovementVector.X > 0.0f && SkateMesh)
	{
		if (!GetMovementComponent()->IsFalling())
		{
			bPushing = true;
		}

		bBreaking = false;
	}
	else if (MovementVector.X < 0.0f)
	{
		bBreaking = true;
	}

	if (MovementVector.X <= 0.0f)
	{
		bPushing = false;
	}

	if (MovementVector.Y && SkateSpeed >= SkateTurningSpeed)
	{
		FRotator Rotation(0.0f);
		Rotation.Yaw = MovementVector.Y;
		AddActorWorldRotation(Rotation);
	}
}

void ASkateCharacter::Look(const FInputActionValue& InputActionValue)
{
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	if (APlayerController* PlayerController = GetController<APlayerController>())
	{
		PlayerController->AddPitchInput(MovementVector.Y);
		PlayerController->AddYawInput(MovementVector.X);
	}
}

void ASkateCharacter::Push()
{
	if (IsLosingBalance())
	{
		return;
	}

	if (SkateSpeed < SkateMaxSpeed)
	{
		if (SkateSpeed < 0.0f)
		{
			SkateSpeed = 0.0f;
		}

		SkateSpeed += SkateAcceleration;
	}
}

int ASkateCharacter::GetScore() const
{
	return Score;
}

void ASkateCharacter::StartJumping()
{
	if (SkateSpeed >= SkateJumpingSpeed && !IsLosingBalance())
	{
		bJumping = true;
	}
}

void ASkateCharacter::StopJumping()
{
	ACharacter::StopJumping();
	bJumping = false;
}

void ASkateCharacter::SetCurrentGoal(AGoalBox* GoalBox)
{
	CurrentGoal = GoalBox;
}

void ASkateCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Movement forward
	if (UPawnMovementComponent* PawnMovementComponent = GetMovementComponent())
	{
		if (!PawnMovementComponent->IsFalling() && !bWasJumping)
		{
			PawnMovementComponent->Velocity = GetActorForwardVector() * SkateSpeed * DeltaTime;
		}
	}

	// Break
	const bool bMovingForward = FVector::DotProduct(GetVelocity(), GetActorForwardVector()) > 0.0f;
	if (bMovingForward && bBreaking)
	{
		SkateSpeed -= SkateBreakingStrength * DeltaTime;
	}

	// Friction
	if (SkateSpeed > 0.0f)
	{
		SkateSpeed -= SkateFriction * DeltaTime;
	}
	else if (SkateSpeed < 0.0f)
	{
		SkateSpeed += SkateFriction * DeltaTime;
	}
}

void ASkateCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = GetController<APlayerController>())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASkateCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASkateCharacter::OnEndMove);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASkateCharacter::StartJumping);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASkateCharacter::StopJumping);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASkateCharacter::Look);
	}
}

void ASkateCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->OnComponentHit.RemoveDynamic(this, &ASkateCharacter::OnCapsuleHit);
	}
}
