#include "cyn.h"
#include <stdio.h>

int main(void) {
    cyn_options opts = { .fetch_public_ip = 1 };
    cyn_ctx_t *ctx = cyn_create(&opts);

    cyn_info info;
    if (cyn_collect(ctx, &info) != CYN_OK) {
        fprintf(stderr, "collect: %s\n", cyn_last_error());
        return 1;
    }

    /* The library gives you data; presentation is entirely yours. */
    if (cyn_field_ok(&info.user.username))
        printf("user = %s\n", info.user.username.value);

    if (cyn_field_ok(&info.system.distro))
        printf("distro = %s\n", info.system.distro.value);

    if (cyn_field_ok(&info.memory.size))
        printf("ram = %s\n", info.memory.size.value);

    /* Unavailable fields are still safe to inspect: */
    printf("mem speed = %s (%s)\n",
           info.memory.speed.value,
           cyn_status_str(info.memory.speed.status));

    /* Or one-shot convenience accessor: */
    const char *cpu = cyn_cpu_model(ctx);
    if (cpu) printf("cpu = %s\n", cpu);

    cyn_destroy(ctx);
    return 0;
}