#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <pthread.h>

#include <mutex>

#include "unsupported/Eigen/CXX11/ThreadPool"

using namespace Eigen;

int nthr;
ThreadPoolInterface *tp;

template <typename F>
void parallel_for(int n, F f, bool with_hint = false) {
    const int start = 0;
    const int end = n;

    Barrier b(end - start);

    for (int i = start; i < end; ++i)
    if (with_hint) {
        tp->ScheduleWithHint([f = std::move(f), &b, i] { f(i); b.Notify(); }, i, i + 1);
    } else {
        tp->Schedule([f = std::move(f), &b, i] { f(i); b.Notify(); });
    }

    b.Wait();
}

void test_distrib(int x) {
    parallel_for(nthr, [=](int ithr) {
        sleep(1);
        printf("%d\tithr:%d thread_id:%d     %s\n",
                x, ithr, tp->CurrentThreadId(),
                ithr == tp->CurrentThreadId() ? "OK" : "fail");
    }, true);
}

void bar(int x) {
    parallel_for(2, [=](int ithr) {
        sleep(1);
        printf("[bar] %d: ithr:%d thread_id:%d\n",
                x, ithr, tp->CurrentThreadId());
    }, true);
}

void foo(int x) {
    parallel_for(2, [=](int ithr) {
        sleep(1);
        printf("[foo] %d: ithr:%d thread_id:%d     %s\n",
                x, ithr, tp->CurrentThreadId(),
                ithr == tp->CurrentThreadId() ? "OK" : "fail");
        bar(ithr);
    }, true);
}

int main(int argc, char **argv) {
    nthr = argc > 1 ? atoi(argv[1]) : 16;
    tp = new ThreadPool(nthr, false); sleep(1);

#if 0
    for (int i = 1; i <= 37; ++i) {
        test_distrib(i); printf("\n"); sleep(1);
    }
#endif

    foo(10);

    return 0;
}
