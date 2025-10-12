#include "ShipPawn.h" // Uključuje header fajl za AShipPawn klasu
#include "Components/InputComponent.h" // Uključuje komponentu za obradu inputa
#include "GameFramework/FloatingPawnMovement.h" // Uključuje komponentu za kretanje bez gravitacije
#include "Camera/CameraComponent.h" // Uključuje komponentu za kameru
#include "Components/SpringArmComponent.h" // Uključuje komponentu za spring arm
#include "DrawDebugHelpers.h" // Uključuje funkcije za debag linije
#include "Engine/Engine.h" // Uključuje osnovne funkcije Unreal Engine-a
#include "GameFramework/PlayerController.h" // Uključuje klasu za kontroler igrača
#include "Camera/PlayerCameraManager.h" // Uključuje klasu za upravljanje kamerom

AShipPawn::AShipPawn() // Konstruktor klase AShipPawn
{
    PrimaryActorTick.bCanEverTick = true; // Omogućava Tick funkciju svaki frejm

    // Podešavanje ShipMesh
    ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh")); // Kreira statički mesh komponentu za brod
    RootComponent = ShipMesh; // Postavlja ShipMesh kao koreni komponent
    ShipMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Isključuje koliziju za slobodno kretanje
    ShipMesh->SetSimulatePhysics(false); // Isključuje fiziku za mesh
    ShipMesh->SetMobility(EComponentMobility::Movable); // Omogućava kretanje mesha

    // Podešavanje SpringArm
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm")); // Kreira spring arm komponentu
    SpringArm->SetupAttachment(RootComponent); // Pričvršćuje spring arm za ShipMesh
    SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 0.f)); // Postavlja relativnu poziciju na (0,0,0)
    SpringArm->TargetArmLength = 300.f; // Postavlja dužinu spring arm-a na 300 jedinica
    SpringArm->bUsePawnControlRotation = false; // Isključuje kontrolu rotacije od strane kontrolera
    SpringArm->bEnableCameraLag = false; // Isključuje kašnjenje kamere
    SpringArm->bEnableCameraRotationLag = false; // Isključuje kašnjenje rotacije kamere
    SpringArm->bDoCollisionTest = false; // Isključuje proveru kolizije za spring arm
    SpringArm->bInheritPitch = true; // Omogućava nasleđivanje pitch rotacije broda
    SpringArm->bInheritYaw = true; // Omogućava nasleđivanje yaw rotacije broda
    SpringArm->bInheritRoll = true; // Omogućava nasleđivanje roll rotacije broda

    // Podešavanje Camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera")); // Kreira komponentu kamere
    Camera->SetupAttachment(SpringArm); // Pričvršćuje kameru za spring arm
    Camera->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f)); // Postavlja blagi nagib kamere nadole
    Camera->bUsePawnControlRotation = false; // Isključuje kontrolu rotacije od strane kontrolera

    // Podešavanje MovementComponent
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent")); // Kreira komponentu za kretanje
    MovementComponent->UpdatedComponent = RootComponent; // Postavlja ShipMesh kao komponentu za kretanje
    MovementComponent->MaxSpeed = 1000.f; // Postavlja maksimalnu brzinu na 1000 jedinica/sek
    MovementComponent->Acceleration = 5000.f; // Postavlja ubrzanje na 5000 jedinica/sek²
    MovementComponent->Deceleration = 3000.f; // Postavlja usporavanje na 3000 jedinica/sek²
    MovementComponent->bEnablePhysicsInteraction = false; // Isključuje interakciju sa fizikom

    AutoPossessPlayer = EAutoReceiveInput::Player0; // Automatski povezuje brod sa igračem 0

    UE_LOG(LogTemp, Warning, TEXT("ShipPawn konstruktor pozvan")); // Loguje poruku da je konstruktor pozvan
}

void AShipPawn::BeginPlay() // Funkcija koja se poziva na početku igre
{
    Super::BeginPlay(); // Poziva baznu verziju BeginPlay funkcije

    // Onemogućavanje interferencije PlayerCameraManager-a
    if (APlayerController* PC = Cast<APlayerController>(GetController())) // Proverava da li je kontroler igrača validan
    {
        if (APlayerCameraManager* CamMgr = PC->PlayerCameraManager) // Proverava da li je menadžer kamere dostupan
        {
            CamMgr->bUseClientSideCameraUpdates = false; // Isključuje automatsko ažuriranje kamere
            CamMgr->SetGameCamera(this); // Postavlja ovaj Pawn kao glavnu kameru
            UE_LOG(LogTemp, Warning, TEXT("CameraManager bUseClientSideCameraUpdates = %d"), CamMgr->bUseClientSideCameraUpdates); // Loguje stanje camera managera
        }
        PC->SetIgnoreInput(false); // Osigurava da input nije ignorisan
    }

    UE_LOG(LogTemp, Warning, TEXT("Free Cam ShipPawn spreman")); // Loguje poruku da je Pawn spreman
    if (GEngine) // Proverava da li je Game Engine dostupan
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("AShipPawn::BeginPlay() pozvan")); // Ispisuje poruku na ekranu
    }

    // Debag linija za forward pravac
    DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 300.f, FColor::Red, false, 5.f, 0, 5.f); // Crtanje crvene linije za forward pravac
}

