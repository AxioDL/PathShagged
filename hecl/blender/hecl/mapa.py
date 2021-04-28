import bpy, struct, bmesh, operator
from mathutils import Vector

# Function to quantize normals to 15-bit precision
def quant_norm(n):
    nf = n.copy()
    for i in range(3):
        nf[i] = int(nf[i] * 16384) / 16384.0
    return nf.freeze()

# Function to quantize lightmap UVs to 15-bit precision
def quant_luv(n):
    uf = n.copy()
    for i in range(2):
        uf[i] = int(uf[i] * 32768) / 32768.0
    return uf.freeze()

# Class for building unique sets of vertex attributes for VBO generation
class VertPool:

    # Initialize hash-unique index for each available attribute
    def __init__(self, bm, rna_loops, use_luv, material_slots):
        self.bm = bm
        self.rna_loops = rna_loops
        self.material_slots = material_slots
        self.pos = {}
        self.norm = {}
        self.skin = {}
        self.color = {}
        self.uv = {}
        self.luv = {}
        self.dlay = None
        self.clays = []
        self.ulays = []
        self.luvlay = None

        dlay = None
        if len(bm.verts.layers.deform):
            dlay = bm.verts.layers.deform[0]
            self.dlay = dlay

        clays = []
        for cl in range(len(bm.loops.layers.color)):
            clays.append(bm.loops.layers.color[cl])
        self.clays = clays

        luvlay = None
        if use_luv:
            luvlay = bm.loops.layers.uv[0]
            self.luvlay = luvlay
        ulays = []
        for ul in range(len(bm.loops.layers.uv)):
            ulays.append(bm.loops.layers.uv[ul])
        self.ulays = ulays

        # Per-vert pool attributes
        for v in bm.verts:
            pf = v.co.copy().freeze()
            if pf not in self.pos:
                self.pos[pf] = len(self.pos)
            if not rna_loops:
                nf = quant_norm(v.normal)
                if nf not in self.norm:
                    self.norm[nf] = len(self.norm)
            if dlay:
                sf = tuple(sorted(v[dlay].items()))
                if sf not in self.skin:
                    self.skin[sf] = len(self.skin)

        # Per-loop pool attributes
        for f in bm.faces:
            lightmapped = f.material_index < len(material_slots) and \
                          material_slots[f.material_index].material['retro_lightmapped']
            for l in f.loops:
                if rna_loops:
                    nf = quant_norm(rna_loops[l.index].normal)
                    if nf not in self.norm:
                        self.norm[nf] = len(self.norm)
                for cl in range(len(clays)):
                    cf = l[clays[cl]].copy().freeze()
                    if cf not in self.color:
                        self.color[cf] = len(self.color)
                start_uvlay = 0
                if use_luv and lightmapped:
                    start_uvlay = 1
                    uf = quant_luv(l[luvlay].uv)
                    if uf not in self.luv:
                        self.luv[uf] = len(self.luv)
                for ul in range(start_uvlay, len(ulays)):
                    uf = l[ulays[ul]].uv.copy().freeze()
                    if uf not in self.uv:
                        self.uv[uf] = len(self.uv)

    def write_out(self, writebuf, vert_groups):
        writebuf(struct.pack('I', len(self.pos)))
        for p in sorted(self.pos.items(), key=operator.itemgetter(1)):
            writebuf(struct.pack('fff', p[0][0], p[0][1], p[0][2]))

        writebuf(struct.pack('I', len(self.norm)))
        for n in sorted(self.norm.items(), key=operator.itemgetter(1)):
            writebuf(struct.pack('fff', n[0][0], n[0][1], n[0][2]))

        writebuf(struct.pack('II', len(self.clays), len(self.color)))
        for c in sorted(self.color.items(), key=operator.itemgetter(1)):
            writebuf(struct.pack('fff', c[0][0], c[0][1], c[0][2]))

        writebuf(struct.pack('II', len(self.ulays), len(self.uv)))
        for u in sorted(self.uv.items(), key=operator.itemgetter(1)):
            writebuf(struct.pack('ff', u[0][0], u[0][1]))

        luv_count = 0
        if self.luvlay is not None:
            luv_count = 1
        writebuf(struct.pack('II', luv_count, len(self.luv)))
        for u in sorted(self.luv.items(), key=operator.itemgetter(1)):
            writebuf(struct.pack('ff', u[0][0], u[0][1]))

        writebuf(struct.pack('I', len(vert_groups)))
        for vgrp in vert_groups:
            writebuf(struct.pack('I', len(vgrp.name)))
            writebuf(vgrp.name.encode())

        writebuf(struct.pack('I', len(self.skin)))
        for s in sorted(self.skin.items(), key=operator.itemgetter(1)):
            entries = s[0]
            writebuf(struct.pack('I', len(entries)))
            if len(entries):
                total_len = 0.0
                for ent in entries:
                    total_len += ent[1]
                for ent in entries:
                    writebuf(struct.pack('If', ent[0], ent[1] / total_len))

    def write_out_map(self, writebuf):
        writebuf(struct.pack('I', len(self.pos)))
        for p in sorted(self.pos.items(), key=operator.itemgetter(1)):
            writebuf(struct.pack('fff', p[0][0], p[0][1], p[0][2]))

    def get_pos_idx(self, vert):
        pf = vert.co.copy().freeze()
        return self.pos[pf]

    def get_norm_idx(self, loop):
        if self.rna_loops:
            nf = quant_norm(self.rna_loops[loop.index].normal)
        else:
            nf = quant_norm(loop.vert.normal)
        return self.norm[nf]

    def get_skin_idx(self, vert):
        if not self.dlay:
            return 0
        sf = tuple(sorted(vert[self.dlay].items()))
        return self.skin[sf]

    def get_color_idx(self, loop, cidx):
        cf = loop[self.clays[cidx]].copy().freeze()
        return self.color[cf]

    def get_uv_idx(self, loop, uidx):
        if self.luvlay is not None and uidx == 0:
            if self.material_slots[loop.face.material_index].material['retro_lightmapped']:
                uf = quant_luv(loop[self.luvlay].uv)
                return self.luv[uf]
        uf = loop[self.ulays[uidx]].uv.copy().freeze()
        return self.uv[uf]

    def loops_contiguous(self, la, lb):
        if la.vert != lb.vert:
            return False
        if self.get_norm_idx(la) != self.get_norm_idx(lb):
            return False
        for cl in range(len(self.clays)):
            if self.get_color_idx(la, cl) != self.get_color_idx(lb, cl):
                return False
        for ul in range(len(self.ulays)):
            if self.get_uv_idx(la, ul) != self.get_uv_idx(lb, ul):
                return False
        return True

    def splitable_edge(self, edge):
        if len(edge.link_faces) < 2:
            return False
        for v in edge.verts:
            found = None
            for f in edge.link_faces:
                for l in f.loops:
                    if l.vert == v:
                        if not found:
                            found = l
                            break
                        else:
                            if not self.loops_contiguous(found, l):
                                return True
                            break
        return False

    def loop_out(self, writebuf, loop):
        writebuf(struct.pack('B', 1))
        writebuf(struct.pack('II', self.get_pos_idx(loop.vert), self.get_norm_idx(loop)))
        for cl in range(len(self.clays)):
            writebuf(struct.pack('I', self.get_color_idx(loop, cl)))
        for ul in range(len(self.ulays)):
            writebuf(struct.pack('I', self.get_uv_idx(loop, ul)))
        sp = struct.pack('I', self.get_skin_idx(loop.vert))
        writebuf(sp)

    def null_loop_out(self, writebuf):
        writebuf(struct.pack('B', 1))
        writebuf(struct.pack('I', 0xffffffff))

    def loop_out_map(self, writebuf, loop):
        writebuf(struct.pack('B', 1))
        writebuf(struct.pack('I', self.get_pos_idx(loop.vert)))

    def vert_out_map(self, writebuf, vert):
        writebuf(struct.pack('B', 1))
        writebuf(struct.pack('I', self.get_pos_idx(vert)))


