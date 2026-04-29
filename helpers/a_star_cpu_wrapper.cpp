#include "../include/a_star_cpu_wrapper.h"
#include "../include/run_cpu_a_star.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void RunWrapper::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("run", "path_in", "path_out"), &RunWrapper::run);
}

void RunWrapper::run(String path_in, String path_out)
{
    UtilityFunctions::print("Starting A* run...");
    UtilityFunctions::print("Input: ", path_in);
    UtilityFunctions::print("Output: ", path_out);

    CharString utf8_path_in = path_in.utf8();
    CharString utf8_path_out = path_out.utf8();

    ::run(utf8_path_in.get_data(), utf8_path_out.get_data());

    UtilityFunctions::print("Finished A* run.");
}