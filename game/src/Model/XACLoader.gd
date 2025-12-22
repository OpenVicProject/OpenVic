class_name XACLoader

static var unit_shader : ShaderMaterial = preload("res://src/Model/unit_colours_mat.tres")
const MAX_UNIT_TEXTURES : int = 32 # max number of textures supported by the shader
const EXTRA_CULL_MARGIN : int = 2 # extra margin to stop sub-meshes from being culled near the edges of screens
static var added_unit_textures_spec : PackedStringArray
static var added_unit_textures_diffuse : PackedStringArray

static var flag_shader : ShaderMaterial = preload("res://src/Model/flag_mat.tres")

static var scrolling_shader : ShaderMaterial = preload("res://src/Model/scrolling_mat.tres")
const MAX_SCROLLING_TEXTURES : int = 32 # max number of textures supported by the shader
static var added_scrolling_textures_diffuse : PackedStringArray
const SCROLLING_MATERIAL_FACTORS : Dictionary = {
	"TexAnim" : 2.5, # Tank tracks
	"Smoke" : 0.3 # Buildings, factories, steam ships, sieges
}

static func setup_flag_shader() -> void:
	flag_shader.set_shader_parameter(&"flag_dims", GameSingleton.get_flag_dims())
	flag_shader.set_shader_parameter(&"texture_flag_sheet_diffuse", GameSingleton.get_flag_sheet_texture())

# Keys: source_file (String)
# Values: loaded model (UnitModel or Node3D) or LOAD_FAILED_MARKER (StringName)
static var xac_cache : Dictionary

const LOAD_FAILED_MARKER : StringName = &"XAC LOAD FAILED"

static func get_xac_model(source_file : String, is_unit : bool) -> Node3D:
	var cached : Variant = xac_cache.get(source_file)
	if not cached:
		cached = _load_xac_model(source_file, is_unit)
		if cached:
			xac_cache[source_file] = cached
		else:
			xac_cache[source_file] = LOAD_FAILED_MARKER
			push_error("Failed to get XAC model \"", source_file, "\" (current load failed)")
			return null

	if not cached is Node3D:
		push_error("Failed to get XAC model \"", source_file, "\" (previous load failed)")
		return null

	var node : Node3D = cached.duplicate()
	if node is UnitModel:
		node.unit_init()
	return node

