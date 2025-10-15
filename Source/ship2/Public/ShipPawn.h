#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "ShipPawn.generated.h"

/**
 * AShipPawn — 6DoF kinematički brod bez kamere u C++.
 * Kamera može biti dodata iz Blueprint-a.
 * Kolizija je na parent mesh-u (ShipMesh) koji je root.
 */
UCLASS()
class SHIP2_API AShipPawn : public APawn
{
    GENERATED_BODY()

public:
    AShipPawn();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    virtual void Tick(float DeltaTime) override;

private:
    /* --- Components --- */
    // Root i collider
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship", meta=(AllowPrivateAccess="true"))
    UStaticMeshComponent* ShipMesh;

    /* --- Kretanje i rotacija --- */
    void MoveForward(float Value);
    void MoveRight(float Value);
    void MoveUp(float Value);

    void Pitch(float Value);
    void Yaw(float Value);
    void Roll(float Value);

    void BoostPressed();
    void BoostReleased();

    /* --- Debug --- */
    UFUNCTION(Exec)
    void DebugCollision();

    void DebugSweepForward() const;

private:
    /* --- Parametri --- */
    UPROPERTY(EditAnywhere, Category="Ship|Movement")
    float MoveSpeed = 1200.f;

    UPROPERTY(EditAnywhere, Category="Ship|Movement")
    float BoostMultiplier = 4.f;

    UPROPERTY(EditAnywhere, Category="Ship|Rotation")
    float MouseSensitivityDegPerSec = 120.f;

    UPROPERTY(EditAnywhere, Category="Ship|Rotation")
    float RollSpeedDegPerSec = 90.f;

    bool bIsBoosting = false;
    FVector LocalVelocity = FVector::ZeroVector;
};