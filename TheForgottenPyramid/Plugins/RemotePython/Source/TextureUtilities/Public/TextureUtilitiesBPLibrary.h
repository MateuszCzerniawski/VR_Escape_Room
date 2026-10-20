// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "TextureUtilitiesBPLibrary.generated.h"

UCLASS()
class UTextureUtilitiesBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Texture Coverage", Keywords = "TextureUtilities get texture coverage"), Category = "TextureUtilities")
	static float GetTextureCoverage(UTexture2D* Texture);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Render Texture Coverage", Keywords = "TextureUtilities get render texture coverage"), Category = "TextureUtilities")
	static float GetRenderTextureCoverage(UTextureRenderTarget2D* Texture);

};
