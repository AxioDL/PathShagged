import bpy, struct, math
from mathutils import Quaternion

def draw(layout, context):
    if bpy.context.active_object:
        obj = bpy.context.active_object
        layout.label(text="Widget Settings:", icon='OBJECT_DATA')
        layout.prop_menu_enum(obj, 'retro_widget_type', text='Widget Type')
        #layout.prop_search(obj, 'retro_widget_parent', context.scene, 'objects', text='Widget Parent')
        row = layout.row(align=True)
        row.prop(obj, 'retro_widget_default_visible', text='Visible')
        row.prop(obj, 'retro_widget_default_active', text='Active')
        row.prop(obj, 'retro_widget_cull_faces', text='Cull Faces')
        layout.prop(obj, 'retro_widget_color', text='Color')
        layout.prop_menu_enum(obj, 'retro_widget_model_draw_flags', text='Draw Flags')
        row = layout.row(align=True)
        row.prop(obj, 'retro_widget_is_worker', text='Is Worker')
        if obj.retro_widget_is_worker:
            row.prop(obj, 'retro_widget_worker_id', text='Worker Id')

        if obj.retro_widget_type == 'RETRO_MODL':
            layout.prop(obj, 'retro_model_light_mask', text='Light Mask')
        elif obj.retro_widget_type == 'RETRO_PANE':
            layout.prop(obj, 'retro_pane_dimensions', text='Dimensions')
            layout.prop(obj, 'retro_pane_scale_center', text='Center')
        elif obj.retro_widget_type == 'RETRO_TXPN':
            layout.prop(obj, 'retro_pane_dimensions', text='Dimensions')
            layout.prop(obj, 'retro_pane_scale_center', text='Center')
            layout.prop(obj, 'retro_textpane_font_path', text='Font Path')
            row = layout.row(align=True)
            row.prop(obj, 'retro_textpane_word_wrap', text='Word Wrap')
            row.prop(obj, 'retro_textpane_horizontal', text='Horizontal')
            layout.prop(obj, 'retro_textpane_fill_color', text='Fill Color')
            layout.prop(obj, 'retro_textpane_outline_color', text='Outline Color')
            layout.prop(obj, 'retro_textpane_block_extent', text='Point Dimensions')
            layout.prop(obj, 'retro_textpane_jp_font_path', text='JP Font Path')
            layout.prop(obj, 'retro_textpane_jp_font_scale', text='JP Point Dimensions')
            layout.prop_menu_enum(obj, 'retro_textpane_hjustification', text='Horizontal Justification')
            layout.prop_menu_enum(obj, 'retro_textpane_vjustification', text='Vertical Justification')
        elif obj.retro_widget_type == 'RETRO_TBGP':
            layout.prop(obj, 'retro_tablegroup_elem_count', text='Element Count')
            layout.prop(obj, 'retro_tablegroup_elem_default', text='Default Element')
            layout.prop(obj, 'retro_tablegroup_wraparound', text='Wraparound')
        elif obj.retro_widget_type == 'RETRO_GRUP':
            layout.prop(obj, 'retro_group_default_worker', text='Default Worker')
        elif obj.retro_widget_type == 'RETRO_SLGP':
            row = layout.row(align=True)
            row.prop(obj, 'retro_slider_min', text='Min')
            row.prop(obj, 'retro_slider_max', text='Max')
            layout.prop(obj, 'retro_slider_default', text='Default')
            layout.prop(obj, 'retro_slider_increment', text='Increment')
        elif obj.retro_widget_type == 'RETRO_ENRG':
            layout.prop(obj, 'retro_energybar_texture_path', text='Energy Bar Texture Path')
        elif obj.retro_widget_type == 'RETRO_METR':
            layout.prop(obj, 'retro_meter_no_round_up', text='No Round Up')
            layout.prop(obj, 'retro_meter_max_capacity', text='Max Capacity')
            layout.prop(obj, 'retro_meter_worker_count', text='Worker Count')
        elif obj.retro_widget_type == 'RETRO_LITE':
            if obj.data and obj.type == 'LIGHT':
                layout.prop(obj.data, 'retro_light_index', text='Index')
                layout.label(text="Angular Falloff:", icon='LIGHT')
                row = layout.row(align=True)
                row.prop(obj.data, 'retro_light_angle_constant', text='Constant')
                row.prop(obj.data, 'retro_light_angle_linear', text='Linear')
                row.prop(obj.data, 'retro_light_angle_quadratic', text='Quadratic')

hjustifications = None
vjustifications = None
model_draw_flags_e = None