static func _load_xac_model(source_file : String, is_unit : bool) -> Node3D:
	var source_path : String = GameSingleton.lookup_file_path(source_file)
	var file : FileAccess = FileAccess.open(source_path, FileAccess.READ)
	if file == null:
		push_error("Failed to load XAC ", source_file, " from looked up path ", source_path)
		return null

	var metaDataChunk : MetadataChunk
	var nodeHierarchyChunk : NodeHierarchyChunk
	var materialTotalsChunk : MaterialTotalsChunk
	var materialDefinitionChunks : Array[MaterialDefinitionChunk] = []
	var mesh_chunks : Array[MeshChunk] = []
	var skinningChunks : Array[SkinningChunk] = []
	var chunkType6s : Array[ChunkType6] = []
	var nodeChunks : Array[NodeChunk] = []
	var chunkType4s : Array[ChunkTypeUnknown] = []
	var chunkUnknowns : Array[ChunkTypeUnknown] = []

	readHeader(file)

	while file.get_position() < file.get_length():
		var type : int = FileAccessUtils.read_int32(file)
		var length : int = FileAccessUtils.read_int32(file)
		var version : int = FileAccessUtils.read_int32(file)
		match type:
			0x7:
				metaDataChunk = readMetaDataChunk(file)
			0xB:
				nodeHierarchyChunk = readNodeHierarchyChunk(file)
			0xD:
				materialTotalsChunk = readMaterialTotalsChunk(file)
			0x3:
				# Ver=1 Appears on old version of format
				var chunk : MaterialDefinitionChunk = readMaterialDefinitionChunk(file, version==1)
				if chunk.has_specular():
					is_unit = true
				materialDefinitionChunks.push_back(chunk)
			0x1:
				mesh_chunks.push_back(readMeshChunk(file))
			0x2:
				skinningChunks.push_back(readSkinningChunk(file,mesh_chunks, version==2))
			0x6:
				chunkType6s.push_back(readChunkType6(file))
			0x0: # Appears on old version of format
				nodeChunks.push_back(readNodeChunk(file))
			0xA: # Appears on old version of format
				chunkUnknowns.push_back(readChunkTypeUnknown(file, length))
			0x4: # Appears on old version of format
				chunkType4s.push_back(readChunkTypeUnknown(file, length))
			0x8:
				push_warning("XAC model ", source_file, " contains junk data chunk 0x8 (skipping)")
				break
			_:
				push_error(">> INVALID XAC CHUNK TYPE %s in model %s" % [type, source_file])
				break

	#BUILD THE GODOT MATERIALS
	var materials : Array[MaterialDefinition] = make_materials(materialDefinitionChunks)

	#BUILD THE MESH
	var node : Node3D = null

	if is_unit:
		node = UnitModel.new()
	else:
		node = Node3D.new()

	node.name = metaDataChunk.origFileName.replace("\\", "/").split("/", false)[-1].get_slice(".", 0)

	var skeleton : Skeleton3D = null

	# build the skeleton hierarchy
	if nodeHierarchyChunk:
		skeleton = build_armature(nodeHierarchyChunk)
	elif not nodeChunks.is_empty():
		skeleton = build_armature_chunk0(nodeChunks)

	if skeleton:
		node.add_child(skeleton)
	else:
		push_warning("MODEL HAS NO SKELETON: ", source_file)

	var st : SurfaceTool = SurfaceTool.new()

	for chunk : MeshChunk in mesh_chunks:
		var mesh_chunk_name : String
		if nodeHierarchyChunk:
			mesh_chunk_name = nodeHierarchyChunk.nodes[chunk.nodeId].name
		elif not nodeChunks.is_empty():
			mesh_chunk_name = nodeChunks[chunk.nodeId].name

		const INVALID_MESHES : PackedStringArray = ["polySurface95"]
		if mesh_chunk_name in INVALID_MESHES:
			push_warning("Skipping unused mesh \"", mesh_chunk_name, "\" in model \"", node.name, "\"")
			continue

		var mesh : ArrayMesh = null
		var verts : PackedVector3Array
		var normals : PackedVector3Array
		var tangents : Array[Vector4]
		var uvs : Array[PackedVector2Array] = []
		var influenceRangeInd : PackedInt64Array

		# vert attributes could be in any order, so search for them
		for vertAttrib : VerticesAttribute in chunk.VerticesAttributes:
			match vertAttrib.type:
				0: # position
					verts = vertAttrib.data
				1: # normals vec3
					normals = vertAttrib.data
				2: # tangents vec4
					tangents = vertAttrib.data
				3: # uv coords vec2
					uvs.push_back(vertAttrib.data) #can have multiple sets of uv data
				5: # influence range, uint32
					influenceRangeInd = vertAttrib.data
				_: # type 4 32bit colours and type 6 128bit colours aren't used
					pass

		#FIXME: find a better solution if possible
		#pCube1 hardcoding is to fix the cruiser which doesn't properly mark its collision mesh
		if chunk.bIsCollisionMesh or mesh_chunk_name == "pCube1":
			var ar3d : Area3D = Area3D.new()
			node.add_child(ar3d)
			ar3d.owner = node
			for submesh : SubMesh in chunk.SubMeshes:
				var shape : ConvexPolygonShape3D = ConvexPolygonShape3D.new()
				shape.points = verts

				var col : CollisionShape3D = CollisionShape3D.new()
				col.shape = shape

				ar3d.add_child(col)
				col.owner = node
			continue

		#TODO will this produce correct results?
		var applyVertexWeights : bool = true
		var skinning_chunk_ind : int = 0
		for skin : SkinningChunk in skinningChunks:
			if skin.nodeId == chunk.nodeId:
				break
			skinning_chunk_ind += 1
		if skinning_chunk_ind >= len(skinningChunks):
			skinning_chunk_ind = 1
			applyVertexWeights = false

		# polySurface97 corresponds to the "arab_infantry_helmet", and needs to be removed often
		# but only in cases where it isn't an attachment
		# problem, this is also the body of the S-P infantry
		# so this should only be valid if inside an attachment or makes use of bone weights
		const INVALID_IF_NOT_ONLY_MESH : PackedStringArray = ["polySurface97"]
		if influenceRangeInd.is_empty() or skinningChunks.is_empty() or not applyVertexWeights:
			if mesh_chunks.size() != 1 and mesh_chunk_name in INVALID_IF_NOT_ONLY_MESH:
				push_warning("Skipping unused mesh \"", mesh_chunk_name, "\" in model \"", node.name, "\" because it was not the only mesh chunk in its file")
				break

		var meshInstance : MeshInstance3D = MeshInstance3D.new()
		node.add_child(meshInstance)
		meshInstance.owner = node

		#stop the culling of units near the tops of screens
		meshInstance.extra_cull_margin = EXTRA_CULL_MARGIN

		if mesh_chunk_name:
			meshInstance.name = mesh_chunk_name

		if skeleton:
			meshInstance.skeleton = meshInstance.get_path_to(skeleton)

		if not verts.is_empty():
			var vert_total : int = 0
			var surfaceIndex : int = 0

			for submesh : SubMesh in chunk.SubMeshes:
				st.begin(Mesh.PRIMITIVE_TRIANGLES)

				for i : int in submesh.relativeIndices.size():
					var rel_index : int = vert_total + submesh.relativeIndices[i]

					if not normals.is_empty():
						st.set_normal(normals[rel_index])

					if not tangents.is_empty():
						st.set_tangent(Plane(
							-tangents[rel_index].x,
							tangents[rel_index].y,
							tangents[rel_index].z,
							tangents[rel_index].w
						))

					if not uvs.is_empty():
						st.set_uv(uvs[0][rel_index])

					if not influenceRangeInd.is_empty() and not skinningChunks.is_empty() and applyVertexWeights:
						#TODO: Which skinning Chunk?
						# likely look at the skinning chunk's nodeId, see if it matches our mesh's id
						var vert_inf_range_ind : int = influenceRangeInd[rel_index]
						var skin_chunk : SkinningChunk = skinningChunks[skinning_chunk_ind]
						var influenceRange : InfluenceRange = skin_chunk.influenceRange[vert_inf_range_ind]
						var boneWeights : Array[InfluenceData] = skinningChunks[skinning_chunk_ind].influenceData.slice(
							influenceRange.firstInfluenceIndex,
							influenceRange.firstInfluenceIndex + influenceRange.numInfluences
						)
						if len(boneWeights) > 4:
							push_error("num BONE WEIGHTS WAS > 4, GODOT DOESN'T LIKE THIS")
							# TODO: Less hacky fix?
							boneWeights = boneWeights.slice(0,4)

						var godotBoneIds : PackedInt32Array = PackedInt32Array()
						var godotBoneWeights : PackedFloat32Array = PackedFloat32Array()
						godotBoneIds.resize(4)
						godotBoneIds.fill(0)
						godotBoneWeights.resize(4)
						godotBoneWeights.fill(0)

						var index : int = 0
						for bone : InfluenceData in boneWeights:
							godotBoneIds.set(index, bone.boneId)
							godotBoneWeights.set(index, bone.fWeight)
							index += 1

						if skeleton:
							st.set_bones(godotBoneIds)
							st.set_weights(godotBoneWeights)

					st.add_vertex(verts[rel_index])

				vert_total += submesh.numVertices

				mesh = st.commit(mesh) # add a new surface to the mesh
				meshInstance.mesh = mesh

				st.clear()

				mesh.surface_set_material(surfaceIndex, materials[submesh.materialId].mat)
				surfaceIndex += 1

				if materials[submesh.materialId].spec_index != -1:
					meshInstance.set_instance_shader_parameter(&"tex_index_specular", materials[submesh.materialId].spec_index)

				if materials[submesh.materialId].diffuse_index != -1:
					meshInstance.set_instance_shader_parameter(&"tex_index_diffuse", materials[submesh.materialId].diffuse_index)

				if materials[submesh.materialId].scroll_index != -1:
					meshInstance.set_instance_shader_parameter(&"scroll_tex_index_diffuse", materials[submesh.materialId].scroll_index)

	return node

