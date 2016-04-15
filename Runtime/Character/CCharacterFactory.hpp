#ifndef __URDE_CCHARACTERFACTORY_HPP__
#define __URDE_CCHARACTERFACTORY_HPP__

#include "IFactory.hpp"
#include "IObjFactory.hpp"
#include "CToken.hpp"
#include "CSimplePool.hpp"
#include "CAnimationSet.hpp"

namespace urde
{
class CSimplePool;
class CAnimCharacterSet;
class CCharacterInfo;
class CCharLayoutInfo;
class CAdditiveAnimationInfo;
class CTransitionDatabaseGame;
class CAnimationManager;
class CTransitionManager;
class CAllFormatsAnimSource;
class CAnimData;

class CCharacterFactory : public IObjFactory
{
public:
    class CDummyFactory : public IFactory
    {
    public:
        CFactoryFnReturn Build(const SObjectTag&, const CVParamTransfer&);
        void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**);
        void CancelBuild(const SObjectTag&);
        bool CanBuild(const SObjectTag&);
        const SObjectTag* GetResourceIdByName(const char*) const;
        FourCC GetResourceTypeById(ResId id) const;
    };

private:
    std::vector<CCharacterInfo> x4_charInfoDB;
    std::vector<TLockedToken<CCharLayoutInfo>> x14_charLayoutInfoDB;
    std::shared_ptr<CAnimSysContext> x24_sysContext;
    std::shared_ptr<CAnimationManager> x28_animMgr;
    std::shared_ptr<CTransitionManager> x2c_transMgr;
    std::vector<TCachedToken<CAllFormatsAnimSource>> x30_animSourceDB;
    std::vector<std::pair<u32, CAdditiveAnimationInfo>> x40_additiveInfo;
    CAdditiveAnimationInfo x50_defaultAdditiveInfo;
    std::vector<std::pair<ResId, ResId>> x58_animResources;
    ResId x68_selfId;
    CDummyFactory x6c_dummyFactory;
    CSimplePool x70_cacheResPool;

    static std::vector<CCharacterInfo> GetCharacterInfoDB(const CAnimCharacterSet& ancs);
    static std::vector<TLockedToken<CCharLayoutInfo>>
    GetCharLayoutInfoDB(CSimplePool& store,
                        const std::vector<CCharacterInfo>& chars);

public:
    CCharacterFactory(CSimplePool& store, const CAnimCharacterSet& ancs, ResId);

    std::unique_ptr<CAnimData> CreateCharacter(int charIdx, bool loop,
                                               const TLockedToken<CCharacterFactory>& factory,
                                               int defaultAnim) const;
    ResId GetEventResourceIdForAnimResourceId(ResId animId) const;
};

}

#endif // __URDE_CCHARACTERFACTORY_HPP__
