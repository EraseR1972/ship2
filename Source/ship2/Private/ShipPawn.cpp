#include "ShipPawn.h"
#include "Components/InputComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

AShipPawn::AShipPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Mesh kao root (kreirati SAMO jednom) ---
    ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
    RootComponent = ShipMesh;

    // Kinematički: bez simulacije fizike
    ShipMesh->SetSimulatePhysics(false);
    ShipMesh->SetMobility(EComponentMobility::Movable);

    // Kolizija: Query & Physics (da AddActorLocalOffset(..., true) radi sweep i poštuje kolizije)
    ShipMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ShipMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    // Alternativa ako želiš eksplicitno:
    // ShipMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // --- Kamera (FPS stil) ---
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    Camera->SetRelativeLocation(FVector(-30.f, 0.f, 10.f));
    Camera->bUsePawnControlRotation = false;

    // Ne koristimo Controller rotacije (rotiramo Actor direktno)
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AShipPawn::BeginPlay()
{
    Super::BeginPlay();
    DebugCollision(); // jednom na startu ispiše stanje kolizije i nacrta test sweep
}

void AShipPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    const float EffectiveSpeed = MoveSpeed * (bIsBoosting ? BoostMultiplier : 1.f);

    // Translacija u lokalnim osama (X fwd, Y right, Z up) sa sweep-om
    const FVector LocalMove = LocalVelocity * EffectiveSpeed * DeltaTime;
    if (!LocalMove.IsNearlyZero())
    {
        AddActorLocalOffset(LocalMove, /*bSweep*/ true);
    }
}

void AShipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    check(PlayerInputComponent);

    // Translacija
    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AShipPawn::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"),   this, &AShipPawn::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("MoveUp"),      this, &AShipPawn::MoveUp);

    // Rotacija (miš + Q/E)
    PlayerInputComponent->BindAxis(TEXT("Turn"),   this, &AShipPawn::Yaw);    // Mouse X
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AShipPawn::Pitch);  // Mouse Y
    PlayerInputComponent->BindAxis(TEXT("Roll"),   this, &AShipPawn::Roll);   // Q/E

    // Boost (Left Shift)
    PlayerInputComponent->BindAction(TEXT("Boost"), IE_Pressed,  this, &AShipPawn::BoostPressed);
    PlayerInputComponent->BindAction(TEXT("Boost"), IE_Released, this, &AShipPawn::BoostReleased);
}

// --- Translacija: puni LocalVelocity ose ---
void AShipPawn::MoveForward(float Value) { LocalVelocity.X = Value; }
void AShipPawn::MoveRight  (float Value) { LocalVelocity.Y = Value; }
void AShipPawn::MoveUp     (float Value) { LocalVelocity.Z = Value; }

// --- Rotacija: primeni ugaone brzine po tik-u (raw osećaj miša) ---
void AShipPawn::Yaw(float Value)
{
    if (FMath::IsNearlyZero(Value)) return;
    const float d = MouseSensitivityDegPerSec * Value * GetWorld()->GetDeltaSeconds();
    AddActorLocalRotation(FRotator(0.f, d, 0.f));
}

void AShipPawn::Pitch(float Value)
{
    if (FMath::IsNearlyZero(Value)) return;
    const float d = -MouseSensitivityDegPerSec * Value * GetWorld()->GetDeltaSeconds(); // invert Y kao u FPS
    AddActorLocalRotation(FRotator(d, 0.f, 0.f));
}

void AShipPawn::Roll(float Value)
{
    if (FMath::IsNearlyZero(Value)) return;
    const float d = RollSpeedDegPerSec * Value * GetWorld()->GetDeltaSeconds();
    AddActorLocalRotation(FRotator(0.f, 0.f, d));
}

// --- Boost ---
void AShipPawn::BoostPressed()  { bIsBoosting = true;  }
void AShipPawn::BoostReleased() { bIsBoosting = false; }

// --- Debug kolizije ---
void AShipPawn::DebugCollision()
{
    if (!ShipMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("ShipMesh is NULL"));
        return;
    }

    const bool bSim = ShipMesh->IsSimulatingPhysics();
    const ECollisionEnabled::Type CE = ShipMesh->GetCollisionEnabled();
    const FName Profile = ShipMesh->GetCollisionProfileName();

    UE_LOG(LogTemp, Log, TEXT("[COLLISION] ShipMesh: SimPhys=%s, Enabled=%d, Profile=%s"),
        bSim ? TEXT("true") : TEXT("false"), (int32)CE, *Profile.ToString());

    UE_LOG(LogTemp, Log, TEXT("[COLLISION] Responses: WS=%d WD=%d Pawn=%d"),
        (int32)ShipMesh->GetCollisionResponseToChannel(ECC_WorldStatic),
        (int32)ShipMesh->GetCollisionResponseToChannel(ECC_WorldDynamic),
        (int32)ShipMesh->GetCollisionResponseToChannel(ECC_Pawn));

    if (GEngine)
    {
        const FString Msg = FString::Printf(TEXT("SimPhys=%d, Enabled=%d, Profile=%s"),
            bSim, (int32)CE, *Profile.ToString());
        GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, Msg);
    }

    DebugSweepForward();
}

void AShipPawn::DebugSweepForward()
{
    const FVector Start = GetActorLocation();
    const FVector End   = Start + GetActorForwardVector() * 200.f;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(DebugSweep), false, this);
    FCollisionResponseParams RespParams;
    FHitResult Hit;

    const float Radius = 50.f;
    const bool bHit = GetWorld()->SweepSingleByChannel(
        Hit, Start, End, FQuat::Identity, ECC_WorldStatic,
        FCollisionShape::MakeSphere(Radius), Params, RespParams
    );

    DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Red : FColor::Cyan, false, 2.f, 0, 2.f);
    DrawDebugSphere(GetWorld(), bHit ? Hit.ImpactPoint : End, Radius, 16, bHit ? FColor::Red : FColor::Cyan, false, 2.f);

    if (bHit)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SWEEP] Blocked by %s at %s normal=%s"),
            *GetNameSafe(Hit.GetActor()),
            *Hit.ImpactPoint.ToString(),
            *Hit.ImpactNormal.ToString());
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("SWEEP: BLOCKED"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[SWEEP] No hit"));
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("SWEEP: no hit"));
    }
}
