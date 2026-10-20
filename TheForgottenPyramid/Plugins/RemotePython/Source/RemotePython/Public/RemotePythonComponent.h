#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RemotePythonComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHttpResponseReceived, const FString&, ResponseContent,const TArray<int32>&,asArray,const TArray<FVector4>&,rects);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHttpRequestFailed,const int,Status);

UCLASS(BlueprintType, Blueprintable, ClassGroup = Computing, meta = (BlueprintSpawnableComponent))
class REMOTEPYTHON_API URemotePythonComponent : public UActorComponent
{
    GENERATED_BODY()
public:	
    URemotePythonComponent();

    UPROPERTY(BlueprintAssignable, Category = "Connection")
    FOnHttpResponseReceived OnHttpResponseReceived;

    UPROPERTY(BlueprintAssignable, Category = "Connection")
    FOnHttpRequestFailed OnHttpRequestFailed;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Connection")
    FString BackgroundSetup;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Connection")
    FString Classification;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Content")
    TArray<UTexture2D*> Images;
    
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void sendTexture(UTexture2D* Texture,bool asBackground);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void sendRenderTexture(UTextureRenderTarget2D* Texture,bool asBackground);
    

    UFUNCTION(BlueprintCallable, Category = "Content")
    UTexture2D* RenderTargetTextureToTexture2D(UTextureRenderTarget2D* PassedTexture);
    
protected:
    virtual void PostLoad() override;

private:
    FString extractMessageFromJson(const FString& json,TArray<int32>& asArray, TArray<FVector4>& rects);
    FString TextureToString(UTexture2D* Texture);
    TArray<int32> StringToArray(const FString& stringArray);
    void sendImage(FString asString,int width, int height,bool asBackground);
};
