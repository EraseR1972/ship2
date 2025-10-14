#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingOriginManager.generated.h"

class APawn;

/**
 * FloatingOriginManager (Sector/Chunk based, Infinite Space)
 * - Ne koristi SetNewWorldOrigin.
 * - Održava "prozor" sektora oko igrača: wrap-uje/pomera aktere koji ispadnu iz prozora
 *   za celobrojne veličine sektora (torus wrapping).
 * - Crta vizuelne "box" sektore oko igrača.
 */
UCLASS()
class SHIP2_API AFloatingOriginManager : public AActor
{
    GENERATED_BODY()

public:
    AFloatingOriginManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    /** Pawn koji se prati (ako je null, automatski uzima PlayerPawn(0)). */
    UPROPERTY(EditAnywhere, Category="Sectors")
    APawn* TargetPawn = nullptr;

    /** Veličina sektora (dužina ivice box-a) u cm. */
    UPROPERTY(EditAnywhere, Category="Sectors", meta=(ClampMin="1000.0", UIMin="1000.0"))
    float SectorSize = 50000.f;

    /** Poluprečnik prozora (koliko sektora po osi oko igrača držimo aktivno). */
    UPROPERTY(EditAnywhere, Category="Sectors", meta=(ClampMin="1", UIMin="1"))
    int32 WindowRadius = 1; // ukupno 3x3x3 ako je 1

    /** Period osvežavanja sektorskog wrappovanja (sekunde). 0 = svaki frame. */
    UPROPERTY(EditAnywhere, Category="Sectors", meta=(ClampMin="0.0", UIMin="0.0"))
    float UpdatePeriod = 0.1f;

    /** (Opcionalno) Pomeraj/Wrap-uj samo aktere sa ovim tagom. */
    UPROPERTY(EditAnywhere, Category="Sectors|Filter")
    bool bUseTagFilter = false;

    /** Tag koji obeležava aktere koji se pomeraju (koristi se ako je bUseTagFilter = true). */
    UPROPERTY(EditAnywhere, Category="Sectors|Filter")
    FName MoveTag = FName(TEXT("Shiftable"));

    /** Debug prikaz sektora. */
    UPROPERTY(EditAnywhere, Category="Sectors|Debug")
    bool bDrawSectorBoxes = true;

    /** Koliko okvira sektora crtati po osi (uvek 2*WindowRadius+1). */
    UPROPERTY(EditAnywhere, Category="Sectors|Debug", meta=(ClampMin="1", UIMin="1"))
    int32 DebugDrawExtraRadius = 0; // dodatnih prstenova za prikaz (samo debug)

    /** Broj wrap-ova koje smo uradili. */
    UPROPERTY(VisibleAnywhere, Category="Sectors|Debug")
    int32 WrapCount = 0;

    /** Interno vreme za periodično osvežavanje. */
    float TimeAcc = 0.f;

private:
    void UpdateSectors(float DeltaTime);
    void WrapActorsToWindow(const FIntVector& PlayerSector, int32 Radius);
    void DrawSectorGrid(const FIntVector& PlayerSector, int32 Radius) const;

    static FIntVector WorldToSector(const FVector& Position, float InSectorSize);
    static FVector SectorToWorldCenter(const FIntVector& Sector, float InSectorSize);
};