
setmetatable(_ENV, { __index = EngineTypes })

-- Template은 AActor라는 가정 하에 작동.

local ReturnTable = {} -- Return용 table. cpp에서 Table 단위로 객체 관리.

local FVector = EngineTypes.FVector -- EngineTypes로 등록된 FVector local로 선언.

local LuaUIBind = EngineTypes.LuaUIBind
local RectTransform = EngineTypes.RectTransform
local FString = EngineTypes.FString
local AnchorDirection = EngineTypes.AnchorDirection
local FLinearColor = EngineTypes.FLinearColor

-- BeginPlay: Actor가 처음 활성화될 때 호출
function ReturnTable:BeginPlay()

    print("BeginPlay ", self.Name) -- Table에 등록해 준 Name 출력.

    local uiName = FString.new("MyDynamicTextUI")
    local textContent = FString.new("Hello from Lua with Dynamic Rect!")

    local posX = -150.0
    local posY = 50.0
    local width = 200.0
    local height = 80.0
    local anchor = AnchorDirection.TopCenter -- Enum 값 사용

    local myRect = RectTransform.new(posX, posY, width, height, anchor)

    local SortOrder = 20
    local fontName = FString.new("Default")
    local fontSize = 50
    local fontColor = FLinearColor.new(1.0, 0.0, 1.0, 1.0)

    LuaUIBind.CreateText(uiName,  myRect, SortOrder, textContent, fontName, fontSize, fontColor)

    local imageName = FString.new("MyImageUI")
    local textureName = FString.new("ExplosionColor")

    local TexturePosX = -450.0
    local TexturePosY = 50.0
    local TextureWidth = 300.0
    local TextureHeight = 80.0
    local TextureAnchor = AnchorDirection.MiddleCenter

    local TextureRect = RectTransform.new(TexturePosX, TexturePosY, TextureWidth, TextureHeight, TextureAnchor)

    local TextureSortOrder = 10
    local TextureColor = FLinearColor.new(1.0, 1.0, 1.0, 0.1)

    LuaUIBind.CreateImage(imageName, TextureRect, TextureSortOrder, textureName, TextureColor)

end

-- Tick: 매 프레임마다 호출
function ReturnTable:Tick(DeltaTime)
    
    -- 기본적으로 Table로 등록된 변수는 self, Class usertype으로 선언된 변수는 self.this로 불러오도록 설정됨.
    -- sol::property로 등록된 변수는 변수 사용으로 getter, setter 등록이 되어 .(dot) 으로 접근가능하고
    -- 바로 등록된 경우에는 PropertyName() 과 같이 함수 형태로 호출되어야 함.
    local this = self.this
    -- this.ActorLocation = this.ActorLocation + FVector(1.0, 0.0, 0.0) * DeltaTime -- X 방향으로 이동하도록 선언.

end

-- EndPlay: Actor가 파괴되거나 레벨이 전환될 때 호출
function ReturnTable:EndPlay(EndPlayReason)
    -- print("[Lua] EndPlay called. Reason:", EndPlayReason) -- EndPlayReason Type 등록된 이후 사용 가능.
    print("EndPlay")

end

function ReturnTable:Attack(AttackDamage)
    self.GetDamate(AttackDamage)

end

return ReturnTable
