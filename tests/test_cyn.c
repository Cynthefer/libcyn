#include "cyn.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    cyn_ctx_t *ctx = cyn_create(NULL);
    assert(ctx);

    cyn_info info;
    assert(cyn_collect(ctx, &info) == CYN_OK);

    assert(cyn_field_ok(&info.user.username));
    assert(cyn_field_ok(&info.user.uid));
    assert(cyn_field_ok(&info.system.kernel));
    assert(cyn_field_ok(&info.system.architecture));
    assert(cyn_field_ok(&info.memory.size));
    assert(cyn_field_ok(&info.storage.total));

    /* DMI hardware should be present on most machines but not guaranteed */
    printf("hardware brand: %s (%s)\n",
           info.hardware.brand.value,
           cyn_status_str(info.hardware.brand.status));

    const char *k = cyn_system_kernel(ctx);
    assert(k && *k);

    cyn_destroy(ctx);
    puts("OK");
    return 0;
}