def strip_next_loop(prev_loop, out_count):
    if out_count & 1:
        radial_loop = prev_loop.link_loop_radial_next
        loop = radial_loop.link_loop_prev
        return loop, loop
    else:
        radial_loop = prev_loop.link_loop_radial_prev
        loop = radial_loop.link_loop_next
        return loop.link_loop_next, loop


def recursive_faces_islands(list_out, rem_list, face):
    if face not in rem_list:
        return []

    list_out.append(face)
    rem_list.remove(face)
    next_faces = []
    for e in face.edges:
        if not e.is_contiguous or e.seam:
            continue
        for f in e.link_faces:
            if f == face:
                continue
            next_faces.append(f)
    return next_faces

def cook(writebuf, mesh_obj):
    if mesh_obj.type != 'MESH':
        raise RuntimeError("%s is not a mesh" % mesh_obj.name)

    obj_vismodes = dict((i[0], i[3]) for i in bpy.types.Object.retro_mapobj_vis_mode.keywords['items'])

    # Write out visibility type
    vis_types = dict((i[0], i[3]) for i in bpy.types.Scene.retro_map_vis_mode.keywords['items'])
    writebuf(struct.pack('I', vis_types[bpy.context.scene.retro_map_vis_mode]))

    # Copy mesh (and apply mesh modifiers with triangulation)
    copy_name = mesh_obj.name + "_hmdltri"
    copy_mesh = bpy.data.meshes.new_from_object(mesh_obj, preserve_all_data_layers=True,
                                                depsgraph=bpy.context.evaluated_depsgraph_get())
    copy_obj = bpy.data.objects.new(copy_name, copy_mesh)
    copy_obj.scale = mesh_obj.scale
    bpy.context.scene.collection.objects.link(copy_obj)
    bpy.ops.object.select_all(action='DESELECT')
    bpy.context.view_layer.objects.active = copy_obj
    copy_obj.select_set(True)
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.quads_convert_to_tris()
    bpy.ops.mesh.select_all(action='DESELECT')
    bpy.context.scene.update_tag()
    bpy.ops.object.mode_set(mode='OBJECT')
    copy_mesh.calc_normals_split()
    rna_loops = copy_mesh.loops

    # Create master BMesh and VertPool
    bm_master = bmesh.new()
    bm_master.from_mesh(copy_obj.data)
    vert_pool = VertPool(bm_master, rna_loops, False, mesh_obj.material_slots)

    # Output vert pool
    vert_pool.write_out_map(writebuf)

    # Create map surfaces and borders
    faces_rem = list(bm_master.faces)
    loop_iter = 0
    loop_ranges = []
    while len(faces_rem):
        island_faces = []
        faces = [faces_rem[0]]
        while len(faces):
            next_faces = []
            ret_faces = None
            for f in faces:
                ret_faces = recursive_faces_islands(island_faces, faces_rem, f)
                if ret_faces == False:
                    break
                next_faces.extend(ret_faces)
            if ret_faces == False:
                break
            faces = next_faces

        # island_faces now holds one island (map edge delimited)
        prev_loop_emit = None
        loop_set = set()
        edge_set = set()
        out_count = 0
        loop_count = 0
        while len(island_faces):
            sel_lists_local = []
            restore_out_count = out_count
            for start_face in island_faces:
                for l in start_face.loops:
                    out_count = restore_out_count
                    island_local = list(island_faces)
                    if out_count & 1:
                        prev_loop = l.link_loop_prev
                        loop = prev_loop.link_loop_prev
                        sel_list = [l, prev_loop, loop]
                        prev_loop = loop
                    else:
                        prev_loop = l.link_loop_next
                        loop = prev_loop.link_loop_next
                        sel_list = [l, prev_loop, loop]
                    out_count += 3
                    island_local.remove(start_face)
                    while True:
                        if not prev_loop.edge.is_contiguous or prev_loop.edge.seam:
                            break
                        loop, prev_loop = strip_next_loop(prev_loop, out_count)
                        face = loop.face
                        if face not in island_local:
                            break
                        sel_list.append(loop)
                        island_local.remove(face)
                        out_count += 1
                    sel_lists_local.append((sel_list, island_local, out_count))
            max_count = 0
            max_sl = None
            max_island_faces = None
            for sl in sel_lists_local:
                if len(sl[0]) > max_count:
                    max_count = len(sl[0])
                    max_sl = sl[0]
                    max_island_faces = sl[1]
                    out_count = sl[2]
            island_faces = max_island_faces

            if prev_loop_emit:
                vert_pool.loop_out_map(writebuf, prev_loop_emit)
                vert_pool.loop_out_map(writebuf, max_sl[0])
                loop_count += 2
                loop_set.add(prev_loop_emit)
                loop_set.add(max_sl[0])
            loop_count += len(max_sl)
            for loop in max_sl:
                vert_pool.loop_out_map(writebuf, loop)
                prev_loop_emit = loop
                loop_set.add(loop)
                for edge in loop.face.edges:
                    if edge.seam:
                        edge_set.add(edge)

        # Create island surface with edges
        if len(edge_set):
            trace_edge = edge_set.pop()
        else:
            trace_edge = None
        edge_ranges = []
        edge_iter = loop_iter + loop_count
        while trace_edge:
            edge_count = 0
            vert_pool.vert_out_map(writebuf, trace_edge.verts[0])
            vert_pool.vert_out_map(writebuf, trace_edge.verts[1])
            edge_count += 2
            next_vert = trace_edge.verts[1]
            found_edge = True
            while found_edge:
                found_edge = False
                for edge in next_vert.link_edges:
                    if edge in edge_set:
                        edge_set.remove(edge)
                        next_vert = edge.other_vert(next_vert)
                        vert_pool.vert_out_map(writebuf, next_vert)
                        edge_count += 1
                        found_edge = True
                        break
            if len(edge_set):
                trace_edge = edge_set.pop()
            else:
                trace_edge = None
            edge_ranges.append((edge_iter, edge_count))
            edge_iter += edge_count

        pos_avg = Vector()
        norm_avg = Vector()
        if len(loop_set):
            for loop in loop_set:
                pos_avg += loop.vert.co
                norm_avg += loop.vert.normal
            pos_avg /= len(loop_set)
            norm_avg /= len(loop_set)
            norm_avg.normalize()

        loop_ranges.append((loop_iter, loop_count, edge_ranges, pos_avg, norm_avg))
        loop_iter = edge_iter

    # No more surfaces
    writebuf(struct.pack('B', 0))

    # Write out loop ranges and averages
    writebuf(struct.pack('I', len(loop_ranges)))
    for loop_range in loop_ranges:
        writebuf(struct.pack('fff', loop_range[3][0], loop_range[3][1], loop_range[3][2]))
        writebuf(struct.pack('fff', loop_range[4][0], loop_range[4][1], loop_range[4][2]))
        writebuf(struct.pack('II', loop_range[0], loop_range[1]))
        writebuf(struct.pack('I', len(loop_range[2])))
        for edge_range in loop_range[2]:
            writebuf(struct.pack('II', edge_range[0], edge_range[1]))

    # Write out mappable objects
    poi_count = 0
    for obj in bpy.context.scene.objects:
        if obj.retro_mappable_type != -1:
            poi_count += 1

    writebuf(struct.pack('I', poi_count))
    for obj in bpy.context.scene.objects:
        if obj.retro_mappable_type != -1:
            writebuf(struct.pack('III',
                                 obj.retro_mappable_type, obj_vismodes[obj.retro_mapobj_vis_mode], int(obj.retro_mappable_sclyid, 0)))
            writebuf(struct.pack('ffffffffffffffff',
                                 obj.matrix_world[0][0], obj.matrix_world[0][1], obj.matrix_world[0][2], obj.matrix_world[0][3],
                                 obj.matrix_world[1][0], obj.matrix_world[1][1], obj.matrix_world[1][2], obj.matrix_world[1][3],
                                 obj.matrix_world[2][0], obj.matrix_world[2][1], obj.matrix_world[2][2], obj.matrix_world[2][3],
                                 obj.matrix_world[3][0], obj.matrix_world[3][1], obj.matrix_world[3][2], obj.matrix_world[3][3]))

