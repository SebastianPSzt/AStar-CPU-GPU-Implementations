#ifndef A_STAR_GPU4_WRAPPER_H
#define A_STAR_GPU4_WRAPPER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot
{
    class RunGPU4Wrapper : public Node
    {
        GDCLASS(RunGPU4Wrapper, Node);

    protected:
        static void _bind_methods();

    public:
        void run(String path_in, String path_out);
    };
}

#endif