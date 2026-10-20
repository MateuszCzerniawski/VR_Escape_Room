#include "RemotePythonComponent.h"
#include "Engine/Engine.h"
#include "Runtime/Engine/Public/LatentActions.h"
#include "CoreMinimal.h"
#include "IPAddress.h"
#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Http.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Async/Async.h"
#include <thread>
#include <atomic>
#include "Engine/Texture2D.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "Modules/ModuleManager.h"
#include "Misc/Base64.h"
#include "Modules/ModuleManager.h"
#include "Misc/Base64.h"
#include "Engine/Texture2D.h"

URemotePythonComponent::URemotePythonComponent() {
    BackgroundSetup=TEXT("http://127.0.0.1:2137/background");
    Classification=TEXT("http://127.0.0.1:2137/classify");
}

FString URemotePythonComponent::extractMessageFromJson(const FString& json,TArray<int32>& asArray,  TArray<FVector4>& rects)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(json);
    if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
    {
        FString message=TEXT("");
        message=JsonObject->HasField(TEXT("message"))?JsonObject->GetStringField(TEXT("message")):message;
        if(JsonObject->HasField(TEXT("horizontals")) && JsonObject->HasField(TEXT("verticals")) && JsonObject->HasField(TEXT("widths")) && JsonObject->HasField(TEXT("heights")))
        {
            TArray<int32> horizontals=StringToArray(JsonObject->GetStringField(TEXT("horizontals")));
            TArray<int32> verticals=StringToArray(JsonObject->GetStringField(TEXT("verticals")));
            TArray<int32> widths=StringToArray(JsonObject->GetStringField(TEXT("widths")));
            TArray<int32> heights=StringToArray(JsonObject->GetStringField(TEXT("heights")));
            int hCount = horizontals.Num();
            if(verticals.Num()==hCount && widths.Num()==hCount && heights.Num()==hCount)
            {
               for(int i=0;i<hCount;i++)
               {
                   rects.Add(FVector4(horizontals[i],verticals[i],widths[i],heights[i]));
               }
            }
            asArray=StringToArray(message);
        }
        return message;
    }
    return TEXT("ERROR: Invalid Json response");
}


FString URemotePythonComponent::TextureToString(UTexture2D* Texture)
{
    Texture = DuplicateObject<UTexture2D>(Texture, nullptr);
    if (!IsValid(Texture)) {
        UE_LOG(LogTemp, Error, TEXT("Texture is not valid"));
        return TEXT("");
    }
    Texture->UpdateResource();
    
    FTexturePlatformData* PlatformData = Texture->GetPlatformData();
    if (!PlatformData || PlatformData->Mips.Num() == 0) {
        UE_LOG(LogTemp, Error, TEXT("Texture has no PlatformData or Mips"));
        return TEXT("");
    }
    
    TArray<FColor> DestPixels{};
    DestPixels.AddDefaulted(PlatformData->SizeX * PlatformData->SizeY);
    
    const auto TextureData = PlatformData->Mips[0].BulkData;
    const auto PixelData = static_cast<const FColor*>(TextureData.LockReadOnly());
    if (!TextureData.IsLocked()) {
        UE_LOG(LogTemp, Error, TEXT("TextureData is not locked or invalid"));
        return TEXT("");
    }
    
    FMemory::Memcpy(DestPixels.GetData(), PixelData, sizeof(FColor) * DestPixels.Num());
    if(TextureData.IsLocked())
    {
        TextureData.Unlock();
    }else
    {
        UE_LOG(LogTemp, Warning, TEXT("TextureData somehow not locked!"));
    }
    
    FString AsString=TEXT("[");
    for (int i = 0; i < DestPixels.Num(); i++)
    {
        const FColor& Pixel = DestPixels[i];
        AsString += FString::Printf(TEXT("%d,%d,%d,"), Pixel.R, Pixel.G, Pixel.B);
    }
    AsString[AsString.Len() - 1] = TEXT(']');
    return AsString;

}

void URemotePythonComponent::sendTexture(UTexture2D* Texture,bool asBackground)
{
    sendImage(TextureToString(Texture),Texture->GetSizeX(),Texture->GetSizeY(),asBackground);
}

