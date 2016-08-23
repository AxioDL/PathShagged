#include "CGameState.hpp"
#include "IOStreams.hpp"
#include "zeus/Math.hpp"

namespace urde
{

CGameState::CGameState()
{
    x98_playerState.reset(new CPlayerState());
    x9c_transManager.reset(new CWorldTransManager());
    x228_25_deferPowerupInit = true;
}

CGameState::CGameState(CBitStreamReader& stream)
{
    x228_25_deferPowerupInit = true;

    for (u32 i = 0; i < 128; i++)
        stream.ReadEncoded(8);
    u32 tmp = stream.ReadEncoded(32);
    double val1 = *(reinterpret_cast<float*>(&tmp));
    bool val2 = stream.ReadEncoded(1);
    stream.ReadEncoded(1);
    tmp = stream.ReadEncoded(32);
    double val3 = *(reinterpret_cast<float*>(&tmp));
    tmp = stream.ReadEncoded(32);
    double val4 = *(reinterpret_cast<float*>(&tmp));
    tmp = stream.ReadEncoded(32);
    double val5 = *(reinterpret_cast<float*>(&tmp));

    CPlayerState tmpPlayer(stream);
    float currentHealth = tmpPlayer.GetHealthInfo().GetHP();
}

void CGameState::SetCurrentWorldId(unsigned int id)
{
}

void CGameState::SetTotalPlayTime(float time)
{
    xa0_playTime = zeus::clamp<double>(0.0, time, 359999.0);
}

CWorldState& CGameState::StateForWorld(ResId mlvlId)
{
    auto it = x88_worldStates.begin();
    for (; it != x88_worldStates.end() ; ++it)
    {
        if (it->GetWorldAssetId() == mlvlId)
            break;
    }

    if (it == x88_worldStates.end())
    {
        x88_worldStates.emplace_back(mlvlId);
        return x88_worldStates.back();
    }
    return *it;
}

}
