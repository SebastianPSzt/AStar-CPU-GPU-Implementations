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
    "ds/bmh.cpp",
    "ds/grid.cpp",
    "ds/pq.cpp",

    "helpers/a_star_cpu_wrapper.cpp",
    "helpers/a_star_output.cpp",
    "helpers/cJSON.c",
    "helpers/run_cpu_a_star.cpp",
    "helpers/register_types.cpp",

    "src/a_star_cpu.cpp",
]

env.SharedLibrary(
    target="godot/bin/a_star_cpu_wrapper",
    source=sources
)