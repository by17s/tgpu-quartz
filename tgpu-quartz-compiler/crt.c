#include "crt.h"

#include "stdio.h"
#include "stdlib.h"

#ifdef TARGET_TGPU_QUARTZ
# include "target/tgpu_quartz_gen.c"
#else
#error [Err] Invalid target
#endif