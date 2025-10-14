
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingOriginManager.generated.h"

class APawn;

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
	UPROPERTY(EditAnywhere, Category="Floating Origin")
	APawn* TargetPawn = nullptr;

	UPROPERTY(EditAnywhere, Category="Floating Origin")
	float OriginThreshold = 10000.f;

	UPROPERTY(EditAnywhere, Category="Floating Origin")
	float GridSnap = 1000.f;

	UPROPERTY(EditAnywhere, Category="Floating Origin")
	float FadeTime = 0.2f;

	UPROPERTY(VisibleAnywhere, Category="Floating Origin|Debug")
	int32 RebaseCount = 0;

	float CooldownAfterRebase = 0.f;

	void CheckAndRebaseOrigin();
	void SmoothCameraFade(bool bFadeOut);
	void DrawDebugInfo();
};
