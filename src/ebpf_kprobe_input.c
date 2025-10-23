// ebpf_input_kprobe.c
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

char LICENSE[] SEC("license") = "GPL";

struct event
{
    __u64 ts_ns;
    __u32 code;
    __s32 value;
    char comm[16];
};

struct
{
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 1 << 24);
} events_rb SEC(".maps");

/*
 * Kprobe na funkcję input_event(struct input_dev *dev, unsigned int type, unsigned int code, int value)
 *  - type = 1 => EV_KEY (klawisze)
 *  - code = kod klawisza (KEY_XXX)
 *  - value = 1 (down), 0 (up), 2 (repeat)
 */
SEC("kprobe/input_event")
int BPF_KPROBE(kprobe_input_event, struct input_dev *dev, unsigned int type, unsigned int code, int value)
{
    if (type != 1)
        return 0; // nie interesują nas inne eventy niż klawiatura

    struct event *e = bpf_ringbuf_reserve(&events_rb, sizeof(*e), 0);
    if (!e)
        return 0;

    e->ts_ns = bpf_ktime_get_ns();
    e->code = code;
    e->value = value;
    bpf_get_current_comm(&e->comm, sizeof(e->comm));

    bpf_ringbuf_submit(e, 0);
    return 0;
}
