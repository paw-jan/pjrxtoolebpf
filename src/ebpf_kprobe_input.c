// ebpf_kprobe_input.c
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

#include <linux/input-event-codes.h>

#include "ebpf_kprobe_input_kspace.h"

char LICENSE[] SEC("license") = "GPL";


struct
{
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 1 << 24);
} events_rb SEC(".maps");

/*
 * Kprobe of function: input_event(struct input_dev *dev, unsigned int type, unsigned int code, int value)
 *  - type = 1 => EV_KEY (klawisze)
 *  - code = kod klawisza (KEY_XXX)
 *  - value = 1 (down), 0 (up), 2 (repeat)
 */
SEC("kprobe/input_event")
int BPF_KPROBE(kprobe_input_event, struct input_dev *dev, unsigned int type, unsigned int code, int value)
{
    if (type != EV_KEY)
        return 0;

    // Filter out mouse buttons
    if (code == BTN_LEFT || code == BTN_RIGHT || code == BTN_MIDDLE)
        return 0;        

    struct event *e = bpf_ringbuf_reserve(&events_rb, sizeof(*e), 0);
    if (!e)
        return 0;

    e->ts_ns = bpf_ktime_get_ns();
    e->type = 1;
    e->code = code;
    e->value = value;
    bpf_get_current_comm(&e->comm, sizeof(e->comm));

    bpf_ringbuf_submit(e, 0);
    return 0;
}
