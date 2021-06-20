#include "FRME.hpp"
#include "DataSpec/DNACommon/TXTR.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMP1 {

template <>
void FRME::Enumerate<BigDNA::Read>(athena::io::IStreamReader& __dna_reader) {
  /* version */
  version = __dna_reader.readUint32Big();
  /* unk1 */
  unk1 = __dna_reader.readUint32Big();
  /* modelCount */
  modelCount = __dna_reader.readUint32Big();
  /* unk3 */
  unk3 = __dna_reader.readUint32Big();
  /* widgetCount */
  widgetCount = __dna_reader.readUint32Big();
  /* widgets */
  __dna_reader.enumerate<Widget>(widgets, widgetCount, [this](athena::io::IStreamReader& reader, Widget& w) {
    w.owner = this;
    w.read(reader);
  });
}

template <>
void FRME::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& __dna_writer) {
  /* version */
  __dna_writer.writeUint32Big(version);
  /* unk1 */
  __dna_writer.writeUint32Big(unk1);
  /* modelCount */
  __dna_writer.writeUint32Big(modelCount);
  /* unk3 */
  __dna_writer.writeUint32Big(unk3);
  /* widgetCount */
  __dna_writer.writeUint32Big(widgetCount);
  /* widgets */
  __dna_writer.enumerate(widgets);
}

template <>
void FRME::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  for (const Widget& w : widgets)
    w.binarySize(__isz);
  __isz += 20;
}

template <>
void FRME::Widget::Enumerate<BigDNA::Read>(athena::io::IStreamReader& __dna_reader) {
  /* type */
  type.read(__dna_reader);
  /* header */
  header.read(__dna_reader);
  switch (type.toUint32()) {
  case SBIG('BWIG'):
    widgetInfo = std::make_unique<BWIGInfo>();
    break;
  case SBIG('HWIG'):
    widgetInfo = std::make_unique<HWIGInfo>();
    break;
  case SBIG('CAMR'):
    widgetInfo = std::make_unique<CAMRInfo>();
    break;
  case SBIG('LITE'):
    widgetInfo = std::make_unique<LITEInfo>();
    break;
  case SBIG('ENRG'):
    widgetInfo = std::make_unique<ENRGInfo>();
    break;
  case SBIG('MODL'):
    widgetInfo = std::make_unique<MODLInfo>();
    break;
  case SBIG('METR'):
    widgetInfo = std::make_unique<METRInfo>();
    break;
  case SBIG('GRUP'):
    widgetInfo = std::make_unique<GRUPInfo>();
    break;
  case SBIG('PANE'):
    widgetInfo = std::make_unique<PANEInfo>();
    break;
  case SBIG('TXPN'):
    widgetInfo = std::make_unique<TXPNInfo>(owner->version);
    break;
  case SBIG('IMGP'):
    widgetInfo = std::make_unique<IMGPInfo>();
    break;
  case SBIG('TBGP'):
    widgetInfo = std::make_unique<TBGPInfo>();
    break;
  case SBIG('SLGP'):
    widgetInfo = std::make_unique<SLGPInfo>();
    break;
  default:
    Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Unsupported FRME widget type {}")), type);
  }

  /* widgetInfo */
  widgetInfo->read(__dna_reader);

  /* isWorker */
  isWorker = __dna_reader.readBool();
  if (isWorker) {
    /* workerId */
    workerId = __dna_reader.readUint16Big();
  }
  /* origin */
  origin = __dna_reader.readVec3fBig();
  /* basis[0] */
  basis[0] = __dna_reader.readVec3fBig();
  /* basis[1] */
  basis[1] = __dna_reader.readVec3fBig();
  /* basis[2] */
  basis[2] = __dna_reader.readVec3fBig();
  /* rotationCenter */
  rotationCenter = __dna_reader.readVec3fBig();
  /* unk1 */
  unk1 = __dna_reader.readInt32Big();
  /* unk2 */
  unk2 = __dna_reader.readInt16Big();
}

