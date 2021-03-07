#pragma once

#include <array>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Collision/CAreaOctTree.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CLineSeg.hpp>
#include <zeus/CVector3d.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CCollisionInfo;
class CCollisionInfoList;
class CMaterialList;

class CAABoxAreaCache {
  friend class CMetroidAreaCollider;
  const zeus::CAABox& x0_aabb;
  const std::array<zeus::CPlane, 6>& x4_planes;
  const CMaterialFilter& x8_filter;
  const CMaterialList& xc_material;
  CCollisionInfoList& x10_collisionList;
  zeus::CVector3f x14_center;
  zeus::CVector3f x20_halfExtent;

public:
  CAABoxAreaCache(const zeus::CAABox& aabb, const std::array<zeus::CPlane, 6>& pl, const CMaterialFilter& filter,
                  const CMaterialList& material, CCollisionInfoList& collisionList);
};

class CBooleanAABoxAreaCache {
  friend class CMetroidAreaCollider;
  const zeus::CAABox& x0_aabb;
  const CMaterialFilter& x4_filter;
  zeus::CVector3f x8_center;
  zeus::CVector3f x14_halfExtent;

public:
  CBooleanAABoxAreaCache(const zeus::CAABox& aabb, const CMaterialFilter& filter);
};

class CSphereAreaCache {
  friend class CMetroidAreaCollider;
  const zeus::CAABox& x0_aabb;
  const zeus::CSphere& x4_sphere;
  const CMaterialFilter& x8_filter;
  const CMaterialList& xc_material;
  CCollisionInfoList& x10_collisionList;

public:
  CSphereAreaCache(const zeus::CAABox& aabb, const zeus::CSphere& sphere, const CMaterialFilter& filter,
                   const CMaterialList& material, CCollisionInfoList& collisionList);
};

class CBooleanSphereAreaCache {
  friend class CMetroidAreaCollider;
  const zeus::CAABox& x0_aabb;
  const zeus::CSphere& x4_sphere;
  const CMaterialFilter& x8_filter;

public:
  CBooleanSphereAreaCache(const zeus::CAABox& aabb, const zeus::CSphere& sphere, const CMaterialFilter& filter);
};

struct SBoxEdge {
  zeus::CLineSeg x0_seg;
  zeus::CVector3d x28_start;
  zeus::CVector3d x40_end;
  zeus::CVector3d x58_delta;
  zeus::CVector3d x70_coDir;
  double x88_dirCoDirDot;
  SBoxEdge(const zeus::CAABox& aabb, int idx, const zeus::CVector3f& dir);
};

class CMovingAABoxComponents {
  friend class CMetroidAreaCollider;
  friend class CCollidableOBBTree;
  rstl::reserved_vector<SBoxEdge, 12> x0_edges;
  rstl::reserved_vector<u32, 8> x6c4_vertIdxs;
  zeus::CAABox x6e8_aabb;

public:
  CMovingAABoxComponents(const zeus::CAABox& aabb, const zeus::CVector3f& dir);
};

class CMetroidAreaCollider {
  friend class CCollidableOBBTree;
  static u32 g_CalledClip;
  static u32 g_RejectedByClip;
  static u32 g_TrianglesProcessed;
  static u32 g_DupTrianglesProcessed;
  static u16 g_DupPrimitiveCheckCount;
  static std::array<u16, 0x2800> g_DupVertexList;
  static std::array<u16, 0x6000> g_DupEdgeList;
  static std::array<u16, 0x4000> g_DupTriangleList;
  static bool AABoxCollisionCheckBoolean_Internal(const CAreaOctTree::Node& node, const CBooleanAABoxAreaCache& cache);
  static bool AABoxCollisionCheck_Internal(const CAreaOctTree::Node& node, const CAABoxAreaCache& cache);

  static bool SphereCollisionCheckBoolean_Internal(const CAreaOctTree::Node& node,
                                                   const CBooleanSphereAreaCache& cache);
  static bool SphereCollisionCheck_Internal(const CAreaOctTree::Node& node, const CSphereAreaCache& cache);

  static bool MovingAABoxCollisionCheck_BoxVertexTri(const CCollisionSurface& surf, const zeus::CAABox& aabb,
                                                     const rstl::reserved_vector<u32, 8>& vertIndices,
                                                     const zeus::CVector3f& dir, double& d, zeus::CVector3f& normal,
                                                     zeus::CVector3f& point);
  static bool MovingAABoxCollisionCheck_TriVertexBox(const zeus::CVector3f& vert, const zeus::CAABox& aabb,
                                                     const zeus::CVector3f& dir, double& d, zeus::CVector3f& normal,
                                                     zeus::CVector3f& point);
  static bool MovingAABoxCollisionCheck_Edge(const zeus::CVector3f& ev0, const zeus::CVector3f& ev1,
                                             const rstl::reserved_vector<SBoxEdge, 12>& edges,
                                             const zeus::CVector3f& dir, double& d, zeus::CVector3f& normal,
                                             zeus::CVector3f& point);

public:
  class COctreeLeafCache {
    friend class CMetroidAreaCollider;
    const CAreaOctTree& x0_octTree;
    rstl::reserved_vector<CAreaOctTree::Node, 64> x4_nodeCache;
    bool x908_24_overflow : 1 = false;

