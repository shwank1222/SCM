#include "Components/InteractionComponent.h"
#include "Components/InteractableComponent.h"
#include "UI/HUD/SCMHUD.h"
#include "UI/Widgets/InteractionWidget.h"
#include "UI/Widgets/DefaultWidget.h"
#include "DrawDebugHelpers.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	BindWidget();
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateVisibleInteractables();
}

void UInteractionComponent::BindWidget()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) { return; }

	ASCMHUD* HUD = Cast<ASCMHUD>(PC->GetHUD());
	if (!HUD) { return; }

	UDefaultWidget* DefaultWidget = HUD->GetDefaultWidget();
	if (!DefaultWidget) { return; }

	InteractionWidget = DefaultWidget->GetWidgetByClass<UInteractionWidget>();
}

#pragma region Interaction Management

void UInteractionComponent::AddInteractable( UInteractableComponent* Interactable )
{
	if (!Interactable){ return; }

	NearbyInteractables.AddUnique(Interactable);

	if (!IsComponentTickEnabled())
	{
		SetComponentTickEnabled(true);
	}
}

void UInteractionComponent::RemoveInteractable( UInteractableComponent* Interactable)
{
	if (!Interactable){	return; }

	NearbyInteractables.Remove(Interactable);

	Interactable->SetHighlighted(false);
	PreviousVisibleInteractables.Remove(Interactable);

	if (NearbyInteractables.IsEmpty())
	{
		VisibleInteractables.Empty();
		UpdateHighlightState();

		SetComponentTickEnabled(false);
	}
}

#pragma endregion

#pragma region Update Functions

void UInteractionComponent::UpdateVisibleInteractables()
{
	VisibleInteractables.Empty();
	NearbyInteractables.RemoveAll(
		[](const TWeakObjectPtr<UInteractableComponent>& Ptr)
		{
			return !Ptr.IsValid();
		});

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor){ return; }

	FVector ViewLocation;
	FRotator ViewRotation;

	OwnerActor->GetActorEyesViewPoint( ViewLocation, ViewRotation);

	const FVector ForwardVector = ViewRotation.Vector();

	for (const TWeakObjectPtr<UInteractableComponent>&InteractablePtr : NearbyInteractables)
	{
		UInteractableComponent* Interactable = InteractablePtr.Get();

		if (!IsValid(Interactable))
		{
			continue;
		}

		const FVector TargetLocation =
			Interactable->GetComponentLocation();

		const float Distance =
			FVector::Distance(
				ViewLocation,
				TargetLocation);

		if (Distance > MaxInteractionDistance) { continue; }

		const FVector DirectionToTarget = (TargetLocation - ViewLocation).GetSafeNormal();

		const float Dot = FVector::DotProduct(ForwardVector,DirectionToTarget);

		if (Dot < MinViewDot) { continue; }

		if (!IsInteractableVisible(Interactable)) { continue; }

		FInteractionCandidate Candidate;

		Candidate.Interactable = Interactable;
		Candidate.Dot = Dot;
		Candidate.Distance = Distance;

		VisibleInteractables.Add(Candidate);
	}

	VisibleInteractables.Sort(
		[](const FInteractionCandidate& A,
			const FInteractionCandidate& B)
		{
			return A.Dot > B.Dot;
		});

	if (CurrentSelectionIndex >= VisibleInteractables.Num())
	{
		CurrentSelectionIndex = 0;
	}

	UpdateHighlightState();
}

void UInteractionComponent::UpdateHighlightState()
{
	// Highlight OFF
	for (const TWeakObjectPtr<UInteractableComponent>&PreviousPtr: PreviousVisibleInteractables)
	{
		UInteractableComponent* Previous =	PreviousPtr.Get();
		if (!IsValid(Previous))
		{
			continue;
		}

		const bool bStillVisible =
			VisibleInteractables.ContainsByPredicate(
				[Previous](
					const FInteractionCandidate& Candidate)
				{
					return Candidate.Interactable.Get() == Previous;
				});

		if (!bStillVisible)
		{
			Previous->SetHighlighted(false);
		}
	}

	// Hightlight ON
	for (const FInteractionCandidate& Candidate: VisibleInteractables)
	{
		if (!Candidate.Interactable.IsValid())
		{
			continue;
		}

		UInteractableComponent* Interactable =	Candidate.Interactable.Get();

		if (!PreviousVisibleInteractables.Contains(Candidate.Interactable.Get()))
		{
			Interactable->SetHighlighted(true);
		}
	}

	// Update PreviousVisibleInteractables
	PreviousVisibleInteractables.Empty();

	for (const FInteractionCandidate& Candidate: VisibleInteractables)
	{
		if (!Candidate.Interactable.IsValid())
		{
			continue;
		}

		PreviousVisibleInteractables.Add(Candidate.Interactable);
	}
}

bool UInteractionComponent::IsInteractableVisible(UInteractableComponent* Interactable) const
{
	AActor* OwnerActor = GetOwner();

	if (!Interactable || !OwnerActor){	return false; }

	FVector ViewLocation;
	FRotator ViewRotation;

	OwnerActor->GetActorEyesViewPoint(
		ViewLocation,
		ViewRotation);

	const FVector TargetLocation =
		Interactable->GetComponentLocation();

	FHitResult HitResult;

	FCollisionQueryParams Params;

	Params.AddIgnoredActor(OwnerActor);

	const bool bHit =
		GetWorld()->LineTraceSingleByChannel(
			HitResult,
			ViewLocation,
			TargetLocation,
			ECC_Visibility,
			Params);

	if (!bHit)
	{
		return true;
	}

	return HitResult.GetActor() ==
		Interactable->GetOwner();
}

#pragma endregion