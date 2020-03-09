// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerVolume.h"
#include "OpenDoor.generated.h" //The .generated #include statement MUST always be the beneath all the other #include statements


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BUILDINGESCAPE_API UOpenDoor : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOpenDoor();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void OpenDoor(float DeltaTime);
	void CloseDoor(float DeltaTime);
	float TotalMassOfActors();
	void FindAudioComponent();
	void FindPressurePlateTriggerVolume();

private:
	float InitialYaw = 0.0f;
	float DoorCloseAngle = 0.0f;
	float DoorLastOpen = 0.0f;
	bool bHasDoorBeenOpened = false;
	bool bHasDoorBeenClosed = true;

	UPROPERTY(EditAnywhere)
	float MinMassToOpenDoor = 0.0f;

	UPROPERTY(EditAnywhere) //Exposes parameter to the editor for easy editing
	float DoorOpenAngle = 90.0f; //Designer : The angle at which you want the door to open

	UPROPERTY(EditAnywhere)
	float DoorCloseDelay = 2.0f; //Designer: How long (in seconds) you want the door to remain open until it begins to close

	UPROPERTY(EditAnywhere)
	float DoorOpenSpeed = 2.0f; //Designer: How fast or slow you want the door to open

	UPROPERTY(EditAnywhere)
	float DoorCloseSpeed = 2.5f; //Designer: How fast or slow you want the door to close

	UPROPERTY(EditAnywhere)
	ATriggerVolume* PressurePlate = nullptr; //Pointer which references the Trigger Volume object in the game world. Assign the trigger volume in the editor

	AActor* ActorThatCanOpenDoor = nullptr; //Although player is a Pawn object, since Pawn is derived from Actor class, we can use AActor as the data type. So both APawns & AActors
								  			// will work 	
	UPROPERTY()
	UAudioComponent* AudioComponent = nullptr;
	
};
