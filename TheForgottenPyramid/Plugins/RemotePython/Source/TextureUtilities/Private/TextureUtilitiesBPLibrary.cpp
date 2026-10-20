// Copyright Epic Games, Inc. All Rights Reserved.

#include "TextureUtilitiesBPLibrary.h"
#include "TextureUtilities.h"
#include "Engine/TextureRenderTarget2D.h"

UTextureUtilitiesBPLibrary::UTextureUtilitiesBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

float UTextureUtilitiesBPLibrary::GetTextureCoverage(UTexture2D* Texture)
{
	Texture = DuplicateObject<UTexture2D>(Texture, nullptr);
	if (!IsValid(Texture)) {
		return -1;
	}
	Texture->UpdateResource();
	FTexturePlatformData* PlatformData = Texture->GetPlatformData();
	if (!PlatformData || PlatformData->Mips.Num() == 0) {
		return -1;
	}
	TArray<FColor> DestPixels{};
	DestPixels.AddDefaulted(PlatformData->SizeX * PlatformData->SizeY);
	const auto TextureData = PlatformData->Mips[0].BulkData;
	const auto PixelData = static_cast<const FColor*>(TextureData.LockReadOnly());
	if (!TextureData.IsLocked()) {
		return -1;
	}
	FMemory::Memcpy(DestPixels.GetData(), PixelData, sizeof(FColor) * DestPixels.Num());
	if(TextureData.IsLocked())
	{
		TextureData.Unlock();
	}else
	{
		return -1;
	}
	int count=0;
	for (int i = 0; i < DestPixels.Num(); i++)
	{
		const FColor& Pixel = DestPixels[i];
		if(Pixel.R!=0 || Pixel.G!=0 || Pixel.B!=0)
		{
			count++;
		}
	}
	int size = Texture->GetSizeX()*Texture->GetSizeY();
	return count/(size+0.0f);
}

float UTextureUtilitiesBPLibrary::GetRenderTextureCoverage(UTextureRenderTarget2D* Texture)
{
	int TextureLength = Texture->SizeX * Texture->SizeY;
	if(TextureLength<1)
	{
		return -1;
	}
	TArray<FColor> SurfData;
	FRenderTarget *RenderTarget = Texture->GameThread_GetRenderTargetResource();
	RenderTarget->ReadPixels(SurfData);
	if(SurfData.Num()<1)
	{
		return -1;
	}
	int count=0;
	for (int i = 0; i < TextureLength; i++)
	{
		const FColor& Pixel = SurfData[i];
		if(Pixel.R!=0 || Pixel.G!=0 || Pixel.B!=0)
		{
			count++;
		}
	}
	return count/(TextureLength+0.0f);
}

