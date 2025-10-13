#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ShipPawn.generated.h"

/**
 * AShipPawn — 6DoF kinematički brod (bez fizike), sa Boost-om i alatima za debug kolizije.
 * Napomena: Ako ti se modul ne zove Ship2, zameni SHIP2_API sa <TVOJMODUL>_API i preimenuj folder u Source/<TvojModul>/...
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
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship", meta=(AllowPrivateAccess="true"))
    UStaticMeshComponent* ShipMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship", meta=(AllowPrivateAccess="true"))
    UCameraComponent* Camera;

    /* --- Translation input (kinematic) --- */
    void MoveForward(float Value);
    void MoveRight(float Value);
    void MoveUp(float Value);

    /* --- Rotation input --- */
    void Pitch(float Value); // Mouse Y (invert kao u FPS)
    void Yaw(float Value);   // Mouse X
    void Roll(float Value);  // Q/E

    /* --- Boost (Left Shift) --- */
    void BoostPressed();
    void BoostReleased();

    /* --- Debug (console: DebugCollision) --- */
    UFUNCTION(Exec)
    void DebugCollision();

    void DebugSweepForward();

private:
    /* Tunables */
    UPROPERTY(EditAnywhere, Category="Ship|Movement")
    float MoveSpeed = 1200.f;                 // Osnovna brzina (cm/s)

    UPROPERTY(EditAnywhere, Category="Ship|Movement")
    float BoostMultiplier = 4.f;              // Faktor ubrzanja dok je Shift pritisnut

    UPROPERTY(EditAnywhere, Category="Ship|Rotation")
    float MouseSensitivityDegPerSec = 120.f;  // °/s pri inputu 1.0 (yaw/pitch)

    UPROPERTY(EditAnywhere, Category="Ship|Rotation")
    float RollSpeedDegPerSec = 90.f;          // °/s pri inputu 1.0 (roll)

    /* Runtime state */
    bool bIsBoosting = false;
    FVector LocalVelocity = FVector::ZeroVector; // X fwd, Y right, Z up
};
