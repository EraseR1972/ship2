#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingOriginManager.generated.h"

/**
 * Floating Origin manager
 * - Rebase by sector steps
 * - Move only filtered, Movable actors
 * - Keep player near origin
 * - Applies accumulated shift to newly spawned / streamed actors
 */
UCLASS()
class SHIP2_API AFloatingOriginManager : public AActor
{
    GENERATED_BODY()

public:
    AFloatingOriginManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

public:
    /* ===== Settings ===== */
    // Sector size (world units)
    UPROPERTY(EditAnywhere, Category="Sectors")
    float SectorSize = 10000.f;

    // Minimum seconds between two rebase operations
    UPROPERTY(EditAnywhere, Category="Sectors")
    float RebaseCooldown = 0.2f;

    // Require actor to be in this Actor Layer (optional)
    UPROPERTY(EditAnywhere, Category="Filter")
    FName RequiredLayer = NAME_None;

    // Require actor to be in this Data Layer (optional, World Partition)
    UPROPERTY(EditAnywhere, Category="Filter")
    FName RequiredDataLayer = NAME_None;

    // Player pawn to keep near origin
    UPROPERTY(EditAnywhere, Category="Target")
    APawn* TargetPawn = nullptr;

private:
    /* ===== Runtime ===== */
    // Accumulated total world shift (sum of all offsets applied so far)
    FVector AccumulatedShift = FVector::ZeroVector;

    // Cooldown timer
    float TimeSinceLastRebase = 0.f;

    // Delegates
    FDelegateHandle SpawnHandle;
    FDelegateHandle LevelAddedHandle;

private:
    void RebaseWorldIfNeeded();
    void ApplyShiftToActor(AActor* A, const FVector& Shift);
    bool ShouldShiftActor(UWorld* World, AActor* Actor) const;

    // Handlers for newly spawned / streamed-in actors
    void OnActorSpawned(AActor* A);
    void OnLevelAddedToWorld(ULevel* Level, UWorld* InWorld);
};