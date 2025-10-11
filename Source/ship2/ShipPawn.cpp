#include "ShipPawn.h"

AShipPawn::AShipPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
    ShipMesh->SetupAttachment(RootComponent);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    Camera->SetRelativeLocation(FVector(-300.f, 0.f, 200.f));
    Camera->SetRelativeRotation(FRotator(-20.f, 0.f, 0.f));

    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AShipPawn::BeginPlay()
{
    Super::BeginPlay();
}

void AShipPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!CurrentVelocity.IsNearlyZero())
    {
        FVector NewLocation = GetActorLocation() + (CurrentVelocity * DeltaTime);
        SetActorLocation(NewLocation);
    }
}

void AShipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AShipPawn::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AShipPawn::MoveRight);
}

void AShipPawn::MoveForward(float Value)
{
    CurrentVelocity.X = FMath::Clamp(Value, -1.f, 1.f) * 600.f;
}

void AShipPawn::MoveRight(float Value)
{
    CurrentVelocity.Y = FMath::Clamp(Value, -1.f, 1.f) * 600.f;
}