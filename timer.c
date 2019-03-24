#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "apr.h"
#include "apr_thread_proc.h"
#include "apr_time.h"

//David Bishop
void *APR_THREAD_FUNC expensive_call(apr_thread_t *thread, void *data)
{
    (void)thread;
    bool *done = data;

    /* ... calculations and expensive io here, for example:
     * infinitely loop
     */
    for (;;) {}

    // signal caller that we are done
    *done = true;
    return NULL;
}

bool do_or_timeout(apr_pool_t *pool, apr_thread_start_t func, int max_wait_sec)
{
    apr_thread_t *thread;
    bool thread_done = false;
    apr_thread_create(&thread, NULL, func, &thread_done, pool);
    apr_time_t now = apr_time_now(); 
for (;;) {
        if (thread_done) {
            apr_thread_join(NULL, thread);
            return true;
        }
        if (apr_time_now() >= now + apr_time_make(max_wait_sec, 0)) {
            return false;
        }
        // avoid hogging the CPU in this thread
        apr_sleep(10000);
    }
}

int main(void)
{
    // initialize APR
    apr_initialize();
    apr_pool_t *ap;
    if (apr_pool_create(&ap, NULL) != APR_SUCCESS) {
        exit(127);
    }  
  
//  wait up to 3 seconds
    bool completed = do_or_timeout(ap, expensive_call, 3);
    if (completed) {
        printf("expensive_call completed\n");
    } else {
        printf("expensive_call timed out\n");
    }

    apr_terminate();
    return 0;
}