template <>
void FRME::Widget::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& __dna_writer) {
  /* type */
  DNAFourCC _type = widgetInfo ? widgetInfo->fourcc() : FOURCC('BWIG');
  _type.write(__dna_writer);
  /* header */
  header.write(__dna_writer);

  /* widgetInfo */
  if (widgetInfo)
    widgetInfo->write(__dna_writer);

  /* isWorker */
  __dna_writer.writeBool(isWorker);
  if (isWorker) {
    /* workerId */
    __dna_writer.writeUint16Big(workerId);
  }
  /* origin */
  __dna_writer.writeVec3fBig(origin);
  /* basis[0] */
  __dna_writer.writeVec3fBig(basis[0]);
  /* basis[1] */
  __dna_writer.writeVec3fBig(basis[1]);
  /* basis[2] */
  __dna_writer.writeVec3fBig(basis[2]);
  /* rotationCenter */
  __dna_writer.writeVec3fBig(rotationCenter);
  /* unk1 */
  __dna_writer.writeInt32Big(unk1);
  /* unk2 */
  __dna_writer.writeInt16Big(unk2);
}

template <>
void FRME::Widget::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  type.binarySize(__isz);
  header.binarySize(__isz);
  if (widgetInfo)
    widgetInfo->binarySize(__isz);
  if (isWorker)
    __isz += 4;
  __isz += 67;
}

template <>
void FRME::Widget::CAMRInfo::Enumerate<BigDNA::Read>(athena::io::IStreamReader& __dna_reader) {
  projectionType = ProjectionType(__dna_reader.readUint32Big());
  if (projectionType == ProjectionType::Perspective) {
    projection = std::make_unique<PerspectiveProjection>();
  } else if (projectionType == ProjectionType::Orthographic) {
    projection = std::make_unique<OrthographicProjection>();
  } else {
    Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Invalid CAMR projection mode! {}")), int(projectionType));
  }

  projection->read(__dna_reader);
}

template <>
void FRME::Widget::CAMRInfo::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& __dna_writer) {
  if (!projection)
    Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Invalid CAMR projection object!")));
  if (projection->type != projectionType)
    Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("CAMR projection type does not match actual projection type!")));

  __dna_writer.writeUint32Big(atUint32(projectionType));
  projection->write(__dna_writer);
}

template <>
void FRME::Widget::CAMRInfo::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  projection->binarySize(__isz);
  __isz += 4;
}

template <>
void FRME::Widget::LITEInfo::Enumerate<BigDNA::Read>(athena::io::IStreamReader& __dna_reader) {
  /* type */
  type = ELightType(__dna_reader.readUint32Big());
  /* distC */
  distC = __dna_reader.readFloatBig();
  /* distL */
  distL = __dna_reader.readFloatBig();
  /* distQ */
  distQ = __dna_reader.readFloatBig();
  /* angC */
  angC = __dna_reader.readFloatBig();
  /* angL */
  angL = __dna_reader.readFloatBig();
  /* angQ */
  angQ = __dna_reader.readFloatBig();
  /* loadedIdx */
  loadedIdx = __dna_reader.readUint32Big();

  /* cutoff */
  if (type == ELightType::Spot)
    cutoff = __dna_reader.readFloatBig();
}

template <>
void FRME::Widget::LITEInfo::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& __dna_writer) {
  /* type */
  __dna_writer.writeUint32Big(atUint32(type));
  /* distC */
  __dna_writer.writeFloatBig(distC);
  /* distL */
  __dna_writer.writeFloatBig(distL);
  /* distQ */
  __dna_writer.writeFloatBig(distQ);
  /* angC */
  __dna_writer.writeFloatBig(angC);
  /* angL */
  __dna_writer.writeFloatBig(angL);
  /* angQ */
  __dna_writer.writeFloatBig(angQ);
  /* loadedIdx */
  __dna_writer.writeUint32Big(loadedIdx);

  /* cutoff */
  if (type == ELightType::Spot)
    __dna_writer.writeFloatBig(cutoff);
}

template <>
void FRME::Widget::LITEInfo::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  __isz += ((type == ELightType::Spot) ? 36 : 32);
}

