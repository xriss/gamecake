### NAME

MMIO wrapper functions for the Linux userspace `/dev/mem` device.

### SYNOPSIS

``` c
#include <periphery/mmio.h>

/* Primary Functions */
int mmio_open(mmio_t *mmio, uintptr_t base, size_t size);
void *mmio_ptr(mmio_t *mmio);
int mmio_read32(mmio_t *mmio, uintptr_t offset, uint32_t *value);
int mmio_read16(mmio_t *mmio, uintptr_t offset, uint16_t *value);
int mmio_read8(mmio_t *mmio, uintptr_t offset, uint8_t *value);
int mmio_read(mmio_t *mmio, uintptr_t offset, uint8_t *buf, size_t len);
int mmio_write32(mmio_t *mmio, uintptr_t offset, uint32_t value);
int mmio_write16(mmio_t *mmio, uintptr_t offset, uint16_t value);
int mmio_write8(mmio_t *mmio, uintptr_t offset, uint8_t value);
int mmio_write(mmio_t *mmio, uintptr_t offset, const uint8_t *buf, size_t len);
int mmio_close(mmio_t *mmio);

/* Miscellaneous */
uintptr_t mmio_base(mmio_t *mmio);
size_t mmio_size(mmio_t *mmio);
int mmio_tostring(mmio_t *mmio, char *str, size_t len);

/* Error Handling */
int mmio_errno(mmio_t *mmio);
const char *mmio_errmsg(mmio_t *mmio);
```

### DESCRIPTION

``` c
int mmio_open(mmio_t *mmio, uintptr_t base, size_t size);
```
Map the region of physical memory at the specified base address with the specified size.

`mmio` should be a valid pointer to an allocated MMIO handle structure. Neither `base` nor `size` need be aligned to a page boundary.

Returns 0 on success, or a negative [MMIO error code](#return-value) on failure.

------

``` c
void *mmio_ptr(mmio_t *mmio);
```
Return the pointer to the mapped physical memory.

This function is a simple accessor to the MMIO handle structure and always succeeds.

------

``` c
int mmio_read32(mmio_t *mmio, uintptr_t offset, uint32_t *value);
int mmio_read16(mmio_t *mmio, uintptr_t offset, uint16_t *value);
int mmio_read8(mmio_t *mmio, uintptr_t offset, uint8_t *value);
int mmio_read(mmio_t *mmio, uintptr_t offset, uint8_t *buf, size_t len);
```
Read 32-bits, 16-bits, 8-bits, or an array of bytes, respectively, from mapped physical memory, starting at the specified byte offset, relative to the base address the MMIO handle was opened with.

`mmio` should be a valid pointer to an MMIO handle opened with `mmio_open()`.

Returns 0 on success, or a negative [MMIO error code](#return-value) on failure.

------

``` c
int mmio_write32(mmio_t *mmio, uintptr_t offset, uint32_t value);
int mmio_write16(mmio_t *mmio, uintptr_t offset, uint16_t value);
int mmio_write8(mmio_t *mmio, uintptr_t offset, uint8_t value);
int mmio_write(mmio_t *mmio, uintptr_t offset, const uint8_t *buf, size_t len);
```
Write 32-bits, 16-bits, 8-bits, or an array of bytes, respectively, to mapped physical memory, starting at the specified byte offset, relative to the base address the MMIO handle was opened with.

`mmio` should be a valid pointer to an MMIO handle opened with `mmio_open()`.

Returns 0 on success, or a negative [MMIO error code](#return-value) on failure.

------

``` c
int mmio_close(mmio_t *mmio);
```
Unmap mapped physical memory.

`mmio` should be a valid pointer to an MMIO handle opened with `mmio_open()`.

Returns 0 on success, or a negative [MMIO error code](#return-value) on failure.

------

``` c
uintptr_t mmio_base(mmio_t *mmio);
```
Return the base address the MMIO handle was opened with.

`mmio` should be a valid pointer to an MMIO handle opened with `mmio_open()`.

This function is a simple accessor to the MMIO handle structure and always succeeds.

------

``` c
size_t mmio_size(mmio_t *mmio);
```
Return the size the MMIO handle was opened with.

`mmio` should be a valid pointer to an MMIO handle opened with `mmio_open()`.

This function is a simple accessor to the MMIO handle structure and always succeeds.

------

``` c
int mmio_tostring(mmio_t *mmio, char *str, size_t len);
```
Return a string representation of the MMIO handle.

`mmio` should be a valid pointer to an MMIO handle opened with `mmio_open()`.

This function behaves and returns like `snprintf()`.

------

``` c
int mmio_errno(mmio_t *mmio);
```
Return the libc errno of the last failure that occurred.

`mmio` should be a valid pointer to an MMIO handle opened with `mmio_open()`.

------

``` c
const char *mmio_errmsg(mmio_t *mmio);
```
Return a human readable error message of the last failure that occurred.

`mmio` should be a valid pointer to an MMIO handle opened with `mmio_open()`.

### RETURN VALUE

The periphery MMIO functions return 0 on success or one of the negative error codes below on failure.

The libc errno of the failure in an underlying libc library call can be obtained with the `mmio_errno()` helper function. A human readable error message can be obtained with the `mmio_errmsg()` helper function.

| Error Code            | Description                   |
|-----------------------|-------------------------------|
| `MMIO_ERROR_ARG`      | Invalid arguments             |
| `MMIO_ERROR_OPEN`     | Opening /dev/mem              |
| `MMIO_ERROR_MAP`      | Mapping memory                |
| `MMIO_ERROR_CLOSE`    | Closing /dev/mem              |
| `MMIO_ERROR_UNMAP`    | Unmapping memory              |

### EXAMPLE

``` c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "mmio.h"

struct am335x_rtcss_registers {
    uint32_t seconds;       /* 0x00 */
    uint32_t minutes;       /* 0x04 */
    uint32_t hours;         /* 0x08 */
    /* ... */
};

int main(void) {
    mmio_t mmio;
    uint32_t mac_id0_lo, mac_id0_hi;
    volatile struct am335x_rtcss_registers *regs;

    /* Open control module */
    if (mmio_open(&mmio, 0x44E10000, 0x1000) < 0) {
        fprintf(stderr, "mmio_open(): %s\n", mmio_errmsg(&mmio));
        exit(1);
    }

    /* Read lower 2 bytes of MAC address */
    if (mmio_read32(&mmio, 0x630, &mac_id0_lo) < 0) {
        fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(&mmio));
        exit(1);
    }

    /* Read upper 4 bytes of MAC address */
    if (mmio_read32(&mmio, 0x634, &mac_id0_hi) < 0) {
        fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(&mmio));
        exit(1);
    }

    printf("MAC address: %04x%08x\n", mac_id0_lo, mac_id0_hi);

    mmio_close(&mmio);

    /* Open RTC subsystem */
    if (mmio_open(&mmio, 0x44E3E000, 0x1000) < 0) {
        fprintf(stderr, "mmio_open(): %s\n", mmio_errmsg(&mmio));
        exit(1);
    }

    regs = mmio_ptr(&mmio);

    /* Read current RTC time */
    printf("hours: %02x minutes: %02x seconds %02x\n", regs->hours, regs->minutes, regs->seconds);

    mmio_close(&mmio);

    return 0;
}
```

