#pragma once

#include <memory>
#include <vector>

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/Collision/COBBTree.hpp"
#include "Runtime/Collision/CCollisionPrimitive.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CCollidableOBBTreeGroupContainer {
  friend class CCollidableOBBTreeGroup;
  std::vector<std::unique_ptr<COBBTree>> x0_trees;
  std::vector<zeus::CAABox> x10_aabbs;
  zeus::CAABox x20_aabox;

public:
  explicit CCollidableOBBTreeGroupContainer(CInputStream& in);
  CCollidableOBBTreeGroupContainer(const zeus::CVector3f&, const zeus::CVector3f&);
  u32 NumTrees() const { return x0_trees.size(); }
};

class CCollidableOBBTreeGroup : public CCollisionPrimitive {
  static inline u32 sTableIndex = UINT32_MAX;
  const CCollidableOBBTreeGroupContainer* x10_container;

public:
  CCollidableOBBTreeGroup(const CCollidableOBBTreeGroupContainer*, const CMaterialList&);
  ~CCollidableOBBTreeGroup() override = default;

  void ResetTestStats() const;
  u32 GetTableIndex() const override;
  zeus::CAABox CalculateAABox(const zeus::CTransform&) const override;
  zeus::CAABox CalculateLocalAABox() const override;
  FourCC GetPrimType() const override;
  CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const override;
  COBBTree const* GetOBBTreeAABox(int index) const { return x10_container->x0_trees[index].get(); }
  CCollidableOBBTreeGroupContainer const* GetContainer() const { return x10_container; }

  static const Type& GetType();
  static void SetStaticTableIndex(u32 index);
  /* Sphere Collide */
  static bool SphereCollide(const CInternalCollisionStructure&, CCollisionInfoList&);
  static bool SphereCollideBoolean(const CInternalCollisionStructure&);
  static bool CollideMovingSphere(const CInternalCollisionStructure&, const zeus::CVector3f&, double&, CCollisionInfo&);
  /* AABox Collide */
  static bool AABoxCollide(const CInternalCollisionStructure&, CCollisionInfoList&);
  static bool AABoxCollideBoolean(const CInternalCollisionStructure&);
  static bool CollideMovingAABox(const CInternalCollisionStructure&, const zeus::CVector3f&, double&, CCollisionInfo&);
};

CFactoryFnReturn FCollidableOBBTreeGroupFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                                CObjectReference* selfRef);
} // namespace metaforce
