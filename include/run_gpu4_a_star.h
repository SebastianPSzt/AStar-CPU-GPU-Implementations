#ifndef RUN_H_GPU4_A_STAR_H
#define RUN_H_GPU4_A_STAR_H

#ifdef __cplusplus
extern "C"
{
#endif

extern "C" __declspec(dllexport)
void run_gpu4(const char *path_in, const char *path_out);

#ifdef __cplusplus
}
#endif

#endif