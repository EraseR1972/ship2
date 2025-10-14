
#include "FloatingOriginManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"

AFloatingOriginManager::AFloatingOriginManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AFloatingOriginManager::BeginPlay()
{
    Super::BeginPlay();

    if (!TargetPawn)
    {
        TargetPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    }

    UE_LOG(LogTemp, Display, TEXT("[FloatingOrigin] Initialized (DebugProtected) TargetPawn=%p World=%p"), TargetPawn, GetWorld());

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("[FloatingOrigin] DebugProtected version running"));
    }
}

void AFloatingOriginManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsValid(this) || !IsValid(GetWorld()))
    {
        UE_LOG(LogTemp, Warning, TEXT("[FloatingOrigin] Invalid self or world in Tick"));
        return;
    }

    if (CooldownAfterRebase > 0.f)
    {
        CooldownAfterRebase -= DeltaTime;
        return;
    }

    if (IsValid(TargetPawn))
    {
        CheckAndRebaseOrigin();
        DrawDebugInfo();
    }
    else
    {
        TargetPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    }
}

void AFloatingOriginManager::CheckAndRebaseOrigin()
{
    if (!IsValid(this) || !IsValid(GetWorld())) return;
    if (!IsValid(TargetPawn)) return;

    UWorld* World = GetWorld();
    const FVector Loc = TargetPawn->GetActorLocation();

    const float RebaseTrigger = OriginThreshold;
    const float RebaseResetZone = OriginThreshold * 0.6f;

    const bool bShouldRebase =
        (FMath::Abs(Loc.X) > RebaseTrigger ||
         FMath::Abs(Loc.Y) > RebaseTrigger ||
         FMath::Abs(Loc.Z) > RebaseTrigger);

    const bool bTooCloseToZero =
        (FMath::Abs(Loc.X) < RebaseResetZone &&
         FMath::Abs(Loc.Y) < RebaseResetZone &&
         FMath::Abs(Loc.Z) < RebaseResetZone);

    if (!bShouldRebase || bTooCloseToZero)
        return;

    const FIntVector Offset(
        FMath::RoundToInt(Loc.X / GridSnap) * (int32)GridSnap,
        FMath::RoundToInt(Loc.Y / GridSnap) * (int32)GridSnap,
        FMath::RoundToInt(Loc.Z / GridSnap) * (int32)GridSnap);

    static FIntVector LastOffset = FIntVector::ZeroValue;
    if (Offset == LastOffset)
        return;
    LastOffset = Offset;

    UE_LOG(LogTemp, Display, TEXT("[FloatingOrigin] Rebase trigger #%d Loc=%s Offset=%s"), RebaseCount+1, *Loc.ToString(), *Offset.ToString());

    SmoothCameraFade(true);

    if (!IsValid(World))
    {
        UE_LOG(LogTemp, Error, TEXT("[FloatingOrigin] World invalid before SetNewWorldOrigin"));
        return;
    }

    World->SetNewWorldOrigin(Offset);

    // --- Revalidate references ---
    if (!IsValid(World))
    {
        UE_LOG(LogTemp, Warning, TEXT("[FloatingOrigin] World invalid after rebasing!"));
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!IsValid(PC))
    {
        UE_LOG(LogTemp, Warning, TEXT("[FloatingOrigin] PlayerController invalid after rebasing!"));
        return;
    }

    if (!IsValid(TargetPawn))
    {
        TargetPawn = PC->GetPawn();
        if (!IsValid(TargetPawn))
            TargetPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    }

    UE_LOG(LogTemp, Display, TEXT("[FloatingOrigin] Revalidated → TargetPawn=%p PC=%p World=%p"), TargetPawn, PC, World);

    RebaseCount++;
    CooldownAfterRebase = 0.5f;

    if (IsValid(World) && IsValid(TargetPawn))
    {
        DrawDebugSphere(World, TargetPawn->GetActorLocation(), 150.f, 24, FColor::Yellow, false, 5.f);
        DrawDebugLine(World, FVector::ZeroVector, TargetPawn->GetActorLocation(), FColor::Yellow, false, 5.f, 0, 2.f);
    }

    if (GEngine)
    {
        const FString Msg = FString::Printf(TEXT("[FloatingOrigin] #%d Rebased → Offset: %s"), RebaseCount, *Offset.ToString());
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, Msg);
    }

    SmoothCameraFade(false);
}

void AFloatingOriginManager::DrawDebugInfo()
{
    if (!IsValid(GetWorld()) || !IsValid(TargetPawn) || !GEngine) return;

    UWorld* World = GetWorld();
    if (!IsValid(World)) return;

    const FVector Loc = TargetPawn->GetActorLocation();
    const FIntVector Origin = World->OriginLocation;

    const FString PosText = FString::Printf(TEXT("Player Pos: X=%.0f  Y=%.0f  Z=%.0f"), Loc.X, Loc.Y, Loc.Z);
    const FString ShiftText = FString::Printf(TEXT("World Shifts: %d"), RebaseCount);
    const FString OriginText = FString::Printf(TEXT("World Origin: X=%d  Y=%d  Z=%d"), Origin.X, Origin.Y, Origin.Z);

    const uint64 BaseKey = reinterpret_cast<uint64>(this);
    GEngine->AddOnScreenDebugMessage(BaseKey + 0, 0.f, FColor::Cyan, PosText);
    GEngine->AddOnScreenDebugMessage(BaseKey + 1, 0.f, FColor::Orange, ShiftText);
    GEngine->AddOnScreenDebugMessage(BaseKey + 2, 0.f, FColor::Silver, OriginText);
}

void AFloatingOriginManager::SmoothCameraFade(bool bFadeOut)
{
    if (FadeTime <= 0.f) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!IsValid(PC) || !IsValid(PC->PlayerCameraManager))
    {
        UE_LOG(LogTemp, Warning, TEXT("[FloatingOrigin] Invalid camera manager in fade"));
        return;
    }

    PC->PlayerCameraManager->StartCameraFade(bFadeOut ? 0.f : 1.f, bFadeOut ? 1.f : 0.f, FadeTime, FLinearColor::Black, false, true);
}