def recursive_cook(buffer, obj, version, path_hasher, parent_name):
    buffer += struct.pack('>4s', obj.retro_widget_type[6:].encode())
    buffer += obj.name.encode() + b'\0'
    buffer += parent_name.encode() + b'\0'
    buffer += struct.pack('>bbbbffffI',
        False,
        obj.retro_widget_default_visible,
        obj.retro_widget_default_active,
        obj.retro_widget_cull_faces,
        obj.retro_widget_color[0],
        obj.retro_widget_color[1],
        obj.retro_widget_color[2],
        obj.retro_widget_color[3],
        model_draw_flags_e[obj.retro_widget_model_draw_flags])

    angle = Quaternion((1.0, 0.0, 0.0), 0)

    if obj.retro_widget_type == 'RETRO_CAMR':
        angle = Quaternion((1.0, 0.0, 0.0), math.radians(-90.0))
        aspect = bpy.context.scene.render.resolution_x / bpy.context.scene.render.resolution_y

        if obj.data.type == 'PERSP':
            if aspect > 1.0:
                fov = math.degrees(math.atan(math.tan(obj.data.angle / 2.0) / aspect)) * 2.0
            else:
                fov = math.degrees(obj.data.angle)
            buffer += struct.pack('>Iffff', 0, fov, aspect, obj.data.clip_start, obj.data.clip_end)

        elif obj.data.type == 'ORTHO':
            ortho_half = obj.data.ortho_scale / 2.0
            buffer += struct.pack('>Iffffff', 1, -ortho_half, ortho_half, ortho_half / aspect,
                                  -ortho_half / aspect, obj.data.clip_start, obj.data.clip_end)

    elif obj.retro_widget_type == 'RETRO_MODL':
        if len(obj.children) == 0:
            raise RuntimeException('Model Widget must have a child model object')
        model_obj = obj.children[0]
        if model_obj.type != 'MESH':
            raise RuntimeException('Model Widget must have a child MESH')
        if not model_obj.data.library:
            raise RuntimeException('Model Widget must have a linked library MESH')
        path = bpy.path.abspath(model_obj.data.library.filepath)
        path_hash = path_hasher.hashpath32(path)
        buffer += struct.pack('>III', path_hash, 0, obj.retro_model_light_mask)

    elif obj.retro_widget_type == 'RETRO_PANE':
        buffer += struct.pack('>fffff',
                              obj.retro_pane_dimensions[0],
                              obj.retro_pane_dimensions[1],
                              obj.retro_pane_scale_center[0],
                              obj.retro_pane_scale_center[1],
                              obj.retro_pane_scale_center[2])

    elif obj.retro_widget_type == 'RETRO_TXPN':
        path_hash = path_hasher.hashpath32(obj.retro_textpane_font_path)
        buffer += struct.pack('>fffffIbbIIffffffffff',
                              obj.retro_pane_dimensions[0],
                              obj.retro_pane_dimensions[1],
                              obj.retro_pane_scale_center[0],
                              obj.retro_pane_scale_center[1],
                              obj.retro_pane_scale_center[2],
                              path_hash,
                              obj.retro_textpane_word_wrap,
                              obj.retro_textpane_horizontal,
                              hjustifications[obj.retro_textpane_hjustification],
                              vjustifications[obj.retro_textpane_vjustification],
                              obj.retro_textpane_fill_color[0],
                              obj.retro_textpane_fill_color[1],
                              obj.retro_textpane_fill_color[2],
                              obj.retro_textpane_fill_color[3],
                              obj.retro_textpane_outline_color[0],
                              obj.retro_textpane_outline_color[1],
                              obj.retro_textpane_outline_color[2],
                              obj.retro_textpane_outline_color[3],
                              obj.retro_textpane_block_extent[0],
                              obj.retro_textpane_block_extent[1])
        if version >= 1:
            path_hash = path_hasher.hashpath32(obj.retro_textpane_jp_font_path)
            buffer += struct.pack('>III',
                                  path_hash,
                                  obj.retro_textpane_jp_font_scale[0],
                                  obj.retro_textpane_jp_font_scale[1])

    elif obj.retro_widget_type == 'RETRO_TBGP':
        buffer += struct.pack('>HHIHHbbffbfHHHH',
                              obj.retro_tablegroup_elem_count,
                              0,
                              0,
                              obj.retro_tablegroup_elem_default,
                              0,
                              obj.retro_tablegroup_wraparound,
                              False,
                              0.0,
                              0.0,
                              False,
                              0.0,
                              0,
                              0,
                              0,
                              0)

    elif obj.retro_widget_type == 'RETRO_GRUP':
        buffer += struct.pack('>Hb',
                              obj.retro_group_default_worker,
                              False)

    elif obj.retro_widget_type == 'RETRO_SLGP':
        buffer += struct.pack('>ffff',
                              obj.retro_slider_min,
                              obj.retro_slider_max,
                              obj.retro_slider_default,
                              obj.retro_slider_increment)

    elif obj.retro_widget_type == 'RETRO_ENRG':
        path_hash = path_hasher.hashpath32(obj.retro_energybar_texture_path)
        buffer += struct.pack('>I', path_hash)

    elif obj.retro_widget_type == 'RETRO_METR':
        buffer += struct.pack('>bbII',
                              False,
                              obj.retro_meter_no_round_up,
                              obj.retro_meter_max_capacity,
                              obj.retro_meter_worker_count)

    elif obj.retro_widget_type == 'RETRO_LITE':
        angle = Quaternion((1.0, 0.0, 0.0), math.radians(-90.0))
        type_enum = 0
        constant = 1.0
        linear = 0.0
        quadratic = 0.0
        cutoff = 0.0
        if obj.data.type == 'POINT':
            type_enum = 4
        elif obj.data.type == 'SUN':
            type_enum = 2
        elif obj.data.type == 'SPOT':
            type_enum = 0
            cutoff = obj.data.spot_size

        if obj.data.type == 'POINT' or obj.data.type == 'SPOT':
            constant = obj.data.constant_coefficient
            linear = obj.data.linear_coefficient
            quadratic = obj.data.quadratic_coefficient

        buffer += struct.pack('>IffffffI',
                              type_enum, constant, linear, quadratic,
                              obj.data.retro_light_angle_constant,
                              obj.data.retro_light_angle_linear,
                              obj.data.retro_light_angle_quadratic,
                              obj.data.retro_light_index)
        if obj.data.type == 'SPOT':
            buffer += struct.pack('>f', cutoff)

    elif obj.retro_widget_type == 'RETRO_IMGP':
        if obj.type != 'MESH':
            raise RuntimeException('Imagepane Widget must be a MESH')
        if len(obj.data.loops) < 4:
            raise RuntimeException('Imagepane Widget must be a MESH with 4 verts')
        if len(obj.data.uv_layers) < 1:
            raise RuntimeException('Imagepane Widget must ba a MESH with a UV layer')
        path_hash = 0xffffffff
        if len(obj.data.materials):
            material = obj.data.materials[0]
            if 'Image Texture' in material.node_tree.nodes:
                image_node = material.node_tree.nodes['Image Texture']
                if image_node.image:
                    image = image_node.image
                    path = bpy.path.abspath(image.filepath)
                    path_hash = path_hasher.hashpath32(path)

        buffer += struct.pack('>IIII', path_hash, 0, 0, 4)
        for i in range(4):
            vi = obj.data.loops[i].vertex_index
            co = obj.data.vertices[vi].co
            buffer += struct.pack('>fff', co[0], co[1], co[2])

        buffer += struct.pack('>I', 4)
        for i in range(4):
            co = obj.data.uv_layers[0].data[i].uv
            buffer += struct.pack('>ff', co[0], co[1])

    if obj.retro_widget_is_worker:
        buffer += struct.pack('>bH', True, obj.retro_widget_worker_id)
    else:
        buffer += struct.pack('>b', False)

    angMtx = angle.to_matrix() @ obj.matrix_local.to_3x3()
    buffer += struct.pack('>fffffffffffffffIH',
        obj.matrix_local[0][3],
        obj.matrix_local[1][3],
        obj.matrix_local[2][3],
        angMtx[0][0], angMtx[0][1], angMtx[0][2],
        angMtx[1][0], angMtx[1][1], angMtx[1][2],
        angMtx[2][0], angMtx[2][1], angMtx[2][2],
        0.0, 0.0, 0.0, 0, 0)

    ch_list = []
    for ch in obj.children:
        ch_list.append((ch.pass_index, ch.name))
    for s_pair in sorted(ch_list):
        ch = bpy.data.objects[s_pair[1]]
        if ch.retro_widget_type != 'RETRO_NONE':
            recursive_cook(buffer, ch, version, path_hasher, obj.name)