void AShipPawn::Tick(float DeltaTime) // Funkcija koja se poziva svaki frejm
{
    Super::Tick(DeltaTime); // Poziva baznu verziju Tick funkcije

    // Debag: Prikaz rotacija
    if (GEngine) // Proverava da li je Game Engine dostupan
    {
        FRotator ShipRot = GetActorRotation(); // Dobija rotaciju broda
        FRotator CamRot = Camera->GetComponentRotation(); // Dobija rotaciju kamere
        FString Msg = FString::Printf(TEXT("Ship Yaw: %.1f, Pitch: %.1f, Roll: %.1f | Cam Yaw: %.1f, Pitch: %.1f"),
            ShipRot.Yaw, ShipRot.Pitch, ShipRot.Roll, CamRot.Yaw, CamRot.Pitch); // Formatira poruku sa rotacijama
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, Msg); // Ispisuje rotacije na ekranu
    }
}

void AShipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) // Podešava inpute za kontrolu
{
    Super::SetupPlayerInputComponent(PlayerInputComponent); // Poziva baznu verziju funkcije

    PlayerInputComponent->BindAxis("MoveForward", this, &AShipPawn::MoveForward); // Povezuje osa za kretanje napred/nazad
    PlayerInputComponent->BindAxis("MoveRight", this, &AShipPawn::MoveRight); // Povezuje osa za kretanje levo/desno
    PlayerInputComponent->BindAxis("MoveUp", this, &AShipPawn::MoveUp); // Povezuje osa za kretanje gore/dole
    PlayerInputComponent->BindAxis("Turn", this, &AShipPawn::Turn); // Povezuje osa za yaw (Mouse X)
    PlayerInputComponent->BindAxis("LookUp", this, &AShipPawn::LookUp); // Povezuje osa za pitch (Mouse Y)
    PlayerInputComponent->BindAxis("Roll", this, &AShipPawn::Roll); // Povezuje osa za roll (Q/E)
    PlayerInputComponent->BindAction("Boost", IE_Pressed, this, &AShipPawn::StartBoost); // Povezuje akciju za start boost-a
    PlayerInputComponent->BindAction("Boost", IE_Released, this, &AShipPawn::StopBoost); // Povezuje akciju za kraj boost-a
}

void AShipPawn::MoveForward(float Value) // Funkcija za kretanje napred/nazad
{
    if (FMath::Abs(Value) > 0.1f) // Proverava da li je input značajan
    {
        AddMovementInput(GetActorForwardVector(), Value); // Dodaje kretanje u pravcu forward vektora
        DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 200.f * FMath::Sign(Value), FColor::Blue, false, 0.f, 0, 2.0f); // Crtanje plave linije za forward kretanje
        UE_LOG(LogTemp, Warning, TEXT("MoveForward pozvan sa Value = %f"), Value); // Loguje vrednost inputa
    }
}

void AShipPawn::MoveRight(float Value) // Funkcija za kretanje levo/desno
{
    if (FMath::Abs(Value) > 0.1f) // Proverava da li je input značajan
    {
        AddMovementInput(GetActorRightVector(), Value); // Dodaje kretanje u pravcu right vektora
        DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorRightVector() * 200.f * FMath::Sign(Value), FColor::Green, false, 0.f, 0, 2.0f); // Crtanje zelene linije za right kretanje
        UE_LOG(LogTemp, Warning, TEXT("MoveRight pozvan sa Value = %f"), Value); // Loguje vrednost inputa
    }
}

void AShipPawn::MoveUp(float Value) // Funkcija za kretanje gore/dole
{
    if (FMath::Abs(Value) > 0.1f) // Proverava da li je input značajan
    {
        AddMovementInput(GetActorUpVector(), Value); // Dodaje kretanje u pravcu up vektora
        DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorUpVector() * 200.f * FMath::Sign(Value), FColor::White, false, 0.f, 0, 2.0f); // Crtanje bele linije za up/down kretanje
        UE_LOG(LogTemp, Warning, TEXT("MoveUp pozvan sa Value = %f"), Value); // Loguje vrednost inputa
    }
}

