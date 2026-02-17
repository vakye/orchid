
#if x86_64
# include "arch_x64.h"
# include "arch_x64.c"
#else
# error "Unimplemented architecture"
#endif
