#include "CScriptPlatform.hpp"
#include "Collision/CMaterialList.hpp"
#include "Collision/CCollidableOBBTreeGroup.hpp"
#include "TCastTo.hpp"

namespace urde
{

static CMaterialList MakePlatformMaterialList()
{
    CMaterialList ret;
    ret.Add(EMaterialTypes::Solid);
    ret.Add(EMaterialTypes::Immovable);
    ret.Add(EMaterialTypes::Platform);
    ret.Add(EMaterialTypes::Occluder);
    return ret;
}

CScriptPlatform::CScriptPlatform(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, CModelData&& mData, const CActorParameters& actParms,
                                 const zeus::CAABox& aabb, float f1, bool, float f2, bool active,
                                 const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                                 const rstl::optional_object<TLockedToken<CCollidableOBBTreeGroupContainer>>& dcln,
                                 bool, u32, u32)
: CPhysicsActor(uid, active, name, info, xf, std::move(mData), MakePlatformMaterialList(), aabb, SMoverData(15000.f),
                actParms, 0.3f, 0.1f)
, x28c_(hInfo)
, x294_(hInfo)
, x29c_(dVuln)
, x304_treeGroupContainer(dcln)
{
    CActor::SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        CMaterialList(EMaterialTypes::Solid),
        CMaterialList(EMaterialTypes::ThirtyEight, EMaterialTypes::Twenty, EMaterialTypes::Platform)));
    if (x304_treeGroupContainer)
        x314_treeGroup = std::make_unique<CCollidableOBBTreeGroup>(x304_treeGroupContainer->GetObj(), x68_material);
}

void CScriptPlatform::Accept(IVisitor& visitor) { visitor.Visit(this); }

rstl::optional_object<zeus::CAABox> CScriptPlatform::GetTouchBounds() const
{
    if (x314_treeGroup)
        return {x314_treeGroup->CalculateAABox(GetTransform())};

    return {CPhysicsActor::GetBoundingBox()};
}

bool CScriptPlatform::IsRider(TUniqueId id) const
{
    for (const SRiders& rider : x318_riders)
        if (rider.x0_uid == id)
            return true;
    return false;
}
}
