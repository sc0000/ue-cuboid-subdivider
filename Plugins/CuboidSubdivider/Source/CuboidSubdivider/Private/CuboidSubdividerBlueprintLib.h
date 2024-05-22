// Copyright (c) 2024 Sebastian Cyliax

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CuboidSubdividerBlueprintLib.generated.h"

/**
 * 
 */
UCLASS()
class UCuboidSubdividerBlueprintLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Vertex Color Painting Interface")
	static void SubdivideSelectedCuboids(const float MaxSubdivsSideLength);

private:
	static TArray<AActor*> GetSelectedLevelActors();
	static bool IsCuboid(const UStaticMeshComponent* StaticMeshComponent);
	static TArray<double> GetLongestAxesScaledBounds(const FVector& ScaledBounds);
	static int32 GetAssetNumber();

	static bool GetStaticMeshEditorSubsystem();
	static bool GetEditorActorSubsystem();
	
	static class UEditorActorSubsystem* EditorActorSubsystem;
	static class UStaticMeshEditorSubsystem* StaticMeshEditorSubsystem;
};
