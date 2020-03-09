// Fill out your copyright notice in the Description page of Project Settings.
#include "OpenDoor.h"
#include "Components/AudioComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Containers/Array.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h" //Header required to use GetOwner() function
#include "GameFramework/PlayerController.h" //Header required to use Player Controller functions

#define OUT //Used to mark output parameters for better code readability

// Sets default values for this component's properties
UOpenDoor::UOpenDoor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UOpenDoor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Door Rotator : %s"), *GetOwner()->GetActorRotation().ToCompactString());
	UE_LOG(LogTemp, Warning, TEXT("Door Yaw Value : %f"), GetOwner()->GetActorRotation().Yaw);	

	InitialYaw = GetOwner()->GetActorRotation().Yaw;
	DoorCloseAngle = InitialYaw;
	DoorOpenAngle = DoorOpenAngle * (-1.0f);
	DoorOpenAngle = InitialYaw + DoorOpenAngle;

	FindPressurePlateTriggerVolume();

	FindAudioComponent();

	//Assign the Pawn that the player controls to ActorThatCanOpenDoor which helps to specify which actor can successfully run the OpenDoor function.
	ActorThatCanOpenDoor = GetWorld()->GetFirstPlayerController()->GetPawn(); 

	UE_LOG(LogTemp, Warning, TEXT("Door Open Angle : %f"), DoorOpenAngle);
}


// Called every frame
void UOpenDoor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(PressurePlate && TotalMassOfActors() >= MinMassToOpenDoor)
	{
		OpenDoor(DeltaTime);
		DoorLastOpen = GetWorld()->GetTimeSeconds(); //DoorLastOpen will constantly get updated every frame as long as the player is standing in the PressurePlate Trigger Volume
		//UE_LOG(LogTemp, Warning, TEXT("Door Last Open : %f"), DoorLastOpen);
		return;
	}
	else
	{
		//Once the total mass of actors is less than minimum mass to open door, we will have the latest recorded GetTimeSeconds() fixed and stored in DoorLastOpen.
		//Then, by checking if the current GetTimeSeconds() is more than (>) DoorLastOpen plus DoorCloseDelay, we then choose to close the door.
		//
		//Ex: DoorLastOpen when player leaves the PressurePlate Trigger Volume = 5s
		//	  if(current GetTimeSeconds() ~= (5.2s) > DoorLastOpen + DoorCloseDelay (which is total 7s if DoorCloseDelay is set to 2.0f)) is true, then close the door.		
		//
		if(GetWorld()->GetTimeSeconds() > DoorLastOpen + DoorCloseDelay)
		{
			CloseDoor(DeltaTime);
		}
		return;
	}

}


void UOpenDoor::OpenDoor(float DeltaTime)
{
	
	FRotator NewDoorRotator = FMath::Lerp(GetOwner()->GetActorRotation(), FRotator(0.0f, DoorOpenAngle, 0.0f), DoorOpenSpeed * DeltaTime); //Multiplying DoorOpenSpeed with DeltaTime ensures the door opens at a speed according to the FPS of the current system, 
	GetOwner()->SetActorRotation(NewDoorRotator);																		      //This ensures the door opens at a desired speed regardless of the system's performance
																															  //Adjusting the alpha value (DoorOpenSpeed) which is multiplied with DeltaTime affects how fast or slow the door opens
	
	if(!AudioComponent)
	{return;}
	
	if(!bHasDoorBeenOpened)
	{
		AudioComponent->Play();
		bHasDoorBeenOpened = true;
		bHasDoorBeenClosed = false;
		return;
	}

	//float rotationYaw = GetOwner()->GetActorRotation().Yaw + 0.1f;
	//GetOwner()->SetActorRotation(FRotator(0.0f, rotationYaw, 0.0f));

	/*Use FMath::Lerp() to open door with a natural feel*/
	//Note: FMath::Lerp() first argument is current value, NOT starting value. If you put starting value, it will move only for that frame and stop completely
	//GetOwner()->SetActorRotation(FMath::Lerp(GetOwner()->GetActorRotation(), FRotator(0.0f, -90.0f, 0.0f), 0.1f));

	/*Use FInterpConstantTo() if want to open door smoothly at a constant speed*/
	// float NewDoorYaw = FMath::FInterpConstantTo(GetOwner()->GetActorRotation().Yaw, -90.0f, DeltaTime, 50);
	// FRotator NewDoorRotator(0.0f, NewDoorYaw, 0.0f);
	// GetOwner()->SetActorRotation(NewDoorRotator);
}

void UOpenDoor::CloseDoor(float DeltaTime)
{
	FRotator NewDoorCloseRotator = FMath::Lerp(GetOwner()->GetActorRotation(), FRotator(0.0f, DoorCloseAngle, 0.0f), DoorCloseSpeed * DeltaTime);
	GetOwner()->SetActorRotation(NewDoorCloseRotator);

	if(!AudioComponent)
	{return;}

	if(!bHasDoorBeenClosed)
	{
		AudioComponent->Play();
		bHasDoorBeenOpened = false;
		bHasDoorBeenClosed = true;
		return;
	}
}

float UOpenDoor::TotalMassOfActors()
{
	if(!PressurePlate)
	{
		UE_LOG(LogTemp, Error, TEXT("%s has the OpenDoor component attached, but not Pressure Plate assigned"), *GetOwner()->GetName());
		return 0.0f;
	}
	
	float TotalMass = 0.0f;

	TArray<AActor*> OverlappingActors;
	PressurePlate->GetOverlappingActors(OUT OverlappingActors);

	for(AActor* Actor : OverlappingActors)
	{
		TotalMass += Actor->FindComponentByClass<UPrimitiveComponent>()->GetMass();
	}

	return TotalMass;
}

void UOpenDoor::FindAudioComponent()
{
	AudioComponent = GetOwner()->FindComponentByClass<UAudioComponent>();

	if(!AudioComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("%s does not have the AudioComponent attached to it"), *GetOwner()->GetName());
	}
}

void UOpenDoor::FindPressurePlateTriggerVolume()
{
	//The following if statement notifies us if there is a null pointer and which game object it is attached to so that we can see if 
	// we accidentally assigned OpenDoor component to an actor that doesn't need it. This also prevents UE4 from crashing
	if(!PressurePlate) //Same as if(PressurePlate == null)
	{
		UE_LOG(LogTemp, Error, TEXT("%s has the OpenDoor component attached, but not Pressure Plate assigned"), *GetOwner()->GetName());
	}
}