# Information needed to set up a material
# Leave the indices -1 if not using the unit shader
class MaterialDefinition:
	var spec_index : int = -1
	var diffuse_index : int = -1
	var scroll_index : int = -1
	var mat : Material

	func _init(mat : Material, diffuse_ind : int = -1, spec_ind : int = -1, scroll_ind : int = -1) -> void:
		self.mat = mat
		self.diffuse_index = diffuse_ind
		self.spec_index = spec_ind
		self.scroll_index = scroll_ind

static func make_materials(materialDefinitionChunks : Array[MaterialDefinitionChunk]) -> Array[MaterialDefinition]:
	const TEXTURES_PATH : String = "gfx/anims/%s.dds"

	var materials : Array[MaterialDefinition] = []

	for matdef : MaterialDefinitionChunk in materialDefinitionChunks:
		var diffuse_name : String
		var specular_name : String
		var normal_name : String

		# Find important textures
		for layer : Layer in matdef.Layers:
			if layer.texture in ["nospec", "unionjacksquare", "test256texture"]:
				continue

			match layer.mapType:
				2: # diffuse
					if not diffuse_name:
						diffuse_name = layer.texture
					else:
						push_error("Multiple diffuse layers in material: ", diffuse_name, " and ", layer.texture)

				3: # specular
					if not specular_name:
						specular_name = layer.texture
					else:
						push_error("Multiple specular layers in material: ", specular_name, " and ", layer.texture)

				4: # currently unused
					pass

				5: # normal
					if not normal_name:
						normal_name = layer.texture
					else:
						push_error("Multiple normal layers in material: ", normal_name, " and ", layer.texture)

				_:
					push_error("Unknown layer type: ", layer.mapType)
					pass

		# Unit colour mask
		if diffuse_name and specular_name:
			if normal_name:
				push_error("Normal texture present in unit colours material: ", normal_name)

			var textures_index_spec : int = added_unit_textures_spec.find(specular_name)
			if textures_index_spec < 0:
				var unit_colours_mask_texture : ImageTexture = AssetManager.get_texture(TEXTURES_PATH % specular_name)
				if unit_colours_mask_texture:
					added_unit_textures_spec.push_back(specular_name)

					# Should we still attempt to add the texture to the shader?
					if len(added_unit_textures_spec) >= MAX_UNIT_TEXTURES:
						push_error("Colour masks have exceeded max number of textures supported by unit shader!")

					const param_texture_nation_colors_mask : StringName = &"texture_nation_colors_mask"

					var colour_masks : Array = unit_shader.get_shader_parameter(param_texture_nation_colors_mask)
					colour_masks.push_back(unit_colours_mask_texture)
					textures_index_spec = len(colour_masks) - 1
					unit_shader.set_shader_parameter(param_texture_nation_colors_mask, colour_masks)
				else:
					push_error("Failed to load specular texture: ", specular_name)

			var textures_index_diffuse : int = added_unit_textures_diffuse.find(diffuse_name)
			if textures_index_diffuse < 0:
				var diffuse_texture : ImageTexture = AssetManager.get_texture(TEXTURES_PATH % diffuse_name)
				if diffuse_texture:
					added_unit_textures_diffuse.push_back(diffuse_name)

					# Should we still attempt to add the texture to the shader?
					if len(added_unit_textures_diffuse) >= MAX_UNIT_TEXTURES:
						push_error("Diffuse textures have exceeded max number supported by unit shader!")

					const param_texture_diffuse : StringName = &"texture_diffuse"

					var diffuse_textures : Array = unit_shader.get_shader_parameter(param_texture_diffuse)
					diffuse_textures.push_back(diffuse_texture)
					textures_index_diffuse = len(diffuse_textures) - 1
					unit_shader.set_shader_parameter(param_texture_diffuse, diffuse_textures)
				else:
					push_error("Failed to load diffuse texture: ", diffuse_name)

			materials.push_back(MaterialDefinition.new(unit_shader, textures_index_diffuse, textures_index_spec))

		# Flag (diffuse is unionjacksquare which is ignored)
		elif normal_name and not diffuse_name:
			if specular_name:
				push_error("Specular texture present in flag material: ", specular_name)

			var flag_normal_texture : ImageTexture = AssetManager.get_texture(TEXTURES_PATH % normal_name)
			if flag_normal_texture:
				flag_shader.set_shader_parameter(&"texture_normal", flag_normal_texture)
			else:
				push_error("Failed to load normal texture: ", normal_name)

			materials.push_back(MaterialDefinition.new(flag_shader))

		# Scrolling texture
		elif diffuse_name and matdef.name in SCROLLING_MATERIAL_FACTORS:
			if specular_name:
				push_error("Specular texture present in scrolling material: ", specular_name)
			if normal_name:
				push_error("Normal texture present in scrolling material: ", normal_name)

			var scroll_textures_index_diffuse : int = added_scrolling_textures_diffuse.find(diffuse_name)
			if scroll_textures_index_diffuse < 0:
				var diffuse_texture : ImageTexture = AssetManager.get_texture(TEXTURES_PATH % diffuse_name)
				if diffuse_texture:
					added_scrolling_textures_diffuse.push_back(diffuse_name)

					# Should we still attempt to add the texture to the shader?
					if len(added_scrolling_textures_diffuse) >= MAX_SCROLLING_TEXTURES:
						push_error("Diffuse textures have exceeded max number supported by scrolling shader!")

					const param_scroll_texture_diffuse : StringName = &"scroll_texture_diffuse"

					var scroll_diffuse_textures : Array = scrolling_shader.get_shader_parameter(param_scroll_texture_diffuse)
					scroll_diffuse_textures.push_back(diffuse_texture)
					scroll_textures_index_diffuse = len(scroll_diffuse_textures) - 1
					scrolling_shader.set_shader_parameter(param_scroll_texture_diffuse, scroll_diffuse_textures)

					const param_scroll_factor : StringName = &"scroll_factor"

					var scroll_factors : Array = scrolling_shader.get_shader_parameter(param_scroll_factor)
					scroll_factors.push_back(SCROLLING_MATERIAL_FACTORS[matdef.name])
					scrolling_shader.set_shader_parameter(param_scroll_factor, scroll_factors)
				else:
					push_error("Failed to load diffuse texture: ", diffuse_name)

			materials.push_back(MaterialDefinition.new(scrolling_shader, -1, -1, scroll_textures_index_diffuse))

		# Standard material
		else:
			if specular_name:
				push_error("Specular texture present in standard material: ", specular_name)

			var mat : StandardMaterial3D = StandardMaterial3D.new()
			mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA_DEPTH_PRE_PASS

			if diffuse_name:
				var diffuse_texture : ImageTexture = AssetManager.get_texture(TEXTURES_PATH % diffuse_name)
				if diffuse_texture:
					mat.set_texture(BaseMaterial3D.TEXTURE_ALBEDO, diffuse_texture)
				else:
					push_error("Failed to load diffuse texture: ", diffuse_name)

			if normal_name:
				var normal_texture : ImageTexture = AssetManager.get_texture(TEXTURES_PATH % normal_name)
				if normal_texture:
					mat.normal_enabled = true
					mat.set_texture(BaseMaterial3D.TEXTURE_NORMAL, normal_texture)
				else:
					push_error("Failed to load normal texture: ", normal_name)

			#TODO: Verify that this is correct thing to do to make sure
			#that places where models are double sided are correct
			mat.cull_mode = BaseMaterial3D.CULL_DISABLED

			materials.push_back(MaterialDefinition.new(mat))

	return materials

