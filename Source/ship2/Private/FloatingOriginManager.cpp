#include "FloatingOriginManager.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/Brush.h"
#include "GameFramework/DefaultPhysicsVolume.h"
#include "Engine/DirectionalLight.h"
#include "NavigationSystem.h"
#include "Camera/CameraActor.h"
#include "WorldPartition/DataLayer/DataLayerSubsystem.h"
#include "Components/PrimitiveComponent.h"

AFloatingOriginManager::AFloatingOriginManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AFloatingOriginManager::BeginPlay()
{
    Super::BeginPlay();

    // Subscribe to spawned actors
    if (UWorld* World = GetWorld())
    {
        SpawnHandle = World->AddOnActorSpawnedHandler(
            FOnActorSpawned::FDelegate::CreateUObject(this, &AFloatingOriginManager::OnActorSpawned)
        );
    }

    // Subscribe to level streaming (actors coming from streamed levels)
    LevelAddedHandle = FWorldDelegates::LevelAddedToWorld.AddUObject(
        this, &AFloatingOriginManager::OnLevelAddedToWorld
    );
}

void AFloatingOriginManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        if (SpawnHandle.IsValid())
        {
            World->RemoveOnActorSpawnedHandler(SpawnHandle);
            SpawnHandle.Reset();
        }
    }
    FWorldDelegates::LevelAddedToWorld.RemoveAll(this);

    Super::EndPlay(EndPlayReason);
}

void AFloatingOriginManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TimeSinceLastRebase += DeltaTime;
    RebaseWorldIfNeeded();
}

static bool IsSystemActor(const AActor* A)
{
    return  A->IsA(ABrush::StaticClass()) ||
            A->IsA(ADefaultPhysicsVolume::StaticClass()) ||
            A->IsA(APlayerStart::StaticClass()) ||
            A->IsA(ADirectionalLight::StaticClass()) ||
            A->IsA(ANavigationData::StaticClass());
}

bool AFloatingOriginManager::ShouldShiftActor(UWorld* World, AActor* Actor) const
{
    if (!IsValid(Actor)) return false;
    if (Actor == TargetPawn || Actor == this) return false;
    if (IsSystemActor(Actor)) return false;

    // Root must be Movable
    const USceneComponent* Root = Actor->GetRootComponent();
    if (!Root || Root->Mobility != EComponentMobility::Movable) return false;

    // Actor Layer filter
    if (RequiredLayer != NAME_None)
    {
        bool bInLayer = false;
        for (const FName& L : Actor->Layers)
        {
            if (L == RequiredLayer) { bInLayer = true; break; }
        }
        if (!bInLayer) return false;
    }

    // Data Layer filter
    if (RequiredDataLayer != NAME_None)
    {
        if (UDataLayerSubsystem* DLS = UWorld::GetSubsystem<UDataLayerSubsystem>(World))
        {
            const TArray<FName> Layers = DLS->GetDataLayerInstanceNames(Actor);
            if (!Layers.Contains(RequiredDataLayer)) return false;
        }
    }

    // Skip current view target CameraActor if any
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
        if (Actor == PC->GetViewTarget()) return false;

    return true;
}

void AFloatingOriginManager::ApplyShiftToActor(AActor* A, const FVector& Shift)
{
    if (!IsValid(A) || Shift.IsNearlyZero()) return;

    UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(A->GetRootComponent());
    const bool bSim = Prim ? Prim->IsSimulatingPhysics() : false;
    const ECollisionEnabled::Type PrevColl = Prim ? Prim->GetCollisionEnabled() : ECollisionEnabled::NoCollision;
    if (Prim) { Prim->SetSimulatePhysics(false); Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision); }

    const bool bPrevActorCollision = A->GetActorEnableCollision();
    A->SetActorEnableCollision(false);

    A->AddActorWorldOffset(Shift, false, nullptr, ETeleportType::TeleportPhysics);

    if (Prim)
    {
        Prim->SetCollisionEnabled(PrevColl);
        Prim->SetSimulatePhysics(bSim);
        if (bSim) Prim->WakeAllRigidBodies();
        Prim->UpdateComponentToWorld();
        Prim->MarkRenderTransformDirty();
    }
    A->SetActorEnableCollision(bPrevActorCollision);
}

void AFloatingOriginManager::OnActorSpawned(AActor* A)
{
    if (!IsValid(A)) return;
    UWorld* World = GetWorld();
    if (!World) return;

    // Apply accumulated shift to any newly spawned/streamed actor that passes filters
    if (!AccumulatedShift.IsNearlyZero() && ShouldShiftActor(World, A))
    {
        ApplyShiftToActor(A, -AccumulatedShift);
        UE_LOG(LogTemp, Verbose, TEXT("[FOM] Spawn shift %s by %s"), *A->GetName(), *AccumulatedShift.ToString());
    }
}

void AFloatingOriginManager::OnLevelAddedToWorld(ULevel* Level, UWorld* InWorld)
{
    if (!InWorld || AccumulatedShift.IsNearlyZero()) return;

    // Shift actors of this level as they come in
    for (AActor* A : Level->Actors)
    {
        if (!IsValid(A)) continue;
        if (ShouldShiftActor(InWorld, A))
        {
            ApplyShiftToActor(A, -AccumulatedShift);
            UE_LOG(LogTemp, Verbose, TEXT("[FOM] Stream shift %s by %s"), *A->GetName(), *AccumulatedShift.ToString());
        }
    }
}

void AFloatingOriginManager::RebaseWorldIfNeeded()
{
    if (!TargetPawn) return;

    // Require a filter to avoid moving whole world by accident
    if (RequiredLayer == NAME_None && RequiredDataLayer == NAME_None)
        return;

    const FVector PawnLoc = TargetPawn->GetActorLocation();

    // Sector step (integer) to avoid jitter; rebase only when leaving central sector
    const FIntVector Sector(
        FMath::FloorToInt(PawnLoc.X / SectorSize),
        FMath::FloorToInt(PawnLoc.Y / SectorSize),
        FMath::FloorToInt(PawnLoc.Z / SectorSize)
    );

    if (Sector == FIntVector::ZeroValue) return;

    if (TimeSinceLastRebase < RebaseCooldown) return;
    TimeSinceLastRebase = 0.f;

    const FVector Offset = FVector(Sector) * SectorSize;   // move by whole sectors
    AccumulatedShift += Offset;                             // remember total shift

    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* A = *It;
        if (!ShouldShiftActor(World, A)) continue;
        ApplyShiftToActor(A, -Offset);
    }

    // Keep pawn relative position inside central sector
    const FVector NewPawnLoc = PawnLoc - Offset;
    TargetPawn->SetActorLocation(NewPawnLoc, false, nullptr, ETeleportType::TeleportPhysics);

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
        if (PC->PlayerCameraManager)
            PC->PlayerCameraManager->SetGameCameraCutThisFrame();
}