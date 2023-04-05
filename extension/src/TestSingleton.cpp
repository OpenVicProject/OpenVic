#include "TestSingleton.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace OpenVic2;

TestSingleton* TestSingleton::singleton = nullptr;

void TestSingleton::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("hello_singleton"), &TestSingleton::hello_singleton);
}

TestSingleton* TestSingleton::get_singleton()
{
	return singleton;
}

TestSingleton::TestSingleton()
{
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

TestSingleton::~TestSingleton()
{
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

void TestSingleton::hello_singleton()
{
	UtilityFunctions::print("Hello GDExtension Singleton!");
}