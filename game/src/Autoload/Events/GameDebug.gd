extends RefCounted

# REQUIREMENTS:
# * SS-56
func _init():
	for engine_args in OS.get_cmdline_args():
		match(engine_args):
			"--game-debug":
				set_debug_mode(true)

	for engine_args in OS.get_cmdline_user_args():
		match(engine_args):
			"--game-debug", "-d", "--debug", "--debug-mode":
				set_debug_mode(true)

func set_debug_mode(value : bool) -> void:
	ProjectSettings.set_setting("openvic2/debug/enabled", value)
	print("Set debug mode to: ", value)

func is_debug_mode() -> bool:
	return ProjectSettings.get_setting("openvic2/debug/enabled", false)
