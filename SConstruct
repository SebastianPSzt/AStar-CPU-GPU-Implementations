CUDA_PATH = r"C:\PROGRA~1\NVIDIA~2\CUDA\v13.2"
NVCC = CUDA_PATH + r"\bin\nvcc.exe"

env = Environment()

# ---------- Compiler flags ----------
env.Append(CXXFLAGS=["/std:c++17"])
env.Append(CPPDEFINES=["TYPED_METHOD_BIND"])

# ---------- Include paths ----------
env.Append(CPPPATH=[
    "include",
    CUDA_PATH + r"\include",
    "C:/Users/thysv/source/godot-cpp/include",
    "C:/Users/thysv/source/godot-cpp/gen/include",
    "C:/Users/thysv/source/godot-cpp/gdextension",
])

# ---------- Libraries ----------
env.Append(LIBPATH=[
    CUDA_PATH + r"\lib\x64",
    "C:/Users/thysv/source/godot-cpp/bin",
])

env.Append(LIBS=[
    "cudart",
    "libgodot-cpp.windows.template_debug.x86_64.lib",
])

# ---------- CPU + shared C++ sources ----------
cpp_sources = [
    "ds/bmh.cpp",
    "ds/grid.cpp",
    "ds/pq.cpp",

    # BOTH wrappers
    "helpers/a_star_cpu_wrapper.cpp",
    "helpers/a_star_gpu4_wrapper.cpp",

    # shared helpers
    "helpers/a_star_output.cpp",
    "helpers/cJSON.c",
    "helpers/register_types.cpp",

    # BOTH run implementations
    "helpers/run_cpu_a_star.cpp",
    "helpers/run_gpu4_a_star.cpp",

    # CPU implementation
    "src/a_star_cpu.cpp",
]

objects = env.Object(cpp_sources)

# ---------- GPU (.cu) build ----------
cu_obj = env.Command(
    target="gpu/a_star_gpu_4.obj",
    source="gpu/a_star_gpu_4.cu",
    action=(
        f'{NVCC} -c -std=c++17 '
        f'-Iinclude '
        f'-I"{CUDA_PATH}\\include" '
        f'-I"C:/Users/thysv/source/godot-cpp/include" '
        f'-I"C:/Users/thysv/source/godot-cpp/gen/include" '
        f'-I"C:/Users/thysv/source/godot-cpp/gdextension" '
        f'-DTYPED_METHOD_BIND '
        f'-o $TARGET $SOURCE'
    )
)

# ---------- Final DLL ----------
env.SharedLibrary(
    target="godot/bin/a_star_wrapper",
    source=objects + [cu_obj]
)