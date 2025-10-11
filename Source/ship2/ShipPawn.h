#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ShipPawn.generated.h"

UCLASS()
class SHIP2_API AShipPawn : public APawn
{
    GENERATED_BODY()

public:
    AShipPawn();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    void MoveForward(float Value);
    void MoveRight(float Value);

private:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* ShipMesh;

    UPROPERTY(VisibleAnywhere)
    UCameraComponent* Camera;

    FVector CurrentVelocity;
};