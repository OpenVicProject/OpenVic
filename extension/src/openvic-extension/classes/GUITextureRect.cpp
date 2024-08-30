#include "GUITextureRect.hpp"

using namespace godot;
using namespace OpenVic;

GUI_TOOLTIP_IMPLEMENTATIONS(GUITextureRect)

void GUITextureRect::_bind_methods() {
	GUI_TOOLTIP_BIND_METHODS(GUITextureRect)
}

void GUITextureRect::_notification(int what) {
	_tooltip_notification(what);
}

GUITextureRect::GUITextureRect() : tooltip_active { false } {}