void URemotePythonComponent::sendRenderTexture(UTextureRenderTarget2D* Texture, bool asBackground)
{
    int TextureLength = Texture->SizeX * Texture->SizeY;
    if(TextureLength<1)
    {
        UE_LOG(LogTemp, Error, TEXT("Incorrect texture length"));
    }
    TArray<FColor> SurfData;
    FRenderTarget *RenderTarget = Texture->GameThread_GetRenderTargetResource();
    RenderTarget->ReadPixels(SurfData);
    if(SurfData.Num()<1)
    {
        UE_LOG(LogTemp, Error, TEXT("read failed"));
    }
    FString AsString=TEXT("[");
    for (int i = 0; i < TextureLength; i++)
    {
        const FColor& Pixel = SurfData[i];
        AsString += FString::Printf(TEXT("%d,%d,%d,"), Pixel.R, Pixel.G, Pixel.B);
    }
    AsString[AsString.Len() - 1] = TEXT(']');
    if(AsString.Len()<3)
    {
        UE_LOG(LogTemp, Error, TEXT("copying failed"));
    }
    sendImage(AsString,Texture->SizeX,Texture->SizeY,asBackground);
}

void URemotePythonComponent::sendImage(FString asString, int width, int height,bool asBackground)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("image"), asString);
    JsonObject->SetStringField(TEXT("width"), FString::FromInt(width));
    JsonObject->SetStringField(TEXT("height"), FString::FromInt(height));
    
    FString json;
    TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&json);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter); //saving json as string
    
    FHttpModule* HttpModule = &FHttpModule::Get();//preparing http communication
    TSharedRef<IHttpRequest> HttpRequest = HttpModule->CreateRequest();
    HttpRequest->SetURL(asBackground?BackgroundSetup:Classification);
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetContentAsString(json);
    HttpRequest->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        if (bWasSuccessful)
        {
            FString ResponseContent = Response->GetContentAsString();
            TArray<int32> arr;
            TArray<FVector4> rects;
            UE_LOG(LogTemp, Warning, TEXT("Response=%s"),*ResponseContent);
            ResponseContent = extractMessageFromJson(ResponseContent,arr,rects);
            OnHttpResponseReceived.Broadcast(ResponseContent,arr,rects);
        }
        else
        {
            EHttpRequestStatus::Type Status = Request->GetStatus();
            OnHttpRequestFailed.Broadcast(static_cast<int>(Status));
        }
    });
    HttpRequest->ProcessRequest(); //sending json
}


UTexture2D* URemotePythonComponent::RenderTargetTextureToTexture2D(UTextureRenderTarget2D* PassedTexture)
{
    int TextureLength = PassedTexture->SizeX * PassedTexture->SizeY;
    UTexture2D* Pointer = UTexture2D::CreateTransient(PassedTexture->SizeX, PassedTexture->SizeY, PF_B8G8R8A8);
    if (!Pointer || !Pointer->GetPlatformData() || Pointer->GetPlatformData()->Mips.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create texture or platform data is missing."));
        return nullptr;
    }
	
    TArray<FColor> SurfData;
    FRenderTarget *RenderTarget = PassedTexture->GameThread_GetRenderTargetResource();
    RenderTarget->ReadPixels(SurfData);

    uint8* MipData = static_cast<uint8*>(Pointer->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

    //Copy Data
    for (int i = 0; i < TextureLength; i++)
    {
        int MipPointer = i * 4;
        const FColor& Color = SurfData[i];
        MipData[MipPointer] = Color.B;
        MipData[MipPointer + 1] = Color.G;
        MipData[MipPointer + 2] = Color.R;
        MipData[MipPointer + 3] = 255;	//Alpha
    }

    //Unlock and Return data
    Pointer->GetPlatformData()->Mips[0].BulkData.Unlock();
    Pointer->UpdateResource();
	
    return Pointer;
}

TArray<int32> URemotePythonComponent::StringToArray(const FString& stringArray)
{
    FString trimmed = stringArray.TrimStartAndEnd();
    if(trimmed.StartsWith(TEXT("[")))
    {
        trimmed=trimmed.RightChop(1);
    }
    if(trimmed.EndsWith(TEXT("]")))
    {
        trimmed=trimmed.LeftChop(1);
    }
    TArray<FString> strings;
    trimmed.ParseIntoArray(strings, TEXT(","), true);
    TArray<int32> arr;
    for (const FString& Str : strings)
    {
        int32 Number = FCString::Atoi(*Str);
        arr.Add(Number);
    }
    return arr;
}

void URemotePythonComponent::PostLoad()
{
    Super::PostLoad();
    for (UTexture2D* Texture : Images)
    {
        if (IsValid(Texture))
        {
            Texture->bForceMiplevelsToBeResident = true;
            Texture->bGlobalForceMipLevelsToBeResident = true;
            Texture->SetForceMipLevelsToBeResident(30.0f);

            UE_LOG(LogTemp, Log, TEXT("PostLoad: Wymuszono ładowanie tekstury %s"), *Texture->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PostLoad: Nieprawidłowa tekstura"));
        }
    }
}
