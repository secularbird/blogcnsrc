#define _GNU_SOURCE

#include <execinfo.h>
#include <signal.h>
#include <assert.h>
#include <dlfcn.h>
#include <unwind.h>

static int step = 0;

_Unwind_Reason_Code handler(struct _Unwind_Context* context, void* ref)
{
	Dl_info dyldInfo;
	void *addr = (void*)_Unwind_GetIP(context);
	if ( dladdr(addr, &dyldInfo) ) {
		printf("#%d [%p] [%p] [%s] [%p] [%s] \n", 
			step,
			addr,
			dyldInfo.dli_saddr,
			dyldInfo.dli_sname,
			dyldInfo.dli_fbase,
			dyldInfo.dli_fname);
		step ++;
	}
	return _URC_NO_REASON;
}

void signalhandler(int sig)
{
    printf("signal revived [%d]\n", sig);
    // get the back trace
    enum {
        MAX_STACK = 32,
    };

    void *stack[MAX_STACK];
    int size = backtrace(stack, MAX_STACK);
    if(size == 0) {
        printf("fail to get backtrace\n");
    }
    char** strs = backtrace_symbols(stack, size);
    int i = 0;
    for (i = 0; i < size; ++i) {
        printf("%s\n", strs[i]);
    }
    free(strs);

	_Unwind_Backtrace(&handler, NULL);

    //show_backtrace();

    signal(sig, SIG_DFL);
}

void addsignalstack(char *stack, int size)
{
	assert(stack!=NULL);
	// using sginal stack
    stack_t sigstack;
    sigstack.ss_sp = stack;
    sigstack.ss_size = SIGSTKSZ;
    sigstack.ss_flags = 0;
    if (sigaltstack(&sigstack, 0) < 0) {
        perror("sig stack setting failed");
    }
}

void registersignal()
{
    struct sigaction act;
    act.sa_handler = signalhandler;
    act.sa_flags = SA_ONSTACK|SA_RESTART;

    int ret = sigaction(SIGSEGV, &act, NULL);
    if (ret < 0) {
        perror("sigaction failed\n");
    }

    ret = sigaction(SIGBUS, &act, NULL);
    if (ret < 0) {
        perror("sigaction failed\n");
    }

    ret = sigaction(SIGILL, &act, NULL);
    if (ret < 0) {
        perror("sigaction failed\n");
    }

    ret = sigaction(SIGABRT, &act, NULL);
    if (ret < 0) {
        perror("sigaction failed\n");
    }

}