static func build_armature(hierarchy_chunk : NodeHierarchyChunk) -> Skeleton3D:
	var skeleton : Skeleton3D = Skeleton3D.new()
	skeleton.name = "skeleton"
	var cur_id : int = 0
	for node : NodeData in hierarchy_chunk.nodes:
		# godot doesn't like the ':' in bone names unlike paradox
		skeleton.add_bone(FileAccessUtils.replace_chars(node.name))
		skeleton.set_bone_parent(cur_id, node.parentNodeId)

		#For now assume rest and current position are the same
		skeleton.set_bone_rest(cur_id, Transform3D(Basis(node.rotation).scaled(node.scale), node.position))

		skeleton.set_bone_pose_position(cur_id, node.position)
		skeleton.set_bone_pose_rotation(cur_id, node.rotation)
		skeleton.set_bone_pose_scale(cur_id, node.scale)

		cur_id += 1
	# conveniently both godot and xac use a parent node id of -1 to represent no parent
	# TODO: What is the point of xac having both a transform and separate vec3s for rotation, scale, pos, etc.?
	# for now, will assume the separate components are the truth
	# it might be that one is a current position and the other is a rest position?
	return skeleton

static func build_armature_chunk0(nodeChunks : Array[NodeChunk]) -> Skeleton3D:
	var skeleton : Skeleton3D = Skeleton3D.new()
	skeleton.name = "skeleton"
	var cur_id : int = 0
	for node : NodeChunk in nodeChunks:
		# godot doesn't like the ':' in bone names unlike paradox
		skeleton.add_bone(FileAccessUtils.replace_chars(node.name))
		skeleton.set_bone_parent(cur_id, node.parentBone)

		# For now assume rest and current position are the same
		skeleton.set_bone_rest(cur_id, Transform3D(Basis(node.rotation).scaled(node.scale), node.position))

		skeleton.set_bone_pose_position(cur_id, node.position)
		skeleton.set_bone_pose_rotation(cur_id, node.rotation)
		skeleton.set_bone_pose_scale(cur_id, node.scale)

		cur_id += 1
	return skeleton

static func readHeader(file : FileAccess) -> void:
	var magic_bytes : PackedByteArray = [file.get_8(), file.get_8(), file.get_8(), file.get_8()]
	var magic : String = magic_bytes.get_string_from_ascii()
	var version : String = "%d.%d" % [file.get_8(), file.get_8()]
	var bBigEndian : bool = file.get_8()
	var multiplyOrder : int = file.get_8()
	#print(magic, ", version: ", version, ", bigEndian: ", bBigEndian, " multiplyOrder: ", multiplyOrder)

static func readMetaDataChunk(file : FileAccess) -> MetadataChunk:
	return MetadataChunk.new(
		file.get_32(), FileAccessUtils.read_int32(file), file.get_8(), file.get_8(),
		Vector2i(file.get_8(), file.get_8()), file.get_float(),
		FileAccessUtils.read_xac_str(file), FileAccessUtils.read_xac_str(file),
		FileAccessUtils.read_xac_str(file), FileAccessUtils.read_xac_str(file)
	)

class MetadataChunk:
	var repositionMask : int # uint32
	var repositioningNode : int # int32
	var exporterMajorVersion : int # byte
	var exporterMinorVersion : int # byte
	var unused : Vector2i # 2x byte
	var retargetRootOffset : float
	var sourceApp : String
	var origFileName : String
	var exportDate : String
	var actorName : String

	func _init(
		repMask : int,
		repNode : int,
		exMajV : int,
		exMinV : int,
		un : Vector2i,
		retRootOff : float,
		sourceApp : String,
		origFile : String,
		date : String,
		actorName : String
	) -> void:
		self.repositionMask = repMask
		self.repositioningNode = repNode
		self.exporterMajorVersion = exMajV
		self.exporterMinorVersion = exMinV
		self.unused = un
		self.retargetRootOffset = retRootOff
		self.sourceApp = sourceApp
		self.origFileName = origFile
		self.exportDate = date
		self.actorName = actorName

	func debugPrint() -> void:
		print("Ver: %d.%d, exported: %s, fileName: %s, sourceApp: %s" %
			[exporterMajorVersion, exporterMinorVersion, exportDate, origFileName, sourceApp])
		print("Actor: %s, retargetRootOffset: %d, repositionMask: %d, repositionNode: %d" %
			[actorName, retargetRootOffset, repositionMask, repositioningNode])

