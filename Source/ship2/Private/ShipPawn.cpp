#include "ShipPawn.h"
#include "Components/InputComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

AShipPawn::AShipPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Mesh (root) ---
    ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
    RootComponent = ShipMesh;
    ShipMesh->SetCollisionProfileName(TEXT("Pawn"));
    ShipMesh->SetSimulatePhysics(false);
    ShipMesh->SetMobility(EComponentMobility::Movable);

    // ✅ Kolizija direktno na root mesh-u (parent collider)
    ShipMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ShipMesh->SetNotifyRigidBodyCollision(true);
}

void AShipPawn::BeginPlay()
{
    Super::BeginPlay();
}

void AShipPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector Move = LocalVelocity * DeltaTime;
    AddActorLocalOffset(Move, true);
}

void AShipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AShipPawn::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AShipPawn::MoveRight);
    PlayerInputComponent->BindAxis("MoveUp", this, &AShipPawn::MoveUp);
    PlayerInputComponent->BindAxis("Pitch", this, &AShipPawn::Pitch);
    PlayerInputComponent->BindAxis("Yaw", this, &AShipPawn::Yaw);
    PlayerInputComponent->BindAxis("Roll", this, &AShipPawn::Roll);

    PlayerInputComponent->BindAction("Boost", IE_Pressed, this, &AShipPawn::BoostPressed);
    PlayerInputComponent->BindAction("Boost", IE_Released, this, &AShipPawn::BoostReleased);
}

void AShipPawn::MoveForward(float Value)
{
    LocalVelocity.X = Value * MoveSpeed * (bIsBoosting ? BoostMultiplier : 1.f);
}

void AShipPawn::MoveRight(float Value)
{
    LocalVelocity.Y = Value * MoveSpeed * (bIsBoosting ? BoostMultiplier : 1.f);
}

void AShipPawn::MoveUp(float Value)
{
    LocalVelocity.Z = Value * MoveSpeed * (bIsBoosting ? BoostMultiplier : 1.f);
}

void AShipPawn::Pitch(float Value)
{
    AddActorLocalRotation(FRotator(Value * MouseSensitivityDegPerSec * GetWorld()->GetDeltaSeconds(), 0.f, 0.f));
}

void AShipPawn::Yaw(float Value)
{
    AddActorLocalRotation(FRotator(0.f, Value * MouseSensitivityDegPerSec * GetWorld()->GetDeltaSeconds(), 0.f));
}

void AShipPawn::Roll(float Value)
{
    AddActorLocalRotation(FRotator(0.f, 0.f, Value * RollSpeedDegPerSec * GetWorld()->GetDeltaSeconds()));
}

void AShipPawn::BoostPressed()
{
    bIsBoosting = true;
}

void AShipPawn::BoostReleased()
{
    bIsBoosting = false;
}

void AShipPawn::DebugCollision()
{
    UE_LOG(LogTemp, Warning, TEXT("DebugCollision triggered"));
    DebugSweepForward();
}

void AShipPawn::DebugSweepForward() const
{
    const FVector Start = GetActorLocation();
    const FVector End = Start + GetActorForwardVector() * 200.f;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(DebugSweep), false, this);
    FCollisionResponseParams RespParams;
    FHitResult Hit;

    const float Radius = 50.f;
    const bool bHit = GetWorld()->SweepSingleByChannel(
        Hit, Start, End, FQuat::Identity, ECC_WorldStatic,
        FCollisionShape::MakeSphere(Radius), Params, RespParams
    );

    if (bHit)
    {
        DrawDebugSphere(GetWorld(), Hit.ImpactPoint, Radius, 16, FColor::Red, false, 1.f);
    }
    else
    {
        DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.f, 0, 1.f);
    }
}