#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct BloodFlower : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  PatternedInfo patternedInfo;
  ActorParameters actorParameters;
  UniqueID32 particle1;
  UniqueID32 wpsc1;
  UniqueID32 wpsc2;
  DamageInfo damageInfo1;
  DamageInfo damageInfo2;
  DamageInfo damageInfo3;
  UniqueID32 particle2;
  UniqueID32 particle3;
  UniqueID32 particle4;
  Value<float> unknown1;
  UniqueID32 particle5;
  Value<atUint32> unknown2;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const override {
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, patternedInfo.animationParameters);
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    if (wpsc1.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc1);
      ent->name = name + "_wpsc1";
    }
    if (wpsc2.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc2);
      ent->name = name + "_wpsc2";
    }
    if (particle1.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle1);
      ent->name = name + "_part1";
    }
    if (particle2.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle2);
      ent->name = name + "_part2";
    }
    if (particle3.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle3);
      ent->name = name + "_part3";
    }
    if (particle4.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle4);
      ent->name = name + "_part4";
    }
    if (particle5.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle5);
      ent->name = name + "_part5";
    }
    patternedInfo.nameIDs(pakRouter, name + "_patterned");
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    g_curSpec->flattenDependencies(wpsc1, pathsOut);
    g_curSpec->flattenDependencies(wpsc2, pathsOut);
    g_curSpec->flattenDependencies(particle1, pathsOut);
    g_curSpec->flattenDependencies(particle2, pathsOut);
    g_curSpec->flattenDependencies(particle3, pathsOut);
    g_curSpec->flattenDependencies(particle4, pathsOut);
    g_curSpec->flattenDependencies(particle5, pathsOut);
    patternedInfo.depIDs(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const override { actorParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
