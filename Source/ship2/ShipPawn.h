#pragma once // Sprečava višestruko uključivanje header fajla kako bi se izbegle greške pri kompajliranju

#include "CoreMinimal.h" // Uključuje osnovne definicije Unreal Engine-a potrebne za rad
#include "GameFramework/Pawn.h" // Uključuje baznu klasu za Pawn, koja predstavlja kontrolisane objekte u igri
#include "Camera/CameraComponent.h" // Uključuje komponentu za upravljanje kamerom u igri
#include "Components/StaticMeshComponent.h" // Uključuje komponentu za statički mesh (vizuelni model broda)
#include "GameFramework/FloatingPawnMovement.h" // Uključuje komponentu za kretanje bez gravitacije (6DOF)
#include "ShipPawn.generated.h" // Uključuje automatski generisane definicije za UCLASS

UCLASS() // Označava da je ovo Unreal Engine klasa
class SHIP2_API AShipPawn : public APawn // Definiše klasu AShipPawn koja nasleđuje APawn za upravljanje igračem
{
    GENERATED_BODY() // Automatski generiše potrebne makroe za Unreal Engine klasu

public: // Javne funkcije, promenljive i metode
    AShipPawn(); // Deklaracija konstruktora za inicijalizaciju broda

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") // Svojstvo za boost stanje, može se menjati u editoru i Blueprint-u
        bool bIsBoosting = false; // Promenljiva koja označava da li je boost (ubrzanje) aktivan

protected: // Zaštićene funkcije dostupne naslednicima
    virtual void BeginPlay() override; // Preklopljena funkcija koja se poziva kada se objekat spawnuje u igri

public: // Javne funkcije dostupne svima
    virtual void Tick(float DeltaTime) override; // Preklopljena funkcija koja se poziva svaki frejm za ažuriranje
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // Preklopljena funkcija za povezivanje inputa igrača

private: // Privatne promenljive i funkcije, dostupne samo unutar klase
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true")) // Svojstvo vidljivo u editoru, samo za čitanje u Blueprint-u, u kategoriji "Components"
        UStaticMeshComponent* ShipMesh; // Pokazivač na statički mesh komponentu koja predstavlja vizuelni model broda

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true")) // Svojstvo za kameru, vidljivo u editoru
        UCameraComponent* Camera; // Pokazivač na komponentu kamere koja prati brod

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true")) // Svojstvo za komponentu kretanja, vidljivo u editoru
        UFloatingPawnMovement* MovementComponent; // Pokazivač na komponentu za kretanje bez gravitacije (6DOF)

    void MoveForward(float Value); // Funkcija za kretanje napred/nazad (W/S tasteri)
    void MoveRight(float Value); // Funkcija za kretanje levo/desno (A/D tasteri)
    void MoveUp(float Value); // Funkcija za kretanje gore/dole (Space/Left Ctrl tasteri)
    void Turn(float Value); // Funkcija za yaw rotaciju (Mouse X, levo/desno)
    void LookUp(float Value); // Funkcija za pitch rotaciju (Mouse Y, gore/dole)
    void Roll(float Value); // Funkcija za roll rotaciju (Q/E tasteri)
    void StartBoost(); // Funkcija za aktiviranje boost-a (Left Shift pritisnut)
    void StopBoost(); // Funkcija za deaktiviranje boost-a (Left Shift otpušten)
};