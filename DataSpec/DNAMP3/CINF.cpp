#include "CINF.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMP3 {

void CINF::sendCINFToBlender(hecl::blender::PyOutStream& os, const UniqueID64& cinfId) const {
  DNAANIM::RigInverter<CINF> inverter(*this);

  os.format("arm = bpy.data.armatures.new('CINF_%016" PRIX64
            "')\n"
            "arm_obj = bpy.data.objects.new(arm.name, arm)\n"
            "bpy.context.scene.collection.objects.link(arm_obj)\n"
            "bpy.context.view_layer.objects.active = arm_obj\n"
            "bpy.ops.object.mode_set(mode='EDIT')\n"
            "arm_bone_table = {}\n",
            cinfId.toUint64());

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

  if (bones.size()) {
    atUint32 nullId = bones[0].parentId;
    for (const Bone& bone : bones)
      if (bone.parentId != nullId)
        os.format("arm_bone_table[%u].parent = arm_bone_table[%u]\n", bone.id, bone.parentId);
  }

  os << "bpy.ops.object.mode_set(mode='OBJECT')\n";

  for (const DNAANIM::RigInverter<CINF>::Bone& bone : inverter.getBones())
    os.format("arm_obj.pose.bones['%s'].rotation_mode = 'QUATERNION'\n",
              getBoneNameFromId(bone.m_origBone.id)->c_str());
}

std::string CINF::GetCINFArmatureName(const UniqueID64& cinfId) {
  return hecl::Format("CINF_%016" PRIX64, cinfId.toUint64());
}

} // namespace DataSpec::DNAMP3
