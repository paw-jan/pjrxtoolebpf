// user_kprobe.c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <bpf/libbpf.h>
#include "ebpf_kprobe_input.skel.h"
#include "ebpf_kprobe_input_uspace.h"

static volatile bool exiting = false;

static void sig_handler(int signo)
{
    exiting = true;
}

static int handle_event(void *ctx, void *data, size_t data_sz)
{
    struct event *e = data;
    time_t s = e->ts_ns / 1000000000ULL;
    long ns = e->ts_ns % 1000000000ULL;
    char buf[64];
    struct tm tm;
    localtime_r(&s, &tm);
    strftime(buf, sizeof(buf), "%F %T", &tm);

    const char *state = (e->value == 1) ? "DOWN" : (e->value == 0) ? "UP" : "HOLD";
    printf("%s.%09ld code=%u %s comm=%s\n", buf, ns, e->code, state, e->comm);
    return 0;
}

int main(void)
{
    struct ebpf_kprobe_input *skel = NULL;
    struct ring_buffer *rb = NULL;
    int err;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    skel = ebpf_kprobe_input__open_and_load();
    if (!skel)
    {
        fprintf(stderr, "failed to open/load skeleton\n");
        return 1;
    }

    err = ebpf_kprobe_input__attach(skel);
    if (err)
    {
        fprintf(stderr, "failed to attach kprobe: %d\n", err);
        goto cleanup;
    }

    rb = ring_buffer__new(bpf_map__fd(skel->maps.events_rb), handle_event, NULL, NULL);
    if (!rb)
    {
        fprintf(stderr, "failed to create ring buffer: %s\n", strerror(errno));
        goto cleanup;
    }

    printf("Listening for keyboard events via kprobe/input_event... Ctrl-C to exit.\n");

    while (!exiting)
    {
        err = ring_buffer__poll(rb, 100);
        if (err == -EINTR)
            break;
        if (err < 0)
        {
            fprintf(stderr, "poll error: %d\n", err);
            break;
        }
    }

cleanup:
    ring_buffer__free(rb);
    ebpf_kprobe_input__destroy(skel);
    return 0;
}