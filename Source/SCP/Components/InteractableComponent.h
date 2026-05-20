// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
#include "InteractableComponent.generated.h"

class USphereComponent;

// This Component is attached to any Actor that can be interacted with by the player.
// It will handle all interaction logic for the Actor, such as displaying prompts and executing interaction events.

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SCP_API UInteractableComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractableComponent();

	FORCEINLINE const FText& GetInteractionText() const{ return InteractionName; }

	void SetHighlighted(bool bHighlighted);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractionName;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TObjectPtr<UMaterialInterface> OverlayMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> OverlayMID;

#pragma region Overlap Functions

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

#pragma endregion

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void InitializeInteractionSphere();
	void InitializeOverlayMaterial();
	void HandleInteraction(AActor* InteractingActor);
		
};
