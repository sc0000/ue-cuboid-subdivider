// Copyright (c) 2024 Sebastian Cyliax


#include "CuboidSubdividerBlueprintLib.h"

#include "DebugHeader.h"
#include "Editor.h"
#include "EditorAssetLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "StaticMeshEditorSubsystem.h"
#include "StaticMeshEditorSubsystemHelpers.h"
#include "Subsystems/EditorActorSubsystem.h"

UEditorActorSubsystem* UCuboidSubdividerBlueprintLib::EditorActorSubsystem = nullptr;
UStaticMeshEditorSubsystem* UCuboidSubdividerBlueprintLib::StaticMeshEditorSubsystem = nullptr;

void UCuboidSubdividerBlueprintLib::SubdivideSelectedCuboids(const float MaxSubdivsSideLength)
{
	if (!GetEditorActorSubsystem() || !GetStaticMeshEditorSubsystem()) return;

	for (AActor* SelectedActor : GetSelectedLevelActors())
	{
		if (!SelectedActor)
		{
			DebugHeader::ShowMsgDialog(OK, TEXT("Selected Actor not valid"));
			continue;
		}
		
		FVector SelectedActorScale = SelectedActor->GetActorScale();
		FQuat SelectedActorQuat = SelectedActor->GetActorQuat();
		UStaticMeshComponent* SelectedActorMeshComponent = SelectedActor->GetComponentByClass<UStaticMeshComponent>();
		UStaticMesh* SelectedActorMesh = SelectedActorMeshComponent->GetStaticMesh();

		// Support for more than one material needed?
		UMaterialInterface* SelectedActorMaterial = SelectedActorMeshComponent->GetMaterial(0);

		if (!SelectedActorMaterial)
			DebugHeader::ShowNotifyInfo(TEXT("No material..."));

		if (!SelectedActorMeshComponent || !SelectedActorMesh || !IsCuboid(SelectedActorMeshComponent)) continue;

		FVector Min;
		FVector Max;

		SelectedActorMeshComponent->GetLocalBounds(Min, Max);
		FVector SelectedActorScaledBounds = Max * SelectedActorScale;

		TArray<double> LongestAxesBounds = GetLongestAxesScaledBounds(SelectedActorScaledBounds);

		if (LongestAxesBounds[0] < MaxSubdivsSideLength * 2.0 &&
				LongestAxesBounds[1] < MaxSubdivsSideLength * 2.0)
			continue;

		int32 SubdivsLongestAxis = FMath::CeilToInt(LongestAxesBounds[0] / MaxSubdivsSideLength);
		int32 SubdivsSecondLongestAxis = FMath::CeilToInt(LongestAxesBounds[1] / MaxSubdivsSideLength);

		int32 SubdivsX;
		int32 SubdivsY;
		int32 SubdivsZ;

		if (SelectedActorScaledBounds.X == LongestAxesBounds[0])
			SubdivsX = SubdivsLongestAxis;

		else if (SelectedActorScaledBounds.X == LongestAxesBounds[1])
			SubdivsX = SubdivsSecondLongestAxis;

		else SubdivsX = 1;

		if (SelectedActorScaledBounds.Y == LongestAxesBounds[0])
			SubdivsY = SubdivsLongestAxis;

		else if (SelectedActorScaledBounds.Y == LongestAxesBounds[1])
			SubdivsY = SubdivsSecondLongestAxis;

		else SubdivsY = 1;

		if (SelectedActorScaledBounds.Z == LongestAxesBounds[0])
			SubdivsZ = SubdivsLongestAxis;

		else if (SelectedActorScaledBounds.Z == LongestAxesBounds[1])
			SubdivsZ = SubdivsSecondLongestAxis;

		else SubdivsZ = 1;

		FVector Subdivisions = FVector(SubdivsX,
								  SubdivsY,
								  SubdivsZ);

		// Safeguard for input 1
		double LocalOffsetX = SubdivsX > 1 ?
			SelectedActorScaledBounds.X / Subdivisions.X : 0.0;
		double LocalOffsetY = SubdivsY > 1 ?
			SelectedActorScaledBounds.Y / Subdivisions.Y : 0.0;
		double LocalOffsetZ = SubdivsZ > 1 ?
			SelectedActorScaledBounds.Z / Subdivisions.Z : 0.0;

		FVector LocalOffset = FVector(LocalOffsetX * (Subdivisions.X - 1.0),
									LocalOffsetY * (Subdivisions.Y - 1.0),
									LocalOffsetZ * (Subdivisions.Z - 1.0));

		FVector InitialLocalOffset = LocalOffset;

		// Safeguard for input 0
		double SubdivisionScaleX = SubdivsX > 1 ? Subdivisions.X : 1.0;
		double SubdivisionScaleY = SubdivsY > 1 ? Subdivisions.Y : 1.0;
		double SubdivisionScaleZ = SubdivsZ > 1 ? Subdivisions.Z : 1.0;

		FVector SubdivisionScale = SelectedActorScale / 
			FVector(SubdivisionScaleX, SubdivisionScaleY, SubdivisionScaleZ);

		FMergeStaticMeshActorsOptions MergeOptions;
		
		MergeOptions.bDestroySourceActors = true;
		MergeOptions.bSpawnMergedActor = true;
		MergeOptions.NewActorLabel = SelectedActor->GetActorLabel() + "_Subdivided";

		FMeshMergingSettings MergingSettings;
		MergingSettings.bBakeVertexDataToMesh = true;
		MergingSettings.bMergePhysicsData = true;

		MergeOptions.MeshMergingSettings = MergingSettings;

		MergeOptions.BasePackageName = 
			"/Game/SubdividedCuboids/SM_SubdividedCuboid_" + FString::FromInt(GetAssetNumber());

		TArray<AStaticMeshActor*> ActorsToMerge;
		AStaticMeshActor* MergedActor;

		for (uint8 IterX = 0; IterX < SubdivsX; IterX++)
		{
			for (uint8 IterY = 0; IterY < SubdivsY; IterY++)
			{
				for (uint8 IterZ = 0; IterZ < SubdivsZ; IterZ++)
				{
					FTransform WorldTransform = FTransform(FRotator(0.0, 0.0, 0.0), -LocalOffset, SubdivisionScale);
					AActor* SpawnedActor = SelectedActor->GetWorld()->SpawnActor(AStaticMeshActor::StaticClass(), &WorldTransform);
					AStaticMeshActor* SpawnedMeshActor = Cast<AStaticMeshActor>(SpawnedActor);

					if (SpawnedMeshActor && SelectedActorMesh)
					{
						UStaticMeshComponent* SpawnedMeshComponent = SpawnedMeshActor->GetStaticMeshComponent();
						SpawnedMeshComponent->SetStaticMesh(SelectedActorMesh);
					}

					ActorsToMerge.Add(SpawnedMeshActor);
					LocalOffset.Z += -2.0 * LocalOffsetZ;
				}

				LocalOffset.Z = InitialLocalOffset.Z;
				LocalOffset.Y += -2.0 * LocalOffsetY;
			}

			LocalOffset.Y = InitialLocalOffset.Y;
			LocalOffset.X += -2.0 * LocalOffsetX;
		}

		EditorActorSubsystem->SetActorSelectionState(SelectedActor, false);

		StaticMeshEditorSubsystem->MergeStaticMeshActors(ActorsToMerge, MergeOptions, MergedActor);

		MergedActor->SetPivotOffset(InitialLocalOffset);

		FVector MergedActorOffset = SelectedActorQuat.RotateVector(-InitialLocalOffset);
		
		MergedActor->SetActorRotation(SelectedActorQuat);
		MergedActor->SetActorLocation(SelectedActor->GetActorLocation() + MergedActorOffset);
		
		EditorActorSubsystem->SetActorSelectionState(MergedActor, true);

		MergedActor->GetStaticMeshComponent()->GetStaticMesh()->bAllowCPUAccess = true;

		if (SelectedActorMaterial)
			MergedActor->GetStaticMeshComponent()->SetMaterial(0, SelectedActorMaterial);

		MergedActor->GetWorld()->DestroyActor(SelectedActor);
	}
}