def draw(layout, context):
    obj = context.active_object
    layout.prop(context.scene, 'retro_map_vis_mode', text='Visibility Mode')
    if obj and obj.retro_mappable_type != -1:
        layout.prop(obj, 'retro_mappable_type', text='Object Type')
        layout.prop(obj, 'retro_mapobj_vis_mode', text='Object Visibility Mode')
        layout.prop(obj, 'retro_mappable_sclyid', text='Object ID')

def register():
    bpy.types.Object.retro_mappable_type = bpy.props.IntProperty(name='Retro: MAPA object type', default=-1)
    bpy.types.Object.retro_mappable_sclyid = bpy.props.StringProperty(name='Retro: MAPA object SCLY ID')
    bpy.types.Scene.retro_map_vis_mode = bpy.props.EnumProperty(items=[('ALWAYS', 'Always', 'Always Visible', 0),
    ('MAPSTATIONORVISIT', 'Map Station or Visit', 'Visible after Map Station or Visit', 1),
    ('VISIT', 'Visit', 'Visible after Visit', 2),
    ('NEVER', 'Never', 'Never Visible', 3)],
    name='Retro: Map Visibility Mode')
    bpy.types.Object.retro_mapobj_vis_mode = bpy.props.EnumProperty(items=[('ALWAYS', 'Always', 'Always Visible', 0),
    ('MAPSTATIONORVISIT', 'Map Station or Visit', 'Visible after Map Station or Visit', 1),
    ('VISIT', 'Visit', 'Visible after Door Visit', 2),
    ('NEVER', 'Never', 'Never Visible', 3),
    ('MAPSTATIONORVISIT2', 'Map Station or Visit 2', 'Visible after Map Station or Visit', 4)],
    name='Retro: Map Object Visibility Mode')