template <class Op>
void FRME::Widget::TXPNInfo::Enumerate(typename Op::StreamT& s) {
  Do<Op>(athena::io::PropId{"xDim"}, xDim, s);
  Do<Op>(athena::io::PropId{"zDim"}, zDim, s);
  Do<Op>(athena::io::PropId{"scaleCenter"}, scaleCenter, s);
  Do<Op>(athena::io::PropId{"font"}, font, s);
  Do<Op>(athena::io::PropId{"wordWrap"}, wordWrap, s);
  Do<Op>(athena::io::PropId{"horizontal"}, horizontal, s);
  Do<Op>(athena::io::PropId{"justification"}, justification, s);
  Do<Op>(athena::io::PropId{"verticalJustification"}, verticalJustification, s);
  Do<Op>(athena::io::PropId{"fillColor"}, fillColor, s);
  Do<Op>(athena::io::PropId{"outlineColor"}, outlineColor, s);
  Do<Op>(athena::io::PropId{"blockExtent"}, blockExtent, s);
  if (version == 1) {
    Do<Op>(athena::io::PropId{"jpnFont"}, jpnFont, s);
    Do<Op>(athena::io::PropId{"jpnPointScale[0]"}, jpnPointScale[0], s);
    Do<Op>(athena::io::PropId{"jpnPointScale[1]"}, jpnPointScale[1], s);
  }
}

AT_SPECIALIZE_DNA(FRME::Widget::TXPNInfo)