def cook(writepipebuf, version, path_hasher):
    global hjustifications, vjustifications, model_draw_flags_e
    hjustifications = dict((i[0], i[3]) for i in bpy.types.Object.retro_textpane_hjustification.keywords['items'])
    vjustifications = dict((i[0], i[3]) for i in bpy.types.Object.retro_textpane_vjustification.keywords['items'])
    model_draw_flags_e = dict((i[0], i[3]) for i in bpy.types.Object.retro_widget_model_draw_flags.keywords['items'])

    buffer = bytearray()
    buffer += struct.pack('>IIII', 0, 0, 0, 0)

    widget_count = 0
    for obj in bpy.data.objects:
        if obj.retro_widget_type != 'RETRO_NONE':
            widget_count += 1
    buffer += struct.pack('>I', widget_count)

    for obj in bpy.data.objects:
        if obj.retro_widget_type != 'RETRO_NONE' and not obj.parent:
            recursive_cook(buffer, obj, version, path_hasher, 'kGSYS_DummyWidgetID')

    return buffer


# Registration
def register():
    frame_widget_types = [
        ('RETRO_NONE', 'Not a Widget', '', 0),
        ('RETRO_BWIG', 'Base Widget', '', 1),
        ('RETRO_CAMR', 'Camera', '', 2),
        ('RETRO_ENRG', 'Energy Bar', '', 3),
        ('RETRO_GRUP', 'Group', '', 4),
        ('RETRO_HWIG', 'Head Widget', '', 5),
        ('RETRO_IMGP', 'Image Pane', '', 6),
        ('RETRO_LITE', 'Light', '', 7),
        ('RETRO_MODL', 'Model', '', 8),
        ('RETRO_METR', 'Meter', '', 9),
        ('RETRO_PANE', 'Pane', '', 10),
        ('RETRO_SLGP', 'Slider Group', '', 11),
        ('RETRO_TBGP', 'Table Group', '', 12),
        ('RETRO_TXPN', 'Text Pane', '', 13)]
    bpy.types.Object.retro_widget_type = bpy.props.EnumProperty(items=frame_widget_types, name='Retro: FRME Widget Type', default='RETRO_NONE')
    model_draw_flags = [
        ('RETRO_SHADELESS', 'Shadeless', '', 0),
        ('RETRO_OPAQUE', 'Opaque', '', 1),
        ('RETRO_ALPHA', 'Alpha', '', 2),
        ('RETRO_ADDITIVE', 'Additive', '', 3),
        ('RETRO_ALPHA_ADDITIVE_OVERDRAW', 'Alpha Additive Overdraw', '', 4)]
    bpy.types.Object.retro_widget_parent = bpy.props.StringProperty(name='Retro: FRME Widget Parent', description='Refers to internal frame widgets')
    bpy.types.Object.retro_widget_use_anim_controller = bpy.props.BoolProperty(name='Retro: Use Animation Controller')
    bpy.types.Object.retro_widget_default_visible = bpy.props.BoolProperty(name='Retro: Default Visible', description='Sets widget is visible by default')
    bpy.types.Object.retro_widget_default_active = bpy.props.BoolProperty(name='Retro: Default Active', description='Sets widget is cases by default')
    bpy.types.Object.retro_widget_cull_faces = bpy.props.BoolProperty(name='Retro: Cull Faces', description='Enables face culling')
    bpy.types.Object.retro_widget_color = bpy.props.FloatVectorProperty(name='Retro: Color', description='Sets widget color', subtype='COLOR', size=4, min=0.0, max=1.0)
    bpy.types.Object.retro_widget_model_draw_flags = bpy.props.EnumProperty(items=model_draw_flags, name='Retro: Model Draw Flags', default='RETRO_ALPHA')
    bpy.types.Object.retro_widget_is_worker = bpy.props.BoolProperty(name='Retro: Is Worker Widget', default=False)
    bpy.types.Object.retro_widget_worker_id = bpy.props.IntProperty(name='Retro: Worker Widget ID', min=0, default=0)

    bpy.types.Object.retro_model_light_mask = bpy.props.IntProperty(name='Retro: Model Light Mask', min=0, default=0)

    bpy.types.Object.retro_pane_dimensions = bpy.props.FloatVectorProperty(name='Retro: Pane Dimensions', min=0.0, size=2)
    bpy.types.Object.retro_pane_scale_center = bpy.props.FloatVectorProperty(name='Retro: Scale Center', size=3)

    bpy.types.Object.retro_textpane_font_path = bpy.props.StringProperty(name='Retro: Font Path')
    bpy.types.Object.retro_textpane_word_wrap = bpy.props.BoolProperty(name='Retro: Word Wrap')
    bpy.types.Object.retro_textpane_horizontal = bpy.props.BoolProperty(name='Retro: Horizontal', default=True)
    bpy.types.Object.retro_textpane_fill_color = bpy.props.FloatVectorProperty(name='Retro: Fill Color', min=0.0, max=1.0, size=4, subtype='COLOR')
    bpy.types.Object.retro_textpane_outline_color = bpy.props.FloatVectorProperty(name='Retro: Outline Color', min=0.0, max=1.0, size=4, subtype='COLOR')
    bpy.types.Object.retro_textpane_block_extent = bpy.props.FloatVectorProperty(name='Retro: Block Extent', min=0.0, size=2)
    bpy.types.Object.retro_textpane_jp_font_path = bpy.props.StringProperty(name='Retro: Japanese Font Path')
    bpy.types.Object.retro_textpane_jp_font_scale = bpy.props.IntVectorProperty(name='Retro: Japanese Font Scale', min=0, size=2)
    frame_textpane_hjustifications = [
        ('LEFT', 'Left', '', 0),
        ('CENTER', 'Center', '', 1),
        ('RIGHT', 'Right', '', 2),
        ('FULL', 'Full', '', 3),
        ('NLEFT', 'Left Normalized', '', 4),
        ('NCENTER', 'Center Normalized', '', 5),
        ('NRIGHT', 'Right Normalized', '', 6),
        ('LEFTMONO', 'Left Monospaced', '', 7),
        ('CENTERMONO', 'Center Monospaced', '', 8),
        ('RIGHTMONO', 'Right Monospaced', '', 9)]
    bpy.types.Object.retro_textpane_hjustification = bpy.props.EnumProperty(items=frame_textpane_hjustifications, name='Retro: Horizontal Justification', default='LEFT')
    frame_textpane_vjustifications = [
        ('TOP', 'Top', '', 0),
        ('CENTER', 'Center', '', 1),
        ('BOTTOM', 'Bottom', '', 2),
        ('FULL', 'Full', '', 3),
        ('NTOP', 'Top Normalized', '', 4),
        ('NCENTER', 'Center Normalized', '', 5),
        ('NBOTTOM', 'Bottom Normalized', '', 6),
        ('TOPMONO', 'Top Monospaced', '', 7),
        ('CENTERMONO', 'Center Monospaced', '', 8),
        ('BOTTOMMONO', 'Bottom Monospaced', '', 9)]
    bpy.types.Object.retro_textpane_vjustification = bpy.props.EnumProperty(items=frame_textpane_vjustifications, name='Retro: Vertical Justification', default='TOP')

    bpy.types.Object.retro_tablegroup_elem_count = bpy.props.IntProperty(name='Retro: Table Group Element Count', min=0, default=0)
    bpy.types.Object.retro_tablegroup_elem_default = bpy.props.IntProperty(name='Retro: Table Group Default Element', min=0, default=0)
    bpy.types.Object.retro_tablegroup_wraparound = bpy.props.BoolProperty(name='Retro: Table Group Wraparound', default=False)

    bpy.types.Object.retro_group_default_worker = bpy.props.IntProperty(name='Retro: Group Default Worker', min=0, default=0)

    bpy.types.Object.retro_slider_min = bpy.props.FloatProperty(name='Retro: Slider Min', default=0.0)
    bpy.types.Object.retro_slider_max = bpy.props.FloatProperty(name='Retro: Slider Max', default=1.0)
    bpy.types.Object.retro_slider_default = bpy.props.FloatProperty(name='Retro: Slider Default', default=0.0)
    bpy.types.Object.retro_slider_increment = bpy.props.FloatProperty(name='Retro: Slider Increment', min=0.0, default=1.0)

    bpy.types.Object.retro_energybar_texture_path = bpy.props.StringProperty(name='Retro: Energy Bar Texture Path')

    bpy.types.Object.retro_meter_no_round_up = bpy.props.BoolProperty(name='Retro: No Round Up', default=True)
    bpy.types.Object.retro_meter_max_capacity = bpy.props.IntProperty(name='Retro: Max Capacity', min=0, default=100)
    bpy.types.Object.retro_meter_worker_count = bpy.props.IntProperty(name='Retro: Worker Count', min=0, default=1)

    bpy.types.Light.retro_light_index = bpy.props.IntProperty(name='Retro: Light Index', min=0, default=0)
    bpy.types.Light.retro_light_angle_constant = bpy.props.FloatProperty(name='Retro: Light Angle Constant', min=0.0, default=0.0)
    bpy.types.Light.retro_light_angle_linear = bpy.props.FloatProperty(name='Retro: Light Angle Linear', min=0.0, default=0.0)
    bpy.types.Light.retro_light_angle_quadratic = bpy.props.FloatProperty(name='Retro: Light Angle Quadratic', min=0.0, default=0.0)

