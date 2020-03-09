// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabber.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

#define OUT //Anything under #define is ignored by the compiler. It is merely used for readability purposes. For example, OUT is defined for us to identify out parameters
			//So whenever we use the term OUT beside a parameter, it is for the programmer to know that the marked parameter is an out parameter
			//Think of it as comments, but we can write it together with the code and is ignored by the compiler

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();

	FindPhysicsHandle();
	SetupInputComponent();

}


// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(PhysicsHandle->GrabbedComponent)
	{
		CalculateGrab();
		PhysicsHandle->SetTargetLocation(LineTraceEnd);
	}
}


void UGrabber::Grab()
{
	UE_LOG(LogTemp, Warning, TEXT("Grab is pressed"));

	if(!PhysicsHandle)//To protect from null pointer. If no physics handle found, return early from this function to prevent crash
	{
		return;
	}

	if(PhysicsHandle->GrabbedComponent)
	{
		Release();
		return;
	}
	
	if(!PhysicsHandle->GrabbedComponent)
	{
		FHitResult hitResult = GetFirstPhysicsBodyInReach(); //Will only raycast when the Grab key is pressed instead of every frame, which saves on performance
		UPrimitiveComponent* ComponentToGrab = hitResult.GetComponent();

		//If the hitResult returns a valid actor that can be grabbed
		if(hitResult.GetActor())
		{
			//If raycast hit a grabbable object, then attach the physics handle
			PhysicsHandle->GrabComponentAtLocation(ComponentToGrab, NAME_None, LineTraceEnd);
			#pragma region GrabComponentAtLocation Params Explanation (Click to expand or minimize explanation)
			/*
				1st Param: The component we want the Physics Handle to grab
				2nd Param: The name of the bone (if dealing with skeletal meshes) that we want to grab
				3rd Param: The point where we want to grab and 'attach' the component to. The component will be attached to this point when grabbed
			*/
			#pragma endregion
		}

		return;
	}
}

void UGrabber::Release()
{
	if(!PhysicsHandle)
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Released"));
	PhysicsHandle->ReleaseComponent();
}

void UGrabber::FindPhysicsHandle()
{
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if(!PhysicsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("Actor named : %s does not have the UPhysicsHandleComponent attached to it"), *GetOwner()->GetName());
	}
}

void UGrabber::SetupInputComponent()
{
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	
	if(InputComponent)
	{	
		UE_LOG(LogTemp, Warning, TEXT("Input component is present"));
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab); 
		//InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);

		#pragma region BindAction() Params Explanation (Click to expand or minimize explanation)
		/*
		1st param: The name of the action we want to bind as per the action defined in 
			        the Project Settings. It is CASE SENSITIVE, so make sure the string matches
					the name of the action in the Project Settings. You can bind the same action multiple times under the condition that the input event
					for the each of the bindings are different

		2nd param:	The input event for the action. Determine if the action occurs if the key is pressed,
					key held down, key released, etc...

		3rd param:	this keyword refers to the instance of the Grabber class that is executing			
					the Grabber script
																				
		4th param:  Address reference to the function we want the InputComponent to call when the action is executed
					Requires &, because we are passing the address of the function, not the function call itself.
					Also, we don't use the parentheses beside function name, because we are not calling the function, rather we are BINDING the function to the specific
					action key
		*/
		#pragma endregion
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Input component is missing!!!"));
	}
}

/*GetFirstPhysicsBodyInReach() function's purpose is to create a raycast to check if any actor that is a Physics Body is within the range of the raycast
	This is to determine if an object is within the player's reach to grab or not.
	A FHitResult is returned to determine if an object with Physics Body is overlapped or not.

*/
FHitResult UGrabber::GetFirstPhysicsBodyInReach()
{
	CalculateGrab();

	FHitResult Hit;
	FCollisionQueryParams TraceParams(FName(TEXT("")), false, GetOwner()); 
	
	#pragma region FCollsionQueryParams Params Explanation (Click to expand or minimize explanation)
	 //1st Param: Detect if object is using a tag as specified in the FName param
	 //2nd Param: Determine if Line Trace is using simple or complex tracing. Simple = false, Complex = true
	 //3rd Param: Specify actor that you want the Line Trace to ignore. In this case, we want it 
	 //           to ignore the player pawn (since the trace overlaps it), so we use GetOwner()
	 //			  to tell the line trace to ignore the actor that this Grabbercomponent 
	 //			  is attached to
	
	#pragma endregion
	
	GetWorld()->LineTraceSingleByObjectType
	(
		OUT Hit, //FHitResult is an out parameter
		PlayerViewPointLocation, // a
		LineTraceEnd, // a + b
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody), //Make line trace detect objects that are of type PhysicsBody
		TraceParams
	);

	DrawDebugLine
	(
		GetWorld(),
		PlayerViewPointLocation, // a
		LineTraceEnd, // a + b
		FColor(0.0f, 255.0f, 0.0f),
		false,
		0.0f,
		0,
		5.0f
	);

	AActor* ActorThatIsHit = Hit.GetActor(); //Updates the actor that is hit (if any) on the frame the Grab key is pressed

	//NOTE: Could have used if(Hit.bBlockingHit), but if the object behavior is set to OVERLAP, it will not be detected since bBlockingHit 
	//		only detects objects that BLOCK raycast

	if(ActorThatIsHit != nullptr) //To protect from null pointer
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor that was HIT: %s"), *ActorThatIsHit->GetName());
	}

	return Hit;

}

/*CalculateGrab() function's purpose is to calculate the latest values for the following variables of the Grabber.cpp class:

	1. PlayerViewPointLocation
	2. PlayerViewPointRotation
	3. LineTraceEnd

	The LineTraceEnd value is required at multiple areas of the code at varying times. That is why these calculations are placed in a separate function
	so it can be called anywhere in the code easily.

	So if you want to get the latest values for the 3 variables stated above, call this function in the code
*/
void UGrabber::CalculateGrab()
{
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(OUT PlayerViewPointLocation, OUT PlayerViewPointRotation);

	//FRotators when converted to FVectors will automatically become unit vectors (meaning only having a magnitude/length = 1)
	LineTraceEnd = PlayerViewPointLocation + (PlayerViewPointRotation.Vector() * Reach); //Line Trace End = a + b
}
