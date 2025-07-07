class_name XSMLoader

# Keys: source_file (String)
# Values: loaded animation (Animation) or LOAD_FAILED_MARKER (StringName)
static var xsm_cache : Dictionary

const LOAD_FAILED_MARKER : StringName = &"XSM LOAD FAILED"

static func get_xsm_animation(source_file : String) -> Animation:
	var cached : Variant = xsm_cache.get(source_file)
	if not cached:
		cached = _load_xsm_animation(source_file)
		if cached:
			xsm_cache[source_file] = cached
		else:
			xsm_cache[source_file] = LOAD_FAILED_MARKER
			push_error("Failed to get XSM model \"", source_file, "\" (current load failed)")
			return null

	if not cached is Animation:
		push_error("Failed to get XSM model \"", source_file, "\" (previous load failed)")
		return null

	return cached

const SKELETON_PATH : String = "./skeleton:%s"

static func _load_xsm_animation(source_file : String) -> Animation:
	var source_path : String = GameSingleton.lookup_file_path(source_file)
	var file : FileAccess = FileAccess.open(source_path, FileAccess.READ)
	if file == null:
		push_error("Failed to load XSM ", source_file, " from looked up path ", source_path)
		return null

	readHeader(file)

	var metadataChunk : MetadataChunk = null
	var boneAnimationChunks : Array[BoneAnimationChunk] = []

	while file.get_position() < file.get_length():
		var type : int = FileAccessUtils.read_int32(file)
		var length : int = FileAccessUtils.read_int32(file)
		var version : int = FileAccessUtils.read_int32(file)

		match type:
			0xC9: #Metadata
				metadataChunk = readMetadataChunk(file)
			0xCA: #Bone Animation
				# v1 is float32 quaternions, v2 is int16 quaternions
				boneAnimationChunks.push_back(readBoneAnimationChunk(file, version == 2))
			_:
				push_error(">> INVALID XSM CHUNK TYPE %X" % type)
				break

	var animLength : float = 0.0
	var anim : Animation = Animation.new()
	for anim_Chunk : BoneAnimationChunk in boneAnimationChunks:
		for submotion : SkeletalSubMotion in anim_Chunk.SkeletalSubMotions:
			# NOTE: godot uses ':' to specify properties, so we replace such characters with '_'
			var skeleton_path : String = SKELETON_PATH % FileAccessUtils.replace_chars(submotion.nodeName)

			if submotion.numPosKeys > 0:
				var id : int = anim.add_track(Animation.TYPE_POSITION_3D)
				anim.track_set_path(id, skeleton_path)
				for key : PosKey in submotion.PosKeys:
					anim.position_track_insert_key(id, key.fTime, key.pos)
					if key.fTime > animLength:
						animLength = key.fTime
			else: # EXPERIMENTAL: see if setting posePos fixes idle3
				var id : int = anim.add_track(Animation.TYPE_POSITION_3D)
				anim.track_set_path(id, skeleton_path)
				anim.position_track_insert_key(id, 0, submotion.posePos)

			if submotion.numRotKeys > 0:
				var id : int = anim.add_track(Animation.TYPE_ROTATION_3D)
				anim.track_set_path(id, skeleton_path)
				for key : RotKey in submotion.RotKeys:
					anim.rotation_track_insert_key(id, key.fTime, key.rot)
					if key.fTime > animLength:
						animLength = key.fTime
			else: # EXPERIMENTAL: see if setting posePos fixes idle3
				var id : int = anim.add_track(Animation.TYPE_ROTATION_3D)
				anim.track_set_path(id, skeleton_path)
				anim.rotation_track_insert_key(id, 0, submotion.poseRot)

			if submotion.numScaleKeys > 0:
				var id : int = anim.add_track(Animation.TYPE_SCALE_3D)
				anim.track_set_path(id, skeleton_path)
				for key : ScaleKey in submotion.ScaleKeys:
					anim.scale_track_insert_key(id, key.fTime, key.scale)
					if key.fTime > animLength:
						animLength = key.fTime

			# TODO: submotion.numScaleRotKeys

	anim.length = animLength
	anim.loop_mode = Animation.LOOP_LINEAR

	xsm_cache[source_file] = anim
	return anim

static func readHeader(file : FileAccess) -> void:
	var magic_bytes : PackedByteArray = [file.get_8(), file.get_8(), file.get_8(), file.get_8()]
	var magic : String = magic_bytes.get_string_from_ascii()
	var version : String = "%d.%d" % [file.get_8(), file.get_8()]
	var bBigEndian : bool = file.get_8()
	var pad : int = file.get_8()
	#print(magic, ", version: ", version, ", bigEndian: ", bBigEndian, " pad: ", pad)

# NOTE: the "pad" variable is actually very important!
# It seems to have something to do with whether paradox uses int16 or int32
# for quaternions (it's "pad" or version number, can't tell)