TArray<AActor*> UCuboidSubdividerBlueprintLib::GetSelectedLevelActors()
{
	TArray<AActor*> SelectedLevelActors;
	
	if (!GetEditorActorSubsystem())
		return SelectedLevelActors;
	
	SelectedLevelActors = EditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedLevelActors.Num() == 0)
		DebugHeader::ShowMsgDialog(OK, TEXT("No Actor selected"));

	return SelectedLevelActors;
}

bool UCuboidSubdividerBlueprintLib::IsCuboid(const UStaticMeshComponent* StaticMeshComponent)
{
	const int32 NumVertices = 
		StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[0].GetNumVertices();

	return NumVertices == 24;
}

TArray<double> UCuboidSubdividerBlueprintLib::GetLongestAxesScaledBounds(const FVector& ScaledBounds)
{
	TArray<double> BoundsArray = { ScaledBounds.X, ScaledBounds.Y, ScaledBounds.Z };
	BoundsArray.Sort(TGreater<double>());

	return BoundsArray;
}

int32 UCuboidSubdividerBlueprintLib::GetAssetNumber()
{
	TArray<FString> AssetPaths = UEditorAssetLibrary::ListAssets("/Game/SubdividedCuboids");

	FString LogString;
	TArray<int32> UsedNumbers;

	for (const FString& AP : AssetPaths)
	{
		const FAssetData AssetData = UEditorAssetLibrary::FindAssetData(AP);

		FString AssetName = AssetData.AssetName.ToString();
		FString Suffix = AssetName.RightChop(AssetName.Len() - 3);

		int32 Number = 0;

		if (int32 TripleDigit = FCString::Atoi(*Suffix))
			Number = TripleDigit;

		else if (int32 DoubleDigit = FCString::Atoi(*Suffix.RightChop(1)))
			Number = DoubleDigit;

		else if (int32 SingleDigit = FCString::Atoi(&Suffix[2]))
			Number = SingleDigit;

		UsedNumbers.Add(Number);
	}

	int32 AssetNumber = 0;

	while (UsedNumbers.Contains(AssetNumber))
		++AssetNumber;

	return AssetNumber;
}

bool UCuboidSubdividerBlueprintLib::GetStaticMeshEditorSubsystem()
{
	if (!StaticMeshEditorSubsystem)
		StaticMeshEditorSubsystem = GEditor->GetEditorSubsystem<UStaticMeshEditorSubsystem>();

	return StaticMeshEditorSubsystem != nullptr;
}

bool UCuboidSubdividerBlueprintLib::GetEditorActorSubsystem()
{
	if (!EditorActorSubsystem)
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();

	return EditorActorSubsystem != nullptr;
}