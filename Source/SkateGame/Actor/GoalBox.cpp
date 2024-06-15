#include "Actor/GoalBox.h"
#include "Components/ShapeComponent.h"
#include "Character/SkateCharacter.h"

AGoalBox::AGoalBox()
{
	Score = 1000;
	ObstacleActor = nullptr;

	if (UShapeComponent* ShapeCollisionComponent = GetCollisionComponent())
	{
		ShapeCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AGoalBox::OnGoalBeginOverlap);
	}
}

void AGoalBox::OnGoalBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ASkateCharacter* SkateCharacter = Cast<ASkateCharacter>(OtherActor))
	{
		SkateCharacter->SetCurrentGoal(this);
	}
}
