#include "GameGlobalObjects.hpp"

namespace urde
{

class CMemoryCardSys* g_MemoryCardSys = nullptr;
class IFactory* g_ResFactory = nullptr;
class CSimplePool* g_SimplePool = nullptr;
class CCharacterFactoryBuilder* g_CharFactoryBuilder = nullptr;
class CAiFuncMap* g_AiFuncMap = nullptr;
class CGameState* g_GameState = nullptr;
class CInGameTweakManagerBase* g_TweakManager = nullptr;
class CBooRenderer* g_Renderer = nullptr;

DataSpec::ITweakGame*   g_tweakGame = nullptr;
DataSpec::ITweakPlayer* g_tweakPlayer = nullptr;
DataSpec::ITweakPlayerControl* g_tweakPlayerControl = nullptr;

}
