#ifndef A_STAR_CPU_WRAPPER_H
#define A_STAR_CPU_WRAPPER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/binder_common.hpp>

namespace godot
{
    class RunWrapper : public Node
    {
        GDCLASS(RunWrapper, Node);

    protected:
        static void _bind_methods();

    public:
        void run(String path_in, String path_out);
    };
}

#endif