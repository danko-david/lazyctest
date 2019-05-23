
#if !__has_include ( "parse_elf.c" )
	#error "Dependency source file `parse_elf.c` doesn't exists, maybe you have to call ~repo/scripts/get_c_deps.sh to download dependencies."
#endif

#include "parse_elf.c"
#include "utils.c"
#include "concurrency.c"
#include "rerunnable_thread.c"
#include "worker_pool.c"
#include "pcre_util.c"
#include "lct_testing.c"
#include "lct_result_writer.c"
#include "lazyctest.c"
#include "lazyctester.c"