# HIERARCHY

static func readNodeData(file : FileAccess) -> NodeData:
	return NodeData.new(
		FileAccessUtils.read_quat(file), FileAccessUtils.read_quat(file),
		FileAccessUtils.read_pos(file), FileAccessUtils.read_vec3(file),
		FileAccessUtils.read_vec3(file), # 3x unused floats
		FileAccessUtils.read_int32(file), FileAccessUtils.read_int32(file), FileAccessUtils.read_int32(file),
		FileAccessUtils.read_int32(file), FileAccessUtils.read_int32(file),
		FileAccessUtils.read_mat4x4(file), file.get_float(), FileAccessUtils.read_xac_str(file)
	)

class NodeData:
	var rotation : Quaternion
	var scaleRotation : Quaternion
	var position : Vector3
	var scale : Vector3
	var unused : Vector3 # 3x unused floats
	var unknown : int # int32
	var unknown2 : int # int32
	var parentNodeId : int # int32
	var numChildNodes : int # int32
	var bIncludeInBoundsCalc : bool # int32
	var transform : FileAccessUtils.xac_mat4x4
	var fImportanceFactor : float
	var name : String

	func _init(
		rot : Quaternion,
		scaleRot : Quaternion,
		pos : Vector3,
		scale : Vector3,
		unused : Vector3,
		unknown : int,
		unknown2 : int,
		parentNodeId : int,
		numChildNodes : int,
		incInBoundsCalc : bool,
		transform : FileAccessUtils.xac_mat4x4,
		fImportanceFactor : float,
		name : String
	) -> void:
		self.rotation = rot
		self.scaleRotation = scaleRot
		self.position = pos
		self.scale = scale
		self.unused = unused
		self.unknown = unknown2
		self.unknown2 = unknown2
		self.parentNodeId = parentNodeId
		self.numChildNodes = numChildNodes
		self.bIncludeInBoundsCalc = incInBoundsCalc
		self.transform = transform
		self.fImportanceFactor = fImportanceFactor
		self.name = name

	func debugPrint() -> void:
		print("\tparentNodeId: %d,\t numChildNodes: %d,\t Node Name: %s" % [parentNodeId, numChildNodes, name])
		print("\tunused %s,%s,%s, -1: %s, -1: %s" % [unused[0], unused[1], unused[2], unknown, unknown2])

static func readNodeHierarchyChunk(file : FileAccess) -> NodeHierarchyChunk:
	var numNodes : int = FileAccessUtils.read_int32(file)
	var numRootNodes : int = FileAccessUtils.read_int32(file)
	var nodes : Array[NodeData] = []
	for i : int in numNodes:
		nodes.push_back(readNodeData(file))
	return NodeHierarchyChunk.new(numNodes, numRootNodes, nodes)

class NodeHierarchyChunk:
	var numNodes : int # int32
	var numRootNodes : int # int32
	var nodes : Array[NodeData]

	func _init(numNodes : int, numRootNodes : int, nodes : Array[NodeData]) -> void:
		self.numNodes = numNodes
		self.numRootNodes = numRootNodes
		self.nodes = nodes

	func debugPrint() -> void:
		print("numNodes: %d, numRootNodes: %d" % [numNodes, numRootNodes])
		for node : NodeData in nodes:
			node.debugPrint()

# MATERIAL TOTALS

static func readMaterialTotalsChunk(file : FileAccess) -> MaterialTotalsChunk:
	return MaterialTotalsChunk.new(FileAccessUtils.read_int32(file), FileAccessUtils.read_int32(file), FileAccessUtils.read_int32(file))

class MaterialTotalsChunk:
	var numTotalMaterials : int # int32
	var numStandMaterials : int # int32
	var numFxMaterials : int # int32

	func _init(numTotalMaterials : int, numStandMaterials : int, numFxMaterials : int) -> void:
		self.numTotalMaterials = numTotalMaterials
		self.numStandMaterials = numStandMaterials
		self.numFxMaterials = numFxMaterials

	func debugPrint() -> void:
		print("totalMaterials: %d, standardMaterials: %d, fxMaterials: %d" %
			[numTotalMaterials, numStandMaterials, numFxMaterials])

# MATERIAL DEFINITION

static func readLayer(file : FileAccess, isV1 : bool) -> Layer:
	var unknown : Vector3i
	if isV1:
		unknown = Vector3i(
			FileAccessUtils.read_int32(file),
			FileAccessUtils.read_int32(file),
			FileAccessUtils.read_int32(file)
		)
	var layer : Layer = Layer.new(
		file.get_float(), file.get_float(), file.get_float(), file.get_float(),
		file.get_float(), file.get_float(), FileAccessUtils.read_int16(file),
		file.get_8(), file.get_8(), FileAccessUtils.read_xac_str(file), unknown
	)
	return layer

class Layer:
	var amount : float
	var uOffset : float
	var vOffset : float
	var uTiling : float
	var vTiling : float
	var rotInRad : float
	var matId : int # int16
	var mapType : int # byte
	var unused : int # byte
	var texture : String
	var unknown : Vector3i # Unknown 3 integers present in v1 of the chunk

	func _init(
		amount : float,
		uOffset : float,
		vOffset : float,
		uTiling : float,
		vTiling : float,
		rotInRad : float,
		matId : int,
		mapType : int,
		unused : int,
		texture : String,
		unknown : Vector3i
	) -> void:
		self.amount = amount
		self.uOffset = uOffset
		self.vOffset = vOffset
		self.uTiling = uTiling
		self.vTiling = vTiling
		self.rotInRad = rotInRad
		self.matId = matId
		self.mapType = mapType
		self.unused = unused
		self.texture = texture
		self.unknown = unknown

	func debugPrint() -> void:
		print("\tLayer MatId:%d,\t UVOffset:%d,%d,\t UVTiling %d,%d mapType: %d,\t Texture Name: %s" %
			[matId, uOffset, vOffset, uTiling, vTiling, mapType, texture])
		print("\t  amount:%s,\t rot:%s,\t unused:%d" % [amount, rotInRad, unused])

	func is_specular() -> bool:
		return mapType == 3

