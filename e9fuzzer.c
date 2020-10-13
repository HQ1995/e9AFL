/*
  Copyright 2013 Google LLC All rights reserved.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at:

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

/*
   american fuzzy lop - fuzzer code
   --------------------------------

   Written and maintained by Michal Zalewski <lcamtuf@google.com>

   Forkserver design by Jann Horn <jannhorn@googlemail.com>

   This is the real deal: the program takes an instrumented binary and
   attempts a variety of basic fuzzing tricks, paying close attention to
   how they affect the execution path.

*/

#include "e9fuzzer.h"

static int shm_id = 0;
u8 *__afl_area_ptr = NULL;
u32 __afl_prev = 0;
char ENV_STR[13] = "__AFL_SHM_ENV";

static u8 is_persistent;

/* SHM setup. */

static void __afl_map_shm(void) 
{
    __afl_area_ptr = _shmat(shm_id, NULL, 0);

    /* Whooooops. */

    if (__afl_area_ptr == (void *)-1) _fatal("shmat error\n");

    /* Write something into the bitmap so that even with low AFL_INST_RATIO,
    our parent doesn't give up on us. */

    __afl_area_ptr[0] = 1;
}

/* Fork server logic. */

static void __afl_start_forkserver(void) 
{
    static u8 tmp[4];
    s32 child_pid;

    u8 child_stopped = 0;

    /* Phone home and tell the parent that we're OK. If parent isn't there,
        assume we're not running in forkserver mode and just execute program. */

    if (write(FORKSRV_FD + 1, tmp, 4) != 4) return;

    while (1) {
    u32 was_killed;
    int status;

    /* Wait for parent by reading from the pipe. Abort if read fails. */

    if (read(FORKSRV_FD, &was_killed, 4) != 4) _exit(1);

    /* If we stopped the child in persistent mode, but there was a race
        condition and afl-fuzz already issued SIGKILL, write off the old
        process. */

    if (child_stopped && was_killed) {
        child_stopped = 0;
        if (_waitpid(child_pid, &status, 0) < 0) _exit(1);
    }

    if (!child_stopped) {
        /* Once woken up, create a clone of our process. */

        child_pid = fork();
        if (child_pid < 0) _exit(1);

        /* In child process: close fds, resume execution. */

        if (!child_pid) {
            close(FORKSRV_FD);
            close(FORKSRV_FD + 1);
            return;
        }

    } else {
        /* Special handling for persistent mode: if the child is alive but
            currently stopped, simply restart it with SIGCONT. */

        _kill(child_pid, SIGCONT);
        child_stopped = 0;
    }

    /* In parent process: write PID to pipe, then wait for child. */

    if (write(FORKSRV_FD + 1, &child_pid, 4) != 4) _exit(1);

    if (_waitpid(child_pid, &status, is_persistent ? WUNTRACED : 0) < 0)
        _exit(1);

    /* In persistent mode, the child stops itself with SIGSTOP to indicate
        a successful run. In this case, we want to wake it up without forking
        again. */

    if (WIFSTOPPED(status)) child_stopped = 1;

    /* Relay wait status to pipe, then loop back. */

    if (write(FORKSRV_FD + 1, &status, 4) != 4) _exit(1);
    }
}

void e9_maybe_log(intptr_t rip, intptr_t next, intptr_t val) 
{
    u32 src = rip - val;
    u32 dst = next - val;
    __afl_area_ptr[((src >> 1) ^ dst) % MAP_SIZE]++;
}

void init(int argc, char **argv, char **envp) 
{
    bool has_shm_env = false;


    for (; envp && *envp != NULL; envp++) {
    char *var = *envp;
    if (var[0] == '_' && var[1] == '_' && var[2] == 'A' && var[3] == 'F' &&
        var[4] == 'L' && var[5] == '_' && var[6] == 'S' && var[7] == 'H' &&
        var[8] == 'M' && var[9] == '_' && var[10] == 'I' && var[11] == 'D') {
        int val = 0;
        has_shm_env = true;

        for (unsigned i = 13; var[i] >= '0' && var[i] <= '9'; i++)
        val = 10 * val + (var[i] - '0');

        shm_id = val;
        break;
    }
    }

    if (has_shm_env) {
    // map the shared memory
    printf("SHM_ID: %d\n", shm_id);

    __afl_map_shm();
    __afl_start_forkserver();
    } else {
    // to avoid reallocation
    __afl_area_ptr = (u8 *)_mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE,
                                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (__afl_area_ptr == NULL) _fatal("mmap failed\n");
    }
}