void AShipPawn::Turn(float Value) // Funkcija za yaw rotaciju (Mouse X)
{
    if (FMath::Abs(Value) > 0.1f) // Proverava da li je input značajan
    {
        FQuat CurrentQuat = GetActorRotation().Quaternion(); // Dobija trenutnu rotaciju broda kao kvaternion
        FVector LocalUp = GetActorUpVector(); // Dobija lokalnu Z-osu za yaw
        FQuat YawQuat = FQuat(LocalUp, FMath::DegreesToRadians(Value)); // Kreira kvaternion za yaw rotaciju
        SetActorRotation((YawQuat * CurrentQuat).GetNormalized()); // Postavlja novu rotaciju broda
        UE_LOG(LogTemp, Warning, TEXT("Turn pozvan sa Value = %f, Yaw = %f"), Value, GetActorRotation().Yaw); // Loguje input i trenutni yaw
    }
}

void AShipPawn::LookUp(float Value) // Funkcija za pitch rotaciju (Mouse Y)
{
    if (FMath::Abs(Value) > 0.1f) // Proverava da li je input značajan
    {
        FQuat CurrentQuat = GetActorRotation().Quaternion(); // Dobija trenutnu rotaciju broda kao kvaternion
        FVector LocalRight = GetActorRightVector(); // Dobija lokalnu X-osu za pitch
        FQuat PitchQuat = FQuat(LocalRight, FMath::DegreesToRadians(Value)); // Kreira kvaternion za pitch rotaciju
        SetActorRotation((PitchQuat * CurrentQuat).GetNormalized()); // Postavlja novu rotaciju broda
        UE_LOG(LogTemp, Warning, TEXT("LookUp pozvan sa Value = %f, Pitch = %f"), Value, GetActorRotation().Pitch); // Loguje input i trenutni pitch
    }
}

void AShipPawn::Roll(float Value) // Funkcija za roll rotaciju (Q/E)
{
    if (FMath::Abs(Value) > 0.1f) // Proverava da li je input značajan
    {
        FQuat CurrentQuat = GetActorRotation().Quaternion(); // Dobija trenutnu rotaciju broda kao kvaternion
        FVector LocalForward = GetActorForwardVector(); // Dobija lokalnu Y-osu za roll
        FQuat RollQuat = FQuat(LocalForward, FMath::DegreesToRadians(Value)); // Kreira kvaternion za roll rotaciju
        SetActorRotation((RollQuat * CurrentQuat).GetNormalized()); // Postavlja novu rotaciju broda
        UE_LOG(LogTemp, Warning, TEXT("Roll pozvan sa Value = %f, Roll = %f"), Value, GetActorRotation().Roll); // Loguje input i trenutni roll
    }
}

void AShipPawn::StartBoost() // Funkcija za aktiviranje boost-a
{
    if (!bIsBoosting && MovementComponent) // Proverava da li boost nije aktivan i da li je MovementComponent validan
    {
        bIsBoosting = true; // Postavlja boost na aktivno
        MovementComponent->MaxSpeed *= 3.f; // Povećava maksimalnu brzinu za 3x
        MovementComponent->Acceleration *= 2.f; // Povećava ubrzanje za 2x
        if (GEngine) // Proverava da li je Game Engine dostupan
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("BOOST UKLJUCEN")); // Ispisuje poruku da je boost uključen
        }
        UE_LOG(LogTemp, Warning, TEXT("Boost aktiviran. MaxSpeed = %f, Acceleration = %f"), MovementComponent->MaxSpeed, MovementComponent->Acceleration); // Loguje nove vrednosti brzine i ubrzanja
    }
}

void AShipPawn::StopBoost() // Funkcija za deaktiviranje boost-a
{
    if (bIsBoosting && MovementComponent) // Proverava da li je boost aktivan i da li je MovementComponent validan
    {
        bIsBoosting = false; // Postavlja boost na neaktivno
        MovementComponent->MaxSpeed /= 3.f; // Vraća maksimalnu brzinu na normalnu vrednost
        MovementComponent->Acceleration /= 2.f; // Vraća ubrzanje na normalnu vrednost
        if (GEngine) // Proverava da li je Game Engine dostupan
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver, TEXT("BOOST ISKLJUCEN")); // Ispisuje poruku da je boost isključen
        }
        UE_LOG(LogTemp, Warning, TEXT("Boost deaktiviran. MaxSpeed = %f, Acceleration = %f"), MovementComponent->MaxSpeed, MovementComponent->Acceleration); // Loguje nove vrednosti brzine i ubrzanja
    }
}