static func readMetadataChunk(file : FileAccess) -> MetadataChunk:
	return MetadataChunk.new(
		file.get_float(), file.get_float(), FileAccessUtils.read_int32(file),
		file.get_8(), file.get_8(), file.get_16(),
		FileAccessUtils.read_xac_str(file), FileAccessUtils.read_xac_str(file), FileAccessUtils.read_xac_str(file), FileAccessUtils.read_xac_str(file)
	)

class MetadataChunk:
	var unused : float
	var fMaxAcceptableError : float
	var fps : int # int32
	var exporterMajorVersion : int # byte
	var exporterMinorVersion : int # byte
	var pad : int # 2x byte
	var sourceApp : String
	var origFileName : String
	var exportDate : String
	var motionName : String

	func _init(
		unused : float,
		fMaxAcceptableError : float,
		fps : int,
		exporterMajorVersion : int,
		exporterMinorVersion : int,
		pad : int,
		sourceApp : String,
		origFileName : String,
		exportDate : String,
		motionName : String
	) -> void:
		self.unused = unused
		self.fMaxAcceptableError = fMaxAcceptableError
		self.fps = fps
		self.exporterMajorVersion = exporterMajorVersion
		self.exporterMinorVersion = exporterMinorVersion
		self.pad = pad
		self.sourceApp = sourceApp
		self.origFileName = origFileName
		self.exportDate = exportDate
		self.motionName = motionName

	func debugPrint() -> void:
		print("FileName: %s, sourceApp: %s, exportDate: %s, ExporterV:%d.%d" %
			[origFileName, sourceApp, exportDate, exporterMajorVersion, exporterMinorVersion])
		print("MotionName: %s, fps: %d, MaxError: %s" %
			[motionName, fps, fMaxAcceptableError])

static func readPosKey(file : FileAccess) -> PosKey:
	return PosKey.new(FileAccessUtils.read_pos(file), file.get_float())

class PosKey:
	var pos : Vector3
	var fTime : float

	func _init(pos : Vector3, fTime : float) -> void:
		self.pos = pos
		self.fTime = fTime

	func debugPrint() -> void:
		print("\t\tPos:%s, time:%s" % [pos, fTime])

static func readRotKey(file : FileAccess, use_quat16 : bool) -> RotKey:
	return RotKey.new(FileAccessUtils.read_quat(file, use_quat16), file.get_float())

class RotKey:
	var rot : Quaternion
	var fTime : float

	func _init(rot : Quaternion, fTime : float) -> void:
		self.rot = rot
		self.fTime = fTime

	func debugPrint() -> void:
		print("\t\tRot:%s, time:%s" % [rot, fTime])

static func readScaleKey(file : FileAccess) -> ScaleKey:
	return ScaleKey.new(FileAccessUtils.read_vec3(file), file.get_float())

class ScaleKey:
	var scale : Vector3
	var fTime : float

	func _init(scale : Vector3, fTime : float) -> void:
		self.scale = scale
		self.fTime = fTime

	func debugPrint() -> void:
		print("\t\tScale:%s, time:%s" % [scale, fTime])

static func readScaleRotKey(file : FileAccess, use_quat16 : bool) -> ScaleRotKey:
	return ScaleRotKey.new(FileAccessUtils.read_quat(file, use_quat16), file.get_float())

class ScaleRotKey:
	var rot : Quaternion
	var fTime : float

	func _init(rot : Quaternion, fTime : float) -> void:
		self.rot = rot
		self.fTime = fTime

	func debugPrint() -> void:
		print("\t\tScaleRot:%s, time:%s" % [rot, fTime])

static func readSkeletalSubMotion(file : FileAccess, use_quat16 : bool) -> SkeletalSubMotion:
	var a : Quaternion = FileAccessUtils.read_quat(file, use_quat16)
	var b : Quaternion = FileAccessUtils.read_quat(file, use_quat16)
	var c : Quaternion = FileAccessUtils.read_quat(file, use_quat16)
	var d : Quaternion = FileAccessUtils.read_quat(file, use_quat16)

	var e : Vector3 = FileAccessUtils.read_pos(file)
	var f : Vector3 = FileAccessUtils.read_vec3(file)
	var g : Vector3 = FileAccessUtils.read_pos(file)
	var h : Vector3 = FileAccessUtils.read_vec3(file)

	var p : int = FileAccessUtils.read_int32(file)
	var j : int = FileAccessUtils.read_int32(file)
	var k : int = FileAccessUtils.read_int32(file)
	var l : int = FileAccessUtils.read_int32(file)

	var m : float = file.get_float()
	var n : String = FileAccessUtils.read_xac_str(file)

	var submotion : SkeletalSubMotion = SkeletalSubMotion.new(
		a, b, c, d, # quats
		e, f, g, h, # vec3
		p, j, k, l, # ints
		m, n
	)
	var poskeys : Array[PosKey] = []
	var rotkeys : Array[RotKey] = []
	var scalekeys : Array[ScaleKey] = []
	var scalerotkeys : Array[ScaleRotKey] = []
	#FIXME: Did paradox store the number of pos keys as a float instead of int?

	for i : int in submotion.numPosKeys:
		poskeys.push_back(readPosKey(file))
	for i : int in submotion.numRotKeys:
		rotkeys.push_back(readRotKey(file, use_quat16))
	for i : int in submotion.numScaleKeys:
		scalekeys.push_back(readScaleKey(file))
	for i : int in submotion.numScaleRotKeys:
		scalerotkeys.push_back(readScaleRotKey(file, use_quat16))
	submotion.setPosKeys(poskeys)
	submotion.setRotKeys(rotkeys)
	submotion.setScaleKeys(scalekeys)
	submotion.setScaleRotKeys(scalerotkeys)
	return submotion

