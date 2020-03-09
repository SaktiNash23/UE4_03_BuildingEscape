// Fill out your copyright notice in the Description page of Project Settings.
#include "WorldPosition.h"
#include "GameFramework/Actor.h" //Header required to use GetOwner() function

// Sets default values for this component's properties
UWorldPosition::UWorldPosition()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWorldPosition::BeginPlay()
{
	Super::BeginPlay();

	// FString Log = TEXT("Hello");
	// FString* PtrLog = &Log;

	// Log.Len();
	// (*PtrLog).Len(); //Not recommended to use this to access properties of a pointer
	// PtrLog->Len(); //Recommended to use -> when accessing properties of a pointer

	// UE_LOG(LogTemp, Warning, TEXT("%s"), **PtrLog);  1st * is to dereference pointer, 2nd * is for converting string values to TCHAR array for UE_LOG to recognise it
	
	FString OwnerName = GetOwner()->GetName();
	UE_LOG(LogTemp, Warning, TEXT("The name of the object this component is attached to is : %s"), *OwnerName); //Note: * is required when using an FString as an argument
	

	FString ObjectPosition = GetOwner()->GetActorLocation().ToCompactString(); //ToCompactString returns the FVector in a more readable FString format
	UE_LOG(LogTemp, Warning, TEXT("%s is at position %s"), *OwnerName, *ObjectPosition); //Note: * is required when using an FString as an argument because UE_LOG is expecting a TCHAR array argument. So using * converts a string to a TCHAR array
}


// Called every frame
void UWorldPosition::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

