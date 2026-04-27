#include "../include/astar_cpu_wrapper.h"
#include "../include/run_cpu_astar.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/binder_common.hpp>

using namespace godot;

void RunWrapper::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("run", "path"), &RunWrapper::run);
}

#include <fstream>
#include <godot_cpp/variant/utility_functions.hpp>

void RunWrapper::run(String path_in, String path_out)
{
    godot::UtilityFunctions::print("Before C run");

    // TEMPORARILY COMMENT THIS OUT
    CharString utf8_path_in = path_in.utf8();
    CharString utf8_path_out = path_out.utf8();
    ::run(utf8_path_in.get_data(), utf8_path_out.get_data());

    godot::UtilityFunctions::print("After C run");
}