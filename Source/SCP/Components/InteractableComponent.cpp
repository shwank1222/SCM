#include "Components/InteractableComponent.h"
#include "Components/InteractionComponent.h"
#include "Components/SphereComponent.h"

UInteractableComponent::UInteractableComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetupAttachment(this);
    InteractionSphere->SetSphereRadius(150.f);
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

    InitializeInteractionSphere();
	InitializeOverlayMaterial();

#if WITH_EDITOR

    if (GetWorld() && GetWorld()->IsPlayInEditor())
    {
        InteractionSphere->SetHiddenInGame(false);
    }

#endif
}

void UInteractableComponent::InitializeInteractionSphere()
{
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    InteractionSphere->SetGenerateOverlapEvents(true);

    InteractionSphere->OnComponentBeginOverlap.AddDynamic( this, &UInteractableComponent::OnBeginOverlap );
    InteractionSphere->OnComponentEndOverlap.AddDynamic( this, &UInteractableComponent::OnEndOverlap );
}

void UInteractableComponent::InitializeOverlayMaterial()
{
    if (!OverlayMaterial){ return; }

    UMeshComponent* Mesh = GetOwner()->FindComponentByClass<UMeshComponent>();

    if (!Mesh){ return; }

    OverlayMID = UMaterialInstanceDynamic::Create( OverlayMaterial, this);
    if (!OverlayMID) { return; }
    
    Mesh->SetOverlayMaterial(OverlayMID);

    static const FName LineThicknessParam =
        TEXT("LineThickness");

    OverlayMID->SetScalarParameterValue( LineThicknessParam, 0.f);
}

void UInteractableComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UInteractionComponent* InteractionComponent =
        OtherActor->FindComponentByClass<UInteractionComponent>();

    if (!InteractionComponent) { return; }

    InteractionComponent->AddInteractable(this);

    UE_LOG(LogTemp, Warning, TEXT("Overlap with %s"), *InteractionName.ToString());
}

void UInteractableComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    UInteractionComponent* InteractionComponent =
        OtherActor->FindComponentByClass<UInteractionComponent>();

    if (!InteractionComponent){ return; }
  
    InteractionComponent->RemoveInteractable(this);

    UE_LOG(LogTemp, Warning, TEXT("End overlap with %s"), *InteractionName.ToString());
}

void UInteractableComponent::HandleInteraction(AActor* InteractingActor)
{

}

void UInteractableComponent::SetHighlighted(bool bHighlighted)
{
    if (!OverlayMID) { return;}

    OverlayMID->SetScalarParameterValue( TEXT("LineThickness"), bHighlighted ? 3.f : 0.f);
}

void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}