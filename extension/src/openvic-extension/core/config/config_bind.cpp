#include "config_bind.hpp"

#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/core/config/OVDataloader.hpp"
#include "openvic-extension/core/config/OVGame.hpp"
#include "openvic-extension/core/config/OVLexyVDF.hpp"
#include "openvic-extension/core/config/OVSimulation.hpp"

using namespace OpenVic::CoreBind;
using namespace godot;

OVGame::OVGame() {
	singleton = this;
}

OVGame::~OVGame() {
	singleton = nullptr;
}

TypedDictionary<String, PackedStringArray> OVGame::get_author_info() const {
	return OpenVic::OVGame::get_author_info();
}

TypedArray<Dictionary> OVGame::get_copyright_info() const {
	return OpenVic::OVGame::get_copyright_info();
}

TypedDictionary<String, String> OVGame::get_license_info() const {
	return OpenVic::OVGame::get_license_info();
}

String OVGame::get_license_text() const {
	return OpenVic::OVGame::get_license_text();
}

Dictionary OVGame::get_version_info() const {
	return OpenVic::OVGame::get_version_info();
}

void OVGame::_bind_methods() {
	OV_BIND_METHOD(OVGame::get_author_info);
	OV_BIND_METHOD(OVGame::get_copyright_info);
	OV_BIND_METHOD(OVGame::get_license_info);
	OV_BIND_METHOD(OVGame::get_license_text);
	OV_BIND_METHOD(OVGame::get_version_info);
}

OVSimulation::OVSimulation() {
	singleton = this;
}

OVSimulation::~OVSimulation() {
	singleton = nullptr;
}

TypedDictionary<String, PackedStringArray> OVSimulation::get_author_info() const {
	return OpenVic::OVSimulation::get_author_info();
}

TypedArray<Dictionary> OVSimulation::get_copyright_info() const {
	return OpenVic::OVSimulation::get_copyright_info();
}

TypedDictionary<String, String> OVSimulation::get_license_info() const {
	return OpenVic::OVSimulation::get_license_info();
}

String OVSimulation::get_license_text() const {
	return OpenVic::OVSimulation::get_license_text();
}

Dictionary OVSimulation::get_version_info() const {
	return OpenVic::OVSimulation::get_version_info();
}

void OVSimulation::_bind_methods() {
	OV_BIND_METHOD(OVSimulation::get_author_info);
	OV_BIND_METHOD(OVSimulation::get_copyright_info);
	OV_BIND_METHOD(OVSimulation::get_license_info);
	OV_BIND_METHOD(OVSimulation::get_license_text);
	OV_BIND_METHOD(OVSimulation::get_version_info);
}

OVDataloader::OVDataloader() {
	singleton = this;
}

OVDataloader::~OVDataloader() {
	singleton = nullptr;
}

TypedDictionary<String, PackedStringArray> OVDataloader::get_author_info() const {
	return OpenVic::OVDataloader::get_author_info();
}

TypedArray<Dictionary> OVDataloader::get_copyright_info() const {
	return OpenVic::OVDataloader::get_copyright_info();
}

TypedDictionary<String, String> OVDataloader::get_license_info() const {
	return OpenVic::OVDataloader::get_license_info();
}

String OVDataloader::get_license_text() const {
	return OpenVic::OVDataloader::get_license_text();
}

Dictionary OVDataloader::get_version_info() const {
	return OpenVic::OVDataloader::get_version_info();
}

void OVDataloader::_bind_methods() {
	OV_BIND_METHOD(OVDataloader::get_author_info);
	OV_BIND_METHOD(OVDataloader::get_copyright_info);
	OV_BIND_METHOD(OVDataloader::get_license_info);
	OV_BIND_METHOD(OVDataloader::get_license_text);
	OV_BIND_METHOD(OVDataloader::get_version_info);
}

OVLexyVDF::OVLexyVDF() {
	singleton = this;
}

OVLexyVDF::~OVLexyVDF() {
	singleton = nullptr;
}

TypedDictionary<String, PackedStringArray> OVLexyVDF::get_author_info() const {
	return OpenVic::OVLexyVDF::get_author_info();
}

TypedArray<Dictionary> OVLexyVDF::get_copyright_info() const {
	return OpenVic::OVLexyVDF::get_copyright_info();
}

TypedDictionary<String, String> OVLexyVDF::get_license_info() const {
	return OpenVic::OVLexyVDF::get_license_info();
}

String OVLexyVDF::get_license_text() const {
	return OpenVic::OVLexyVDF::get_license_text();
}

Dictionary OVLexyVDF::get_version_info() const {
	return OpenVic::OVLexyVDF::get_version_info();
}

void OVLexyVDF::_bind_methods() {
	OV_BIND_METHOD(OVLexyVDF::get_author_info);
	OV_BIND_METHOD(OVLexyVDF::get_copyright_info);
	OV_BIND_METHOD(OVLexyVDF::get_license_info);
	OV_BIND_METHOD(OVLexyVDF::get_license_text);
	OV_BIND_METHOD(OVLexyVDF::get_version_info);
}
