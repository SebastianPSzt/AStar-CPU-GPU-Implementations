env = Environment()

env.Append(CXXFLAGS=["/std:c++17"])
env.Append(CPPDEFINES=["TYPED_METHOD_BIND"])

env.Append(CPPPATH=[
    "include",
    "C:/Users/thysv/source/godot-cpp/include",
    "C:/Users/thysv/source/godot-cpp/gen/include",
    "C:/Users/thysv/source/godot-cpp/gdextension",
])

env.Append(LIBPATH=[
    "C:/Users/thysv/source/godot-cpp/bin"
])

env.Append(LIBS=[
    "libgodot-cpp.windows.template_debug.x86_64.lib"
])

sources = [
    "src/astar_cpu_wrapper.cpp",
    "src/run_cpu_astar.c",
    "src/a_star_cpu.c",
    "src/search_log_cpu.c",
    "ds/bmh.c",
    "ds/grid.c",
    "include/cJSON.c",
    "godot_cpp/register_types.cpp"
]

env.SharedLibrary(
    target="godot/bin/astar_cpu_wrapper",
    source=sources
)