# TODO: Might want to change this from vec4d to colours where appropriate
static func readMaterialDefinitionChunk(file : FileAccess, isV1 : bool) -> MaterialDefinitionChunk:
	var chunk : MaterialDefinitionChunk = MaterialDefinitionChunk.new(
		FileAccessUtils.read_vec4(file), FileAccessUtils.read_vec4(file),
		FileAccessUtils.read_vec4(file), FileAccessUtils.read_vec4(file),
		file.get_float(), file.get_float(), file.get_float(), file.get_float(),
		file.get_8(), file.get_8(), file.get_8(), file.get_8(),
		FileAccessUtils.read_xac_str(file)
	)
	var layers : Array[Layer] = []
	for i : int in chunk.numLayers:
		layers.push_back(readLayer(file, isV1))
	chunk.setLayers(layers)
	return chunk

class MaterialDefinitionChunk:
	var ambientColor : Vector4
	var diffuseColor : Vector4
	var specularColor : Vector4
	var emissiveColor : Vector4
	var shine : float
	var shineStrength : float
	var opacity : float
	var ior : float
	var bDoubleSided : bool # byte
	var bWireframe : bool # byte
	var unused : int # 1 byte
	var numLayers : int # byte
	var name : String
	var Layers : Array[Layer]

	func _init(
		ambientColor : Vector4,
		diffuseColor : Vector4,
		specularColor : Vector4,
		emissiveColor : Vector4,
		shine : float,
		shineStrength : float,
		opacity : float,
		ior : float,
		bDoubleSided : bool,
		bWireframe : bool,
		unused : int,
		numLayers : int,
		name : String
	) -> void:
		self.ambientColor = ambientColor
		self.diffuseColor = diffuseColor
		self.specularColor = specularColor
		self.emissiveColor = emissiveColor
		self.shine = shine
		self.shineStrength = shineStrength
		self.opacity = opacity
		self.ior = ior
		self.bDoubleSided = bDoubleSided
		self.bWireframe = bWireframe
		self.unused = unused
		self.numLayers = numLayers
		self.name = name

	func setLayers(layers : Array[Layer]) -> void:
		self.Layers = layers

	# Specular textures are used for country-specific unit colours,
	# which need a UnitModel to be set through
	func has_specular() -> bool:
		for layer : Layer in Layers:
			if layer.is_specular():
				return true
		return false

	func debugPrint() -> void:
		print("Material Name: %s, num layers: %d, doubleSided %s" % [name, numLayers, bDoubleSided])
		print("\tshine:%s\tshineStrength:%s\t,opacity:%s,\tior:%s,\tunused:%s" % [shine, shineStrength, opacity, ior, unused])
		for layer : Layer in Layers:
			layer.debugPrint()

# MESH

static func readVerticesAttribute(file : FileAccess, numVerts : int) -> VerticesAttribute:
	var vertAttrib : VerticesAttribute = VerticesAttribute.new(
		FileAccessUtils.read_int32(file), FileAccessUtils.read_int32(file),
		file.get_8(), file.get_8(), Vector2i(file.get_8(), file.get_8()))
	var data : Variant
	match vertAttrib.type:
		0: # position
			data = PackedVector3Array()
			for i : int in numVerts:
				data.push_back(FileAccessUtils.read_pos(file))
		1: # normals
			data = PackedVector3Array()
			for i : int in numVerts:
				data.push_back(FileAccessUtils.read_pos(file))
		2: # tangents
			data = [] as Array[Vector4]
			for i : int in numVerts:
				var tangent : Vector4 = FileAccessUtils.read_vec4(file)
				#tangent.w *= -1
				data.push_back(tangent)
		3: # uvs
			data = PackedVector2Array()
			for i : int in numVerts:
				data.push_back(FileAccessUtils.read_vec2(file))
		4: # 32bit colors
			data = PackedColorArray()
			for i : int in numVerts:
				data.push_back(FileAccessUtils.read_Color32(file))
		5: # influence range indices
			data = PackedInt64Array()
			for i : int in numVerts:
				data.push_back(file.get_32())
		6: # 128bit colors
			data = PackedColorArray()
			for i : int in numVerts:
				data.push_back(FileAccessUtils.read_Color128(file))
		_:
			push_error("INVALID XAC VERTATTRIBUTE TYPE %d" % vertAttrib.type)
	vertAttrib.setData(data)
	return vertAttrib

# mesh has one of these for each vertex property (position, normals, etc)
class VerticesAttribute:
	var type : int # int32
	var attribSize : int # int32
	var bKeepOriginals : bool # byte
	var bIsScaleFactor : bool # byte
	var pad : Vector2i # 2x byte
	var data : Variant # numVerts * attribSize

	func _init(
		type : int,
		attribSize : int,
		bKeepOriginals : bool,
		bIsScaleFactor : bool,
		pad : Vector2i
	) -> void:
		self.type = type
		self.attribSize = attribSize
		self.bKeepOriginals = bKeepOriginals
		self.bIsScaleFactor = bIsScaleFactor
		self.pad = pad

	func setData(data : Variant) -> void:
		self.data = data

	func debugPrint() -> void:
		var typeStr : String
		match type:
			0:
				typeStr = "Positions"
			1:
				typeStr = "Normals"
			2:
				typeStr = "Tangents"
			3:
				typeStr = "UV Coords"
			4:
				typeStr = "32bit Colors"
			5:
				typeStr = "Influence Range Indices (u32)"
			6:
				typeStr = "128bit Colors"
			_:
				typeStr = "invalid type %d" % type
		print("\tattribSize:%d (bytes),\t keepOriginals:%s,\t isScaleFactor:%s,\t VertAttrib: type:%s" %
			[attribSize, bKeepOriginals, bIsScaleFactor, typeStr])
		print("\tpad: %s" % pad)

