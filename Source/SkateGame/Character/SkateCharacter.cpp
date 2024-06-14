#include "Character/SkateCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"

ASkateCharacter::ASkateCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SkateMaxSpeed = 75'000.0f;
	SkateAcceleration = 1000.0f;
	SkateBreakingStrength = 30'000.0f;
	SkateFriction = 10'000.0f;
	SkateJumpingSpeed = 30'000.0f;
	SkateTurningSpeed = 10'000.0f;

	SkateMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skate Mesh"));
	SkateMesh->SetupAttachment(RootComponent);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	// FollowCamera->bUsePawnControlRotation = false;

	DefaultMappingContext = nullptr;
	MoveAction = nullptr;
	JumpAction = nullptr;

	SkateSpeed = 0.0f;
	bBreaking = false;
	bPushing = false;
	bJumping = false;
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

// Called when the game starts or when spawned
void ASkateCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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

void ASkateCharacter::Push()
{
	if (SkateSpeed < SkateMaxSpeed)
	{
		SkateSpeed += SkateAcceleration;
	}
}

void ASkateCharacter::StartJumping()
{
	if (SkateSpeed >= SkateJumpingSpeed)
	{
		bJumping = true;
	}
}

void ASkateCharacter::StopJumping()
{
	Super::StopJumping();
	bJumping = false;
}

// Called every frame
void ASkateCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetMovementComponent()->IsFalling() && !bWasJumping)
	{
		GetMovementComponent()->Velocity = GetActorForwardVector() * SkateSpeed * DeltaTime;
	}

	// Break
	const bool bMovingForward = FVector::DotProduct(GetVelocity(), GetActorForwardVector()) > 0.0f;
	if (bMovingForward && bBreaking)
	{
		SkateSpeed -= SkateBreakingStrength * DeltaTime;
	}

	if (SkateSpeed > 0.0f)
	{
		SkateSpeed -= SkateFriction * DeltaTime;
	}
}

// Called to bind functionality to input
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
	}
}
