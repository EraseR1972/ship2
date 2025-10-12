#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "ShipPawn.generated.h"   // ⇦ MORA da bude POSLEDNJI include

UCLASS()
class SHIP2_API AShipPawn : public APawn
{
    GENERATED_BODY()

public:
    AShipPawn();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* ShipMesh;

    UPROPERTY(VisibleAnywhere)
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere)
    UFloatingPawnMovement* MovementComponent;

    void MoveForward(float Value);
    void MoveRight(float Value);
};