static func readSubMesh(file : FileAccess) -> SubMesh:
	var subMesh : SubMesh = SubMesh.new(
		FileAccessUtils.read_int32(file), FileAccessUtils.read_int32(file),
		FileAccessUtils.read_int32(file), FileAccessUtils.read_int32(file)
	)
	var relIndices : PackedInt32Array = PackedInt32Array()
	var boneIds : PackedInt32Array = PackedInt32Array()
	for i : int in subMesh.numIndices:
		relIndices.push_back(FileAccessUtils.read_int32(file))
	for i : int in subMesh.numBones:
		boneIds.push_back(FileAccessUtils.read_int32(file))
	subMesh.setBoneIds(boneIds)
	subMesh.setRelIndices(relIndices)
	return subMesh

class SubMesh:
	var numIndices : int # int32
	var numVertices : int # int32
	var materialId : int # int32
	var numBones : int # int32
	var relativeIndices : PackedInt32Array # int32 [numIndices]
	var boneIds : PackedInt32Array # int32 [numBones], unused

	func _init(numIndices : int, numVertices : int, materialId : int, numBones : int) -> void:
		self.numIndices = numIndices
		self.numVertices = numVertices
		self.materialId = materialId
		self.numBones = numBones

	func setRelIndices(relativeIndices : PackedInt32Array) -> void:
		self.relativeIndices = relativeIndices

	func setBoneIds(boneIds : PackedInt32Array) -> void:
		self.boneIds = boneIds

	func debugPrint() -> void:
		print("\tSubMesh:\t numIndices:%d,\t numVerts:%d,\t matId:%d,\t numBones:%d" %
			[numIndices, numVertices, materialId, numBones])

static func readMeshChunk(file : FileAccess) -> MeshChunk:
	var mesh : MeshChunk = MeshChunk.new(
		FileAccessUtils.read_int32(file), FileAccessUtils.read_int32(file),
		FileAccessUtils.read_int32(file), FileAccessUtils.read_int32(file),
		FileAccessUtils.read_int32(file), FileAccessUtils.read_int32(file), file.get_8(),
		Vector3i(file.get_8(), file.get_8(), file.get_8())
	)
	var vertAttribs : Array[VerticesAttribute] = []
	var submeshes : Array[SubMesh] = []
	for i : int in mesh.numAttribLayers:
		vertAttribs.push_back(readVerticesAttribute(file,mesh.numVertices))
	for i : int in mesh.numSubMeshes:
		submeshes.push_back(readSubMesh(file))
	mesh.setVerticesAttributes(vertAttribs)
	mesh.setSubMeshes(submeshes)
	return mesh

class MeshChunk:
	var nodeId : int # int32
	var numInfluenceRanges : int # int32
	var numVertices : int # int32
	var numIndices : int # int32
	var numSubMeshes : int # int32
	var numAttribLayers : int # int32
	var bIsCollisionMesh : bool # byte
	var pad : Vector3i # 3x byte
	var VerticesAttributes : Array[VerticesAttribute]
	var SubMeshes : Array[SubMesh]

	func _init(
		nodeId : int,
		numInfluenceRanges : int,
		numVertices : int,
		numIndices : int,
		numSubMeshes : int,
		numAttribLayers : int,
		bIsCollisionMesh : bool,
		pad : Vector3i
	) -> void:
		self.nodeId = nodeId
		self.numInfluenceRanges = numInfluenceRanges
		self.numVertices = numVertices
		self.numIndices = numIndices
		self.numSubMeshes = numSubMeshes
		self.numAttribLayers = numAttribLayers
		self.bIsCollisionMesh = bIsCollisionMesh
		self.pad = pad

	func setVerticesAttributes(VerticesAttributes : Array[VerticesAttribute]) -> void:
		self.VerticesAttributes = VerticesAttributes

	func setSubMeshes(SubMeshes : Array[SubMesh]) -> void:
		self.SubMeshes = SubMeshes

	func debugPrint() -> void:
		print("Mesh: nodeId:%d, numVerts:%d, numSubMeshes:%d, numVertAttribs:%d, isCollisionMesh:%s" %
			[nodeId, numVertices, numSubMeshes, numAttribLayers, bIsCollisionMesh])
		for vertAttrib in VerticesAttributes:
			vertAttrib.debugPrint()
		for subMesh : SubMesh in SubMeshes:
			subMesh.debugPrint()

# SKINNING
static func readInfluenceData(file : FileAccess) -> InfluenceData:
	return InfluenceData.new(file.get_float(), FileAccessUtils.read_int16(file), Vector2i(file.get_8(), file.get_8()))

class InfluenceData:
	var fWeight : float # (0..1)
	var boneId : int # int16
	var pad : Vector2i # 2x byte

	func _init(fWeight : float, boneId : int, pad : Vector2i) -> void:
		self.fWeight = fWeight
		self.boneId = boneId
		self.pad = pad

	func debugPrint() -> void:
		print("\tInfluenceData:\t boneId: %d,\t Weight: %s" % [boneId, fWeight])

# For some reason influenceRange isn't being loaded, needs investigation
# Weird data on the flag

static func readInfluenceRange(file : FileAccess) -> InfluenceRange:
	return InfluenceRange.new(FileAccessUtils.read_int32(file), FileAccessUtils.read_int32(file))

class InfluenceRange:
	var firstInfluenceIndex : int # int32
	var numInfluences : int # int32

	func _init(firstInfluenceIndex : int, numInfluences : int) -> void:
		self.firstInfluenceIndex = firstInfluenceIndex
		self.numInfluences = numInfluences

	func debugPrint() -> void:
		print("\tInfluenceRange:\t firstIndex: %d,\t numInfluences: %d" % [firstInfluenceIndex, numInfluences])

