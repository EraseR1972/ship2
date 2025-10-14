#include "FloatingOriginManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h" // TActorIterator
#include "Math/UnrealMathUtility.h"

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

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
            TEXT("[Sectors] Manager ready (chunk wrap + visual boxes)"));
    }
}

void AFloatingOriginManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsValid(TargetPawn))
    {
        TargetPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (!IsValid(TargetPawn)) return;
    }

    UpdateSectors(DeltaTime);
}

void AFloatingOriginManager::UpdateSectors(float DeltaTime)
{
    TimeAcc += DeltaTime;
    if (UpdatePeriod > 0.f && TimeAcc < UpdatePeriod)
        return;

    TimeAcc = 0.f;

    UWorld* World = GetWorld();
    if (!IsValid(World) || !IsValid(TargetPawn)) return;

    const FVector PawnLoc = TargetPawn->GetActorLocation();
    const FIntVector PlayerSector = WorldToSector(PawnLoc, SectorSize);

    // Wrap aktere koji su van prozora
    WrapActorsToWindow(PlayerSector, WindowRadius);

    // Debug vizuelizacija sektora
    if (bDrawSectorBoxes)
    {
        DrawSectorGrid(PlayerSector, WindowRadius + DebugDrawExtraRadius);
    }

    // HUD kratki
    if (GEngine)
    {
        const uint64 Key = reinterpret_cast<uint64>(this);
        GEngine->AddOnScreenDebugMessage(Key + 0, 0.f, FColor::Cyan,
            FString::Printf(TEXT("Player Sector: [%d, %d, %d]"), PlayerSector.X, PlayerSector.Y, PlayerSector.Z));
        GEngine->AddOnScreenDebugMessage(Key + 1, 0.f, FColor::Orange,
            FString::Printf(TEXT("WrapCount: %d"), WrapCount));
    }
}

void AFloatingOriginManager::WrapActorsToWindow(const FIntVector& PlayerSector, int32 Radius)
{
    UWorld* World = GetWorld();
    if (!IsValid(World)) return;

    const int32 MinX = PlayerSector.X - Radius;
    const int32 MaxX = PlayerSector.X + Radius;
    const int32 MinY = PlayerSector.Y - Radius;
    const int32 MaxY = PlayerSector.Y + Radius;
    const int32 MinZ = PlayerSector.Z - Radius;
    const int32 MaxZ = PlayerSector.Z + Radius;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!IsValid(Actor)) continue;
        if (Actor == TargetPawn || Actor == this) continue;

        if (bUseTagFilter && !Actor->ActorHasTag(MoveTag))
            continue;

        const FVector Loc = Actor->GetActorLocation();
        const FIntVector ASector = WorldToSector(Loc, SectorSize);

        FIntVector ShiftSectors(0,0,0);

        if (ASector.X < MinX)      ShiftSectors.X = (MinX - ASector.X);
        else if (ASector.X > MaxX) ShiftSectors.X = (MaxX - ASector.X);

        if (ASector.Y < MinY)      ShiftSectors.Y = (MinY - ASector.Y);
        else if (ASector.Y > MaxY) ShiftSectors.Y = (MaxY - ASector.Y);

        if (ASector.Z < MinZ)      ShiftSectors.Z = (MinZ - ASector.Z);
        else if (ASector.Z > MaxZ) ShiftSectors.Z = (MaxZ - ASector.Z);

        if (ShiftSectors != FIntVector::ZeroValue)
        {
            const FVector Delta = FVector(ShiftSectors) * SectorSize;
            Actor->AddActorWorldOffset(Delta, false, nullptr, ETeleportType::TeleportPhysics);
            ++WrapCount;
        }
    }
}

void AFloatingOriginManager::DrawSectorGrid(const FIntVector& PlayerSector, int32 Radius) const
{
    UWorld* World = GetWorld();
    if (!IsValid(World)) return;

    const float Half = SectorSize * 0.5f;
    const FVector Extents(Half, Half, Half);

    for (int32 x = -Radius; x <= Radius; ++x)
    {
        for (int32 y = -Radius; y <= Radius; ++y)
        {
            for (int32 z = -Radius; z <= Radius; ++z)
            {
                const FIntVector S = PlayerSector + FIntVector(x,y,z);
                const FVector Center = SectorToWorldCenter(S, SectorSize);
                DrawDebugBox(World, Center, Extents, FColor::Green, false, UpdatePeriod > 0.f ? UpdatePeriod : 0.f, 0, 1.5f);
            }
        }
    }
}

FIntVector AFloatingOriginManager::WorldToSector(const FVector& Position, float InSectorSize)
{
    // floor po sektoru, radi i za negativne pozicije
    const int32 sx = FMath::FloorToInt(Position.X / InSectorSize);
    const int32 sy = FMath::FloorToInt(Position.Y / InSectorSize);
    const int32 sz = FMath::FloorToInt(Position.Z / InSectorSize);
    return FIntVector(sx, sy, sz);
}

FVector AFloatingOriginManager::SectorToWorldCenter(const FIntVector& Sector, float InSectorSize)
{
    return FVector(Sector) * InSectorSize + FVector(InSectorSize * 0.5f);
}