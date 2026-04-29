#include "../include/a_star_gpu4_wrapper.h"
#include "../include/run_gpu4_a_star.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void RunGPU4Wrapper::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("run", "path_in", "path_out"), &RunGPU4Wrapper::run);
}

void RunGPU4Wrapper::run(String path_in, String path_out)
{
    UtilityFunctions::print("Starting A* run...");
    UtilityFunctions::print("Input: ", path_in);
    UtilityFunctions::print("Output: ", path_out);

    CharString utf8_path_in = path_in.utf8();
    CharString utf8_path_out = path_out.utf8();

    ::run_gpu4(utf8_path_in.get_data(), utf8_path_out.get_data());

    UtilityFunctions::print("Finished A* run.");
}