bool FRME::Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter, const PAK::Entry& entry, bool force, hecl::blender::Token& btok,
                   std::function<void(const hecl::SystemChar*)> fileChanged) {
  if (!force && outPath.isFile())
    return true;

  FRME frme;
  frme.read(rs);

  hecl::blender::Connection& conn = btok.getBlenderConnection();
  if (!conn.createBlend(outPath, hecl::blender::BlendType::Frame))
    return false;

  hecl::blender::PyOutStream os = conn.beginPythonOut(true);

  os << "import bpy, math, bmesh\n"
        "from mathutils import Matrix, Quaternion\n"
        "# Clear Scene\n"
        "if len(bpy.data.collections):\n"
        "    bpy.data.collections.remove(bpy.data.collections[0])\n"
        "\n"
        "def duplicateObject(copy_obj):\n"
        "    # Create new mesh\n"
        "    mesh = bpy.data.meshes.new(copy_obj.name)\n"
        "    # Create new object associated with the mesh\n"
        "    ob_new = bpy.data.objects.new(copy_obj.name, mesh)\n"
        "    # Copy data block from the old object into the new object\n"
        "    ob_new.data = copy_obj.data\n"
        "    ob_new.scale = copy_obj.scale\n"
        "    ob_new.location = copy_obj.location\n"
        "    # Link new object to the given scene and select it\n"
        "    bpy.context.scene.collection.objects.link(ob_new)\n"
        "    return ob_new\n";

  os.format(FMT_STRING("bpy.context.scene.name = '{}'\n"
                       "bpy.context.scene.render.resolution_x = 640\n"
                       "bpy.context.scene.render.resolution_y = 480\n"
                       "bpy.context.scene.world.use_nodes = True\n"
                       "bg_node = bpy.context.scene.world.node_tree.nodes['Background']\n"
                       "bg_node.inputs[1].default_value = 0.0\n"),
            pakRouter.getBestEntryName(entry));

  int pIdx = 0;
  for (const FRME::Widget& w : frme.widgets) {
    os << "binding = None\n"
          "angle = Quaternion((1.0, 0.0, 0.0), 0)\n";
    if (w.type == SBIG('CAMR')) {
      using CAMRInfo = Widget::CAMRInfo;
      os.format(FMT_STRING("cam = bpy.data.cameras.new(name='{}')\n"
                           "binding = cam\n"),
                w.header.name);
      if (CAMRInfo* info = static_cast<CAMRInfo*>(w.widgetInfo.get())) {
        if (info->projectionType == CAMRInfo::ProjectionType::Orthographic) {
          CAMRInfo::OrthographicProjection* proj =
              static_cast<CAMRInfo::OrthographicProjection*>(info->projection.get());
          os.format(FMT_STRING("cam.type = 'ORTHO'\n"
                               "cam.ortho_scale = {}\n"
                               "cam.clip_start = {}\n"
                               "cam.clip_end = {}\n"),
                    std::fabs(proj->right - proj->left), proj->znear, proj->zfar);
        } else if (info->projectionType == CAMRInfo::ProjectionType::Perspective) {
          CAMRInfo::PerspectiveProjection* proj = static_cast<CAMRInfo::PerspectiveProjection*>(info->projection.get());
          os.format(FMT_STRING("cam.type = 'PERSP'\n"
                               "cam.lens_unit = 'FOV'\n"
                               "cam.clip_start = {}\n"
                               "cam.clip_end = {}\n"
                               "bpy.context.scene.render.resolution_x = 480 * {}\n"),
                    proj->znear, proj->zfar, proj->aspect);
          if (proj->aspect > 1.f)
            os.format(FMT_STRING("cam.angle = math.atan2({}, 1.0 / math.tan(math.radians({} / 2.0))) * 2.0\n"),
                      proj->aspect, proj->fov);
          else
            os.format(FMT_STRING("cam.angle = math.radians({})\n"), proj->fov);
        }
      }
      os << "angle = Quaternion((1.0, 0.0, 0.0), math.radians(90.0))\n";
    } else if (w.type == SBIG('LITE')) {
      using LITEInfo = Widget::LITEInfo;
      if (LITEInfo* info = static_cast<LITEInfo*>(w.widgetInfo.get())) {
        switch (info->type) {
        case LITEInfo::ELightType::LocalAmbient: {
          zeus::simd_floats colorF(w.header.color.simd);
          os.format(FMT_STRING("bg_node.inputs[0].default_value = ({},{},{},1.0)\n"
                               "bg_node.inputs[1].default_value = {}\n"),
                    colorF[0], colorF[1], colorF[2], info->distQ / 8.0);
          break;
        }
        case LITEInfo::ELightType::Spot:
        case LITEInfo::ELightType::Directional:
          os << "angle = Quaternion((1.0, 0.0, 0.0), math.radians(90.0))\n";
          [[fallthrough]];
        default: {
          zeus::simd_floats colorF(w.header.color.simd);
          os.format(FMT_STRING("lamp = bpy.data.lights.new(name='{}', type='POINT')\n"
                               "lamp.color = ({}, {}, {})\n"
                               "lamp.hecl_falloff_constant = {}\n"
                               "lamp.hecl_falloff_linear = {}\n"
                               "lamp.hecl_falloff_quadratic = {}\n"
                               "lamp.retro_light_angle_constant = {}\n"
                               "lamp.retro_light_angle_linear = {}\n"
                               "lamp.retro_light_angle_quadratic = {}\n"
                               "lamp.retro_light_index = {}\n"
                               "binding = lamp\n"),
                    w.header.name, colorF[0], colorF[1], colorF[2], info->distC, info->distL, info->distQ, info->angC,
                    info->angL, info->angQ, info->loadedIdx);
          if (info->type == LITEInfo::ELightType::Spot)
            os.format(FMT_STRING("lamp.type = 'SPOT'\n"
                                 "lamp.spot_size = {}\n"),
                      info->cutoff);
          else if (info->type == LITEInfo::ELightType::Directional)
            os << "lamp.type = 'SUN'\n";
        }
        }
      }
    } else if (w.type == SBIG('IMGP')) {
      using IMGPInfo = Widget::IMGPInfo;
      if (IMGPInfo* info = static_cast<IMGPInfo*>(w.widgetInfo.get())) {
        std::string texName;
        hecl::SystemString resPath;
        if (info->texture.isValid()) {
          texName = pakRouter.getBestEntryName(info->texture);
          const nod::Node* node;
          const PAKRouter<PAKBridge>::EntryType* texEntry = pakRouter.lookupEntry(info->texture, &node);
          hecl::ProjectPath txtrPath = pakRouter.getWorking(texEntry);
          if (txtrPath.isNone()) {
            txtrPath.makeDirChain(false);
            PAKEntryReadStream rs = texEntry->beginReadStream(*node);
            TXTR::Extract(rs, txtrPath);
          }
          resPath = pakRouter.getResourceRelativePath(entry, info->texture);
        }

        if (resPath.size()) {
          hecl::SystemUTF8Conv resPathView(resPath);
          os.format(FMT_STRING("if '{}' in bpy.data.images:\n"
                               "    image = bpy.data.images['{}']\n"
                               "else:\n"
                               "    image = bpy.data.images.load('''//{}''')\n"
                               "    image.name = '{}'\n"),
                    texName, texName, resPathView, texName);
        } else {
          os << "image = None\n";
        }

        os.format(FMT_STRING("material = bpy.data.materials.new('{}')\n"
                             "material.use_nodes = True\n"
                             "new_nodetree = material.node_tree\n"
                             "for n in new_nodetree.nodes:\n"
                             "    new_nodetree.nodes.remove(n)\n"
                             "tex_node = new_nodetree.nodes.new('ShaderNodeTexImage')\n"
                             "tex_node.image = image\n"
                             "bm = bmesh.new()\n"
                             "verts = []\n"),
                  w.header.name);

        for (uint32_t i = 0; i < info->quadCoordCount; ++i) {
          int ti;
          if (i == 2)
            ti = 3;
          else if (i == 3)
            ti = 2;
          else
            ti = i;
          zeus::simd_floats f(info->quadCoords[ti].simd);
          os.format(FMT_STRING("verts.append(bm.verts.new(({},{},{})))\n"), f[0], f[1], f[2]);
        }
        os << "bm.faces.new(verts)\n"
              "bm.loops.layers.uv.new('UV')\n"
              "bm.verts.ensure_lookup_table()\n";
        for (uint32_t i = 0; i < info->uvCoordCount; ++i) {
          int ti;
          if (i == 2)
            ti = 3;
          else if (i == 3)
            ti = 2;
          else
            ti = i;
          zeus::simd_floats f(info->uvCoords[ti].simd);
          os.format(FMT_STRING("bm.verts[{}].link_loops[0][bm.loops.layers.uv[0]].uv = ({},{})\n"), i, f[0], f[1]);
        }
        os.format(FMT_STRING("binding = bpy.data.meshes.new('{}')\n"
                             "bm.to_mesh(binding)\n"
                             "bm.free()\n"
                             "binding.materials.append(material)\n"),
                  w.header.name);
      }
    }

    zeus::simd_floats colorF(w.header.color.simd);
    os.format(FMT_STRING(
        "frme_obj = bpy.data.objects.new(name='{}', object_data=binding)\n"
        "frme_obj.pass_index = {}\n"
        "parentName = '{}'\n"
        "frme_obj.retro_widget_type = 'RETRO_{}'\n"
        "frme_obj.retro_widget_use_anim_controller = {}\n"
        "frme_obj.retro_widget_default_visible = {}\n"
        "frme_obj.retro_widget_default_active = {}\n"
        "frme_obj.retro_widget_cull_faces = {}\n"
        "frme_obj.retro_widget_color = ({},{},{},{})\n"
        "frme_obj.retro_widget_model_draw_flags = bpy.types.Object.retro_widget_model_draw_flags.keywords['items'][{}][0]\n"
        "frme_obj.retro_widget_is_worker = {}\n"
        "frme_obj.retro_widget_worker_id = {}\n"
        "if parentName not in bpy.data.objects:\n"
        "    frme_obj.retro_widget_parent = parentName\n"
        "else:\n"
        "    frme_obj.parent = bpy.data.objects[parentName]\n"),
        w.header.name, pIdx++, w.header.parent, w.type,
        w.header.useAnimController ? "True" : "False", w.header.defaultVisible ? "True" : "False",
        w.header.defaultActive ? "True" : "False", w.header.cullFaces ? "True" : "False", colorF[0], colorF[1],
        colorF[2], colorF[3], w.header.modelDrawFlags, w.isWorker ? "True" : "False", w.workerId);

    if (w.type == SBIG('MODL')) {
      using MODLInfo = FRME::Widget::MODLInfo;
      MODLInfo* info = static_cast<MODLInfo*>(w.widgetInfo.get());
      hecl::ProjectPath modelPath = pakRouter.getWorking(info->model);
      const PAKRouter<PAKBridge>::EntryType* cmdlE = pakRouter.lookupEntry(info->model, nullptr, true, true);

      os.linkMesh(modelPath.getAbsolutePathUTF8(), pakRouter.getBestEntryName(*cmdlE));

      os.format(FMT_STRING("frme_obj.retro_model_light_mask = {}\n"), info->lightMask);
      os << "print(obj.name)\n"
            "copy_obj = duplicateObject(obj)\n"
            "copy_obj.parent = frme_obj\n"
            "copy_obj.hide_set(False)\n";
    } else if (w.type == SBIG('CAMR')) {
      os << "bpy.context.scene.camera = frme_obj\n"
            "if 'Camera' in bpy.data.objects:\n"
            "    cam = bpy.data.objects['Camera']\n"
            "    #bpy.context.scene.objects.unlink(cam)\n"
            "    bpy.data.objects.remove(cam)\n";
    } else if (w.type == SBIG('PANE')) {
      using PANEInfo = Widget::PANEInfo;
      if (PANEInfo* info = static_cast<PANEInfo*>(w.widgetInfo.get())) {
        zeus::simd_floats f(info->scaleCenter.simd);
        os.format(FMT_STRING("frme_obj.retro_pane_dimensions = ({},{})\n"
                             "frme_obj.retro_pane_scale_center = ({},{},{})\n"),
                  info->xDim, info->zDim, f[0], f[1], f[2]);
      }
    } else if (w.type == SBIG('TXPN')) {
      using TXPNInfo = Widget::TXPNInfo;
      if (TXPNInfo* info = static_cast<TXPNInfo*>(w.widgetInfo.get())) {
        hecl::ProjectPath fontPath = pakRouter.getWorking(info->font, true);
        hecl::ProjectPath jpFontPath;
        if (frme.version >= 1)
          jpFontPath = pakRouter.getWorking(info->jpnFont, true);

        zeus::simd_floats scaleF(info->scaleCenter.simd);
        zeus::simd_floats fillF(info->fillColor.simd);
        zeus::simd_floats outlineF(info->outlineColor.simd);
        zeus::simd_floats extentF(info->blockExtent.simd);
        os.format(FMT_STRING(
            "frme_obj.retro_pane_dimensions = ({},{})\n"
            "frme_obj.retro_pane_scale_center = ({},{},{})\n"
            "frme_obj.retro_textpane_font_path = '{}'\n"
            "frme_obj.retro_textpane_word_wrap = {}\n"
            "frme_obj.retro_textpane_horizontal = {}\n"
            "frme_obj.retro_textpane_fill_color = ({},{},{},{})\n"
            "frme_obj.retro_textpane_outline_color = ({},{},{},{})\n"
            "frme_obj.retro_textpane_block_extent = ({},{})\n"
            "frme_obj.retro_textpane_jp_font_path = '{}'\n"
            "frme_obj.retro_textpane_jp_font_scale = ({},{})\n"
            "frme_obj.retro_textpane_hjustification = "
            "bpy.types.Object.retro_textpane_hjustification.keywords['items'][{}][0]\n"
            "frme_obj.retro_textpane_vjustification = "
            "bpy.types.Object.retro_textpane_vjustification.keywords['items'][{}][0]\n"),
            info->xDim, info->zDim, scaleF[0], scaleF[1], scaleF[2], fontPath.getRelativePathUTF8(),
            info->wordWrap ? "True" : "False", info->horizontal ? "True" : "False", fillF[0], fillF[1], fillF[2],
            fillF[3], outlineF[0], outlineF[1], outlineF[2], outlineF[3], extentF[0], extentF[1],
            jpFontPath.getRelativePathUTF8(), info->jpnPointScale[0], info->jpnPointScale[1],
            int(info->justification), int(info->verticalJustification));
      }
    } else if (w.type == SBIG('TBGP')) {
      using TBGPInfo = Widget::TBGPInfo;
      if (TBGPInfo* info = static_cast<TBGPInfo*>(w.widgetInfo.get())) {
        os.format(FMT_STRING("frme_obj.retro_tablegroup_elem_count = {}\n"
                             "frme_obj.retro_tablegroup_elem_default = {}\n"
                             "frme_obj.retro_tablegroup_wraparound = {}\n"),
                  info->elementCount, info->defaultSelection, info->selectWraparound ? "True" : "False");
      }
    } else if (w.type == SBIG('GRUP')) {
      using GRUPInfo = Widget::GRUPInfo;
      if (GRUPInfo* info = static_cast<GRUPInfo*>(w.widgetInfo.get())) {
        os.format(FMT_STRING("frme_obj.retro_group_default_worker = {}\n"), info->defaultWorker);
      }
    } else if (w.type == SBIG('SLGP')) {
      using SLGPInfo = Widget::SLGPInfo;
      if (SLGPInfo* info = static_cast<SLGPInfo*>(w.widgetInfo.get())) {
        os.format(FMT_STRING("frme_obj.retro_slider_min = {}\n"
                             "frme_obj.retro_slider_max = {}\n"
                             "frme_obj.retro_slider_default = {}\n"
                             "frme_obj.retro_slider_increment = {}\n"),
                  info->min, info->max, info->cur, info->increment);
      }
    } else if (w.type == SBIG('ENRG')) {
      using ENRGInfo = Widget::ENRGInfo;
      if (ENRGInfo* info = static_cast<ENRGInfo*>(w.widgetInfo.get())) {
        hecl::ProjectPath txtrPath = pakRouter.getWorking(info->texture);
        if (txtrPath)
          os.format(FMT_STRING("frme_obj.retro_energybar_texture_path = '{}'\n"), txtrPath.getRelativePathUTF8());
      }
    } else if (w.type == SBIG('METR')) {
      using METRInfo = Widget::METRInfo;
      if (METRInfo* info = static_cast<METRInfo*>(w.widgetInfo.get())) {
        os.format(FMT_STRING("frme_obj.retro_meter_no_round_up = {}\n"
                             "frme_obj.retro_meter_max_capacity = {}\n"
                             "frme_obj.retro_meter_worker_count = {}\n"),
                  info->noRoundUp ? "True" : "False", info->maxCapacity, info->workerCount);
      }
    }

    zeus::simd_floats xfMtxF[3];
    for (int i = 0; i < 3; ++i)
      w.basis[i].simd.copy_to(xfMtxF[i]);
    zeus::simd_floats originF(w.origin.simd);
    os.format(FMT_STRING("mtx = Matrix((({},{},{},{}),({},{},{},{}),({},{},{},{}),(0.0,0.0,0.0,1.0)))\n"
                         "mtxd = mtx.decompose()\n"
                         "frme_obj.rotation_mode = 'QUATERNION'\n"
                         "frme_obj.location = mtxd[0]\n"
                         "frme_obj.rotation_quaternion = mtxd[1] @ angle\n"
                         "frme_obj.scale = mtxd[2]\n"
                         "bpy.context.scene.collection.objects.link(frme_obj)\n"),
              xfMtxF[0][0], xfMtxF[0][1], xfMtxF[0][2], originF[0], xfMtxF[1][0], xfMtxF[1][1], xfMtxF[1][2],
              originF[1], xfMtxF[2][0], xfMtxF[2][1], xfMtxF[2][2], originF[2]);
  }

  os.centerView();
  os.close();
  conn.saveBlend();
  return true;
}

} // namespace DataSpec::DNAMP1