class SkeletalSubMotion:
	var poseRot : Quaternion
	var bindPoseRot : Quaternion
	var poseScaleRot : Quaternion
	var bindPoseScaleRot : Quaternion
	var posePos : Vector3
	var poseScale : Vector3
	var bindPosePos : Vector3
	var bindPoseScale : Vector3
	var numPosKeys : int # int32
	var numRotKeys : int # int32
	var numScaleKeys : int # int32
	var numScaleRotKeys : int # int32
	var fMaxError : float
	var nodeName : String

	var PosKeys : Array[PosKey]
	var RotKeys : Array[RotKey]
	var ScaleKeys : Array[ScaleKey]
	var ScaleRotKeys : Array[ScaleRotKey]

	func _init(
		poseRot : Quaternion,
		bindPoseRot : Quaternion,
		poseScaleRot : Quaternion,
		bindPoseScaleRot : Quaternion,
		posePos : Vector3,
		poseScale : Vector3,
		bindPosePos : Vector3,
		bindPoseScale : Vector3,
		numPosKeys : int,
		numRotKeys : int,
		numScaleKeys : int,
		numScaleRotKeys : int,
		fMaxError : float,
		nodeName : String
	) -> void:
		self.poseRot = poseRot
		self.bindPoseRot = bindPoseRot
		self.poseScaleRot = poseScaleRot
		self.bindPoseScaleRot = bindPoseScaleRot
		self.posePos = posePos
		self.poseScale = poseScale
		self.bindPosePos = bindPosePos
		self.bindPoseScale = bindPoseScale
		self.numPosKeys = numPosKeys
		self.numRotKeys = numRotKeys
		self.numScaleKeys = numScaleKeys
		self.numScaleRotKeys = numScaleRotKeys
		self.fMaxError = fMaxError
		self.nodeName = nodeName

	func setPosKeys(PosKeys : Array[PosKey]) -> void:
		self.PosKeys = PosKeys

	func setRotKeys(RotKeys : Array[RotKey]) -> void:
		self.RotKeys = RotKeys

	func setScaleKeys(ScaleKeys : Array[ScaleKey]) -> void:
		self.ScaleKeys = ScaleKeys

	func setScaleRotKeys(ScaleRotKeys : Array[ScaleRotKey]) -> void:
		self.ScaleRotKeys = ScaleRotKeys

	func debugPrint() -> void:
		print("Node: %s, #PosKeys %d, #RotKeys %d, #ScaleKeys %d, #ScaleRotKeys %d" % [nodeName, numPosKeys, numRotKeys, numScaleKeys, numScaleRotKeys])
		print("\tposeScaleRot %s,\tbindPoseScaleRot %s,\tposeScale %s,\tbindPoseScale %s" % [poseScaleRot, bindPoseScaleRot, poseScale, bindPoseScale])
		for key : PosKey in PosKeys:
			key.debugPrint()
		for key : RotKey in RotKeys:
			key.debugPrint()
		for key : ScaleKey in ScaleKeys:
			key.debugPrint()
		for key : ScaleRotKey in ScaleRotKeys:
			key.debugPrint()

static func readBoneAnimationChunk(file : FileAccess, use_quat16 : bool) -> BoneAnimationChunk:
	var numSubMotions : int = FileAccessUtils.read_int32(file)
	var animChunk : BoneAnimationChunk = BoneAnimationChunk.new(numSubMotions)
	var submotions : Array[SkeletalSubMotion] = []
	for i : int in animChunk.numSubMotions:
		submotions.push_back(readSkeletalSubMotion(file, use_quat16))
	animChunk.setSkeletalSubMotions(submotions)
	return animChunk

class BoneAnimationChunk:
	var numSubMotions : int # int32
	var SkeletalSubMotions : Array[SkeletalSubMotion]

	func _init(numSubMotions : int) -> void:
		self.numSubMotions = numSubMotions

	func setSkeletalSubMotions(SkeletalSubMotions : Array[SkeletalSubMotion]) -> void:
		self.SkeletalSubMotions = SkeletalSubMotions

	func debugPrint() -> void:
		print("Number of Submotions: %d" % numSubMotions)
		for submotion : SkeletalSubMotion in SkeletalSubMotions:
			submotion.debugPrint()