  public:
    explicit COctreeLeafCache(const CAreaOctTree& octTree);
    void AddLeaf(const CAreaOctTree::Node& node);
    u32 GetNumLeaves() const { return x4_nodeCache.size(); }
    bool HasCacheOverflowed() const { return x908_24_overflow; }
    const CAreaOctTree& GetOctTree() const { return x0_octTree; }
    rstl::reserved_vector<CAreaOctTree::Node, 64>::const_iterator begin() const { return x4_nodeCache.begin(); }
    rstl::reserved_vector<CAreaOctTree::Node, 64>::const_iterator end() const { return x4_nodeCache.end(); }
  };
  static void BuildOctreeLeafCache(const CAreaOctTree::Node& root, const zeus::CAABox& aabb,
                                   CMetroidAreaCollider::COctreeLeafCache& cache);
  static bool ConvexPolyCollision(const std::array<zeus::CPlane, 6>& planes,
                                  const std::array<zeus::CVector3f, 3>& verts, zeus::CAABox& aabb);

  static bool AABoxCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                const CMaterialFilter& filter);
  static bool AABoxCollisionCheckBoolean(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                         const CMaterialFilter& filter);

  static bool SphereCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                 const zeus::CSphere& sphere, const CMaterialFilter& filter);
  static bool SphereCollisionCheckBoolean(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                          const zeus::CSphere& sphere, const CMaterialFilter& filter);

  static bool AABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                         const CMaterialFilter& filter, const CMaterialList& matList,
                                         CCollisionInfoList& list);
  static bool AABoxCollisionCheck(const CAreaOctTree& octTree, const zeus::CAABox& aabb, const CMaterialFilter& filter,
                                  const CMaterialList& matList, CCollisionInfoList& list);

  static bool SphereCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                          const zeus::CSphere& sphere, const CMaterialList& matList,
                                          const CMaterialFilter& filter, CCollisionInfoList& list);
  static bool SphereCollisionCheck(const CAreaOctTree& octTree, const zeus::CAABox& aabb, const zeus::CSphere& sphere,
                                   const CMaterialList& matList, const CMaterialFilter& filter,
                                   CCollisionInfoList& list);

  static bool MovingAABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                               const CMaterialFilter& filter, const CMaterialList& matList,
                                               const zeus::CVector3f& dir, float mag, CCollisionInfo& infoOut,
                                               double& dOut);
  static bool MovingSphereCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                const zeus::CSphere& sphere, const CMaterialFilter& filter,
                                                const CMaterialList& matList, const zeus::CVector3f& dir, float mag,
                                                CCollisionInfo& infoOut, double& dOut);
  static void ResetInternalCounters();
  static std::array<u16, 0x4000> GetTriangleList() { return g_DupTriangleList; }
};

class CAreaCollisionCache {
  zeus::CAABox x0_aabb;
  rstl::reserved_vector<CMetroidAreaCollider::COctreeLeafCache, 3> x18_leafCaches;
  bool x1b40_24_leafOverflow : 1 = false;
  bool x1b40_25_cacheOverflow : 1 = false;

public:
  explicit CAreaCollisionCache(const zeus::CAABox& aabb) : x0_aabb(aabb) {}
  void ClearCache();
  const zeus::CAABox& GetCacheBounds() const { return x0_aabb; }
  void SetCacheBounds(const zeus::CAABox& aabb) { x0_aabb = aabb; }
  void AddOctreeLeafCache(const CMetroidAreaCollider::COctreeLeafCache& leafCache);
  u32 GetNumCaches() const { return x18_leafCaches.size(); }
  const CMetroidAreaCollider::COctreeLeafCache& GetOctreeLeafCache(int idx) { return x18_leafCaches[idx]; }
  bool HasCacheOverflowed() const { return x1b40_24_leafOverflow; }
  rstl::reserved_vector<CMetroidAreaCollider::COctreeLeafCache, 3>::const_iterator begin() const
  { return x18_leafCaches.begin(); }
  rstl::reserved_vector<CMetroidAreaCollider::COctreeLeafCache, 3>::const_iterator end() const
  { return x18_leafCaches.end(); }
};

} // namespace metaforce
