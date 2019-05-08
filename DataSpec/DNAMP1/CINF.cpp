#include "CINF.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMP1 {

atUint32 CINF::getInternalBoneIdxFromId(atUint32 id) const {
  atUint32 idx = 0;
  for (const Bone& b : bones) {
    if (b.id == id)
      return idx;
    ++idx;
  }
  return -1;
}

atUint32 CINF::getBoneIdxFromId(atUint32 id) const {
  atUint32 idx = 0;
  for (atUint32 bid : boneIds) {
    if (bid == id)
      return idx;
    ++idx;
  }
  return 0;
}

const std::string* CINF::getBoneNameFromId(atUint32 id) const {
  for (const Name& name : names)
    if (id == name.boneId)
      return &name.name;
  return nullptr;
}

void CINF::sendVertexGroupsToBlender(hecl::blender::PyOutStream& os) const {
  for (atUint32 bid : boneIds) {
    for (const Name& name : names) {
      if (name.boneId == bid) {
        os.format("obj.vertex_groups.new(name='%s')\n", name.name.c_str());
        break;
      }
    }
  }
}

void CINF::sendCINFToBlender(hecl::blender::PyOutStream& os, const UniqueID32& cinfId) const {
  DNAANIM::RigInverter<CINF> inverter(*this);

  os.format(
      "arm = bpy.data.armatures.new('CINF_%08X')\n"
      "arm_obj = bpy.data.objects.new(arm.name, arm)\n"
      "bpy.context.scene.collection.objects.link(arm_obj)\n"
      "bpy.context.view_layer.objects.active = arm_obj\n"
      "bpy.ops.object.mode_set(mode='EDIT')\n"
      "arm_bone_table = {}\n",
      cinfId.toUint32());

  for (const DNAANIM::RigInverter<CINF>::Bone& bone : inverter.getBones()) {
    zeus::simd_floats originF(bone.m_origBone.origin.simd);
    zeus::simd_floats tailF(bone.m_tail.mSimd);
    os.format(
        "bone = arm.edit_bones.new('%s')\n"
        "bone.head = (%f,%f,%f)\n"
        "bone.tail = (%f,%f,%f)\n"
        "bone.use_inherit_scale = False\n"
        "arm_bone_table[%u] = bone\n",
        getBoneNameFromId(bone.m_origBone.id)->c_str(), originF[0], originF[1], originF[2], tailF[0], tailF[1],
        tailF[2], bone.m_origBone.id);
  }

  for (const Bone& bone : bones)
    if (bone.parentId != 2)
      os.format("arm_bone_table[%u].parent = arm_bone_table[%u]\n", bone.id, bone.parentId);

  os << "bpy.ops.object.mode_set(mode='OBJECT')\n";

  for (const DNAANIM::RigInverter<CINF>::Bone& bone : inverter.getBones())
    os.format("arm_obj.pose.bones['%s'].rotation_mode = 'QUATERNION'\n",
              getBoneNameFromId(bone.m_origBone.id)->c_str());
}

std::string CINF::GetCINFArmatureName(const UniqueID32& cinfId) { return hecl::Format("CINF_%08X", cinfId.toUint32()); }

int CINF::RecursiveAddArmatureBone(const Armature& armature, const BlenderBone* bone, int parent, int& curId,
                                   std::unordered_map<std::string, atInt32>& idMap,
                                   std::map<std::string, int>& nameMap) {
  int selId;
  auto search = idMap.find(bone->name);
  if (search == idMap.end()) {
    selId = curId++;
    idMap.emplace(std::make_pair(bone->name, selId));
  } else
    selId = search->second;

  bones.emplace_back();
  Bone& boneOut = bones.back();
  nameMap[bone->name] = selId;
  boneOut.id = selId;
  boneOut.parentId = parent;
  boneOut.origin = bone->origin;
  boneOut.linkedCount = bone->children.size() + 1;
  boneOut.linked.reserve(boneOut.linkedCount);

  const BlenderBone* child;
  boneOut.linked.push_back(parent);
  for (size_t i = 0; (child = armature.getChild(bone, i)); ++i)
    boneOut.linked.push_back(RecursiveAddArmatureBone(armature, child, boneOut.id, curId, idMap, nameMap));

  return boneOut.id;
}

CINF::CINF(const Armature& armature, std::unordered_map<std::string, atInt32>& idMap) {
  idMap.reserve(armature.bones.size());
  bones.reserve(armature.bones.size());

  std::map<std::string, int> nameMap;

  const BlenderBone* bone = armature.getRoot();
  if (bone) {
    if (bone->children.size()) {
      int curId = 4;
      const BlenderBone* child;
      for (size_t i = 0; (child = armature.getChild(bone, i)); ++i)
        RecursiveAddArmatureBone(armature, child, 3, curId, idMap, nameMap);
    }

    bones.emplace_back();
    Bone& boneOut = bones.back();
    nameMap[bone->name] = 3;
    boneOut.id = 3;
    boneOut.parentId = 2;
    boneOut.origin = bone->origin;
    idMap.emplace(std::make_pair(bone->name, 3));

    if (bone->children.size()) {
      boneOut.linkedCount = 2;
      boneOut.linked = {2, 4};
    } else {
      boneOut.linkedCount = 1;
      boneOut.linked = {2};
    }
  }

  boneCount = bones.size();

  names.reserve(nameMap.size());
  nameCount = nameMap.size();
  for (const auto& name : nameMap) {
    names.emplace_back();
    Name& nameOut = names.back();
    nameOut.name = name.first;
    nameOut.boneId = name.second;
  }

  boneIdCount = boneCount;
  boneIds.reserve(boneIdCount);
  for (auto it = bones.crbegin(); it != bones.crend(); ++it)
    boneIds.push_back(it->id);
}

} // namespace DataSpec::DNAMP1