static func readSkinningChunk(file : FileAccess, meshChunks : Array[MeshChunk], isV2 : bool) -> SkinningChunk:
	var skinning : SkinningChunk = null
	skinning = SkinningChunk.new(
		FileAccessUtils.read_int32(file), -1 if isV2 else FileAccessUtils.read_int32(file),
		FileAccessUtils.read_int32(file), file.get_8(), Vector3i(file.get_8(), file.get_8(), file.get_8())
	)
	var influenceData : Array[InfluenceData] = []
	var influenceRange : Array[InfluenceRange] = []
	for i : int in skinning.numInfluences:
		influenceData.push_back(readInfluenceData(file))
	# search the list of mesh chunks for the one which matches IsCollisionMesh and NodeId?
	# documentation is a little unclear about this (lists the mesh accessing by node, which isn't possible)
	for chunk : MeshChunk in meshChunks:
		if chunk.nodeId == skinning.nodeId and chunk.bIsCollisionMesh == skinning.bIsForCollisionMesh:
			for i : int in chunk.numInfluenceRanges:
				influenceRange.push_back(readInfluenceRange(file))
			break
	skinning.setInfluenceData(influenceData)
	skinning.setInfluenceRange(influenceRange)
	return skinning

class SkinningChunk:
	var nodeId : int # int32
	var numLocalBones : int # int32, of bones in the influence data
	var numInfluences : int # int32
	var bIsForCollisionMesh : bool # byte boolean
	var pad : Vector3i # 3x pad
	var influenceData : Array[InfluenceData]
	var influenceRange : Array[InfluenceRange]

	func _init(
		nodeId : int,
		numLocalBones : int,
		numInfluences : int,
		bIsForCollisionMesh : bool,
		pad : Vector3i
	) -> void:
		self.nodeId = nodeId
		self.numLocalBones = numLocalBones
		self.numInfluences = numInfluences
		self.bIsForCollisionMesh = bIsForCollisionMesh
		self.pad = pad

	func setInfluenceData(influenceData : Array[InfluenceData]) -> void:
		self.influenceData = influenceData

	func setInfluenceRange(influenceRange : Array[InfluenceRange]) -> void:
		self.influenceRange = influenceRange

	func debugPrint() -> void:
		print("Skinning: nodeId:%d, numInfluencedBones: %d, numInfluences: %d, CollisionMesh?: %d" %
			[nodeId, numLocalBones, numInfluences, bIsForCollisionMesh])
		for infDat : InfluenceData in influenceData:
			infDat.debugPrint()
		for infRange : InfluenceRange in influenceRange:
			infRange.debugPrint()

# TODO: What is chunk type6, figure out what the plausible datatypes are
# Currently, since XACs tend to use int32s and float (32bit) + strings, and no strings
# are evident in chunk type 6, to load these as arrays of int32

static func readChunkType6(file : FileAccess) -> ChunkType6:
	var intArr : PackedInt32Array = []
	var floatArr : PackedFloat32Array = []
	for i : int in 12:
		intArr.push_back(FileAccessUtils.read_int32(file))
	for i : int in 9:
		floatArr.push_back(file.get_float())
	return ChunkType6.new(intArr, floatArr, FileAccessUtils.read_int32(file))

class ChunkType6:
	var unknown : PackedInt32Array
	var unknown_floats : PackedFloat32Array
	var maybe_node_id : int

	func _init(unknown : PackedInt32Array, unknown_floats : PackedFloat32Array, maybe_node_id : int) -> void:
		self.unknown = unknown
		self.unknown_floats = unknown_floats
		self.maybe_node_id = maybe_node_id

	func debugPrint() -> void:
		print("\tUnknown: %s\n\tFloats: %s\n\tPerhaps NodeId? %s" % [self.unknown, self.unknown_floats, self.maybe_node_id])

# Chunk type 0x0
static func readNodeChunk(file : FileAccess) -> NodeChunk:
	return NodeChunk.new(
		FileAccessUtils.read_quat(file), FileAccessUtils.read_quat(file),
		FileAccessUtils.read_pos(file), FileAccessUtils.read_vec3(file),
		FileAccessUtils.read_vec3(file), # unused 3 floats
		FileAccessUtils.read_int32(file), FileAccessUtils.read_int32(file), #-1-1
		# 17 x 32bit unknowns, likely matrix and some other info
		# last 32bit unknown is likely fImportanceFactor, (float 1f). Rest is likely matrix
		(func() -> PackedInt32Array:
			var array : PackedInt32Array = PackedInt32Array()
			for i : int in 17:
				array.push_back(FileAccessUtils.read_int32(file))
			return array).call(),
		FileAccessUtils.read_xac_str(file)
	)

class NodeChunk:
	var rotation : Quaternion
	var scaleRotation : Quaternion
	var position : Vector3
	var scale : Vector3
	var unused : Vector3 # 3x unused floats
	var unknown : int # int32
	var parentBone : int # int32
	var unknown2 : PackedInt32Array # 17x int32 sized numbers
	var name : String

	func _init(
		rot : Quaternion,
		scaleRot : Quaternion,
		pos : Vector3,
		scale : Vector3,
		unused : Vector3,
		unknown : int,
		parentBone : int,
		unknown2 : PackedInt32Array,
		name : String
	) -> void:
		self.rotation = rot
		self.scaleRotation = scaleRot
		self.position = pos
		self.scale = scale
		self.unused = unused
		self.unknown = unknown
		self.parentBone = parentBone
		self.unknown2 = unknown2
		self.name = name

	func debugPrint() -> void:
		print("\tNode Name: %s, parentBone %s" % [name, parentBone])
		print("\tunused (%s) unknown: %s" % [unused, unknown])
		print("\tunknown after -1-1: %s" % unknown2)

static func readChunkTypeUnknown(file : FileAccess, length : int) -> ChunkTypeUnknown:
	return ChunkTypeUnknown.new(file.get_buffer(length))

class ChunkTypeUnknown:
	var unknown : PackedByteArray

	func _init(unknown : PackedByteArray) -> void:
		self.unknown = unknown

	func debugPrint() -> void:
		print("\tUnknown: %s" % self.unknown)

# TODO: MorphTarget, Deformation, Transformation, MorphTargetsChunk
