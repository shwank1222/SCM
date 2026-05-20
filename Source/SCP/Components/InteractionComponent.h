#pragma once

#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class UInteractableComponent;
class UInteractionWidget;

USTRUCT()
struct FInteractionCandidate
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UInteractableComponent> Interactable = nullptr;

	UPROPERTY()
	float Dot = 0.f;

	UPROPERTY()
	float Distance = 0.f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SCP_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	FORCEINLINE const TArray<FInteractionCandidate>& GetVisibleInteractables() const{ return VisibleInteractables; }

	FORCEINLINE UInteractableComponent* GetCurrentInteractable() const
	{
		if (VisibleInteractables.IsEmpty())
		{
			return nullptr;
		}

		return VisibleInteractables[CurrentSelectionIndex].Interactable;
	}

#pragma region Interaction Management

public:
	void AddInteractable(UInteractableComponent* Interactable);
	void RemoveInteractable(UInteractableComponent* Interactable);

protected:
	UPROPERTY()
	TArray<TObjectPtr<UInteractableComponent>> NearbyInteractables;

	UPROPERTY()
	TArray<FInteractionCandidate> VisibleInteractables;

	UPROPERTY()
	TArray<TObjectPtr<UInteractableComponent>> PreviousVisibleInteractables;

	UPROPERTY()
	int32 CurrentSelectionIndex = 0;

private:
	void UpdateVisibleInteractables();
	void UpdateHighlightState();

	bool IsInteractableVisible(UInteractableComponent* Interactable) const;

#pragma endregion

#pragma region Interaction Settings

protected:
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float MaxInteractionDistance = 300.f;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float MinViewDot = 0.7f;

#pragma endregion

#pragma region Widget Binding

protected:
	UPROPERTY()
	TObjectPtr<UInteractionWidget> InteractionWidget;

private:
	void BindWidget();

#pragma endregion

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
};