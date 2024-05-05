class_name FileAccessUtils

static func read_vec2(file : FileAccess) -> Vector2:
	return Vector2(file.get_float(), file.get_float())

static func read_vec3(file : FileAccess) -> Vector3:
	return Vector3(file.get_float(), file.get_float(), file.get_float())

static func read_pos(file : FileAccess) -> Vector3:
	var pos : Vector3 = read_vec3(file)
	pos.x = -pos.x
	return pos

static func read_vec4(file : FileAccess) -> Vector4:
	return Vector4(file.get_float(), file.get_float(), file.get_float(), file.get_float())

# Because paradox may or may not be consistent with the xsm spec depending on if its Tuesday or not
static func read_quat(file : FileAccess, int16 : bool = false) -> Quaternion:
	if int16:
		return Quaternion(read_f16(file), -read_f16(file), -read_f16(file), read_f16(file))
	else:
		return Quaternion(file.get_float(), -file.get_float(), -file.get_float(), file.get_float())

static func read_f16(file : FileAccess) -> float:
	# 32767 or 0x7FFF is the max magnitude of a signed int16
	return float(read_int16(file)) / 32767.0

static func replace_chars(string : String) -> String:
	return string.replace(":", "_").replace("\\", "_").replace("/", "_")

static func read_xac_str(file : FileAccess) -> String:
	var length : int = file.get_32()
	var buffer : PackedByteArray = file.get_buffer(length)
	return buffer.get_string_from_ascii()

static func read_int32(file : FileAccess) -> int:
	var bytes : int = file.get_32()
	var negative : bool = bytes >> 31
	var val : int = bytes & 0x7FFFFFFF
	if negative:
		val = -((val ^ 0x7FFFFFFF) + 1)
	return val

static func read_int16(file : FileAccess) -> int:
	var bytes : int = file.get_16()
	var negative : bool = bytes >> 15
	var val : int = bytes & 0x7FFF
	if negative:
		val = -((val ^ 0x7FFF) + 1)
	return val

static func read_Color32(file : FileAccess) -> Color:
	return Color8(file.get_8(), file.get_8(), file.get_8(), file.get_8())

static func read_Color128(file : FileAccess) -> Color:
	return Color(
		file.get_32() / 0xFFFFFFFF,
		file.get_32() / 0xFFFFFFFF,
		file.get_32() / 0xFFFFFFFF,
		file.get_32() / 0xFFFFFFFF
	)

static func read_mat4x4(file : FileAccess) -> xac_mat4x4:
	return xac_mat4x4.new(read_vec4(file), read_vec4(file), read_vec4(file), read_vec4(file))

# This datatype is only ever used to hold a transform for nodes (bones)
class xac_mat4x4:
	var col1 : Vector4
	var col2 : Vector4
	var col3 : Vector4
	var col4 : Vector4

	func _init(col1 : Vector4, col2 : Vector4, col3 : Vector4, col4 : Vector4) -> void:
		self.col1 = col1
		self.col2 = col2
		self.col3 = col3
		self.col4 = col4

	func debugPrint() -> void:
		print("\t\tMat4x4 col1:", col1, " col2:", col2, " col3:", col3, " col4:", col4)

	func getAsTransform() -> Transform3D: # godot wants 3x4 matrix
		return Transform3D(
			Vector3(col1.x, col1.y, col1.z),
			Vector3(col2.x, col2.y, col2.z),
			Vector3(col3.x, col3.y, col3.z),
			Vector3(col4.x, col4.y, col4.z)
		)
