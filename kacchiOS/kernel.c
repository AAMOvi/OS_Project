/* kernel.c - Main kernel with simple scheduler demo */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"
#include "ipc.h"

#define MAX_INPUT 128
#define SHELL_STACK 4096
#define WORKER_STACK 4096

static ipc_queue_t global_queue;

static void busy_delay(void)
{
    volatile uint32_t i;
    for (i = 0; i < 5000000; i++)
        ;
}

static void serial_putu(uint32_t value)
{
    char buf[11];
    int idx = 0;
    do
    {
        buf[idx++] = (char)('0' + (value % 10));
        value /= 10;
    } while (value && idx < 10);
    while (idx--)
    {
        serial_putc(buf[idx]);
    }
}

static void idle_process(void *arg)
{
    (void)arg;
    while (1)
    {
        scheduler_yield();
    }
}

static void heartbeat_process(void *arg)
{
    (void)arg;
    uint32_t tick = 0;
    /* Wait at startup so welcome message is visible */
    for (int i = 0; i < 5; i++)
    {
        busy_delay();
        scheduler_yield();
    }
    while (1)
    {
        serial_puts("[heartbeat] tick ");
        serial_putu(tick++);
        serial_puts("\n");
        busy_delay();
        scheduler_yield();
    }
}

static void receiver_process(void *arg)
{
    (void)arg;
    uint32_t val = 0;
    while (1)
    {
        ipc_recv(&global_queue, &val);
        serial_puts("[ipc recv] value=");
        serial_putu(val);
        serial_puts("\n");
        scheduler_yield();
    }
}

static int parse_send_command(const char *input)
{
    const char *p = input;
    if (strncmp(p, "send", 4) != 0)
    {
        return 0;
    }
    p += 4;
    while (*p == ' ')
        p++;
    int val = atoi(p);
    ipc_send(&global_queue, (uint32_t)val);
    serial_puts("[ipc send] queued ");
    serial_putu((uint32_t)val);
    serial_puts("\n");
    return 1;
}

static int parse_help_command(const char *input)
{
    if (strcmp(input, "help") != 0)
    {
        return 0;
    }
    serial_puts("Commands: help, send <num>, ps, mem\n");
    return 1;
}

static int parse_ps_command(const char *input)
{
    if (strcmp(input, "ps") != 0)
    {
        return 0;
    }
    serial_puts("PID  STATE      STACK\n");
    for (int i = 0; i < process_get_count(); i++)
    {
        process_t *p = process_get_by_index(i);
        if (p->state == PROC_UNUSED)
            continue;
        serial_putu(p->pid);
        serial_puts("    ");
        const char *state = "UNKNOWN";
        if (p->state == PROC_CURRENT)
            state = "CURRENT";
        else if (p->state == PROC_READY)
            state = "READY";
        else if (p->state == PROC_BLOCKED)
            state = "BLOCKED";
        else if (p->state == PROC_TERMINATED)
            state = "TERMINATED";
        serial_puts(state);
        serial_puts("   ");
        serial_putu(p->stack_size);
        serial_puts("\n");
    }
    return 1;
}

static int parse_mem_command(const char *input)
{
    if (strcmp(input, "mem") != 0)
    {
        return 0;
    }
    uint32_t total_free, largest;
    memory_get_stats(&total_free, &largest);
    serial_puts("Heap free: ");
    serial_putu(total_free);
    serial_puts(" bytes, largest block: ");
    serial_putu(largest);
    serial_puts(" bytes\n");
    return 1;
}

static void shell_process(void *arg)
{
    (void)arg;
    char input[MAX_INPUT];
    int pos = 0;

    while (1)
    {
        serial_puts("kacchiOS> ");
        pos = 0;

        while (1)
        {
            while (!serial_available())
            {
                scheduler_yield();
            }
            char c = serial_getc();

            if (c == '\r' || c == '\n')
            {
                input[pos] = '\0';
                serial_puts("\n");
                break;
            }
            else if ((c == '\b' || c == 0x7F) && pos > 0)
            {
                pos--;
                serial_puts("\b \b");
            }
            else if (c >= 32 && c < 127 && pos < MAX_INPUT - 1)
            {
                input[pos++] = c;
                serial_putc(c);
            }
        }

        if (pos > 0)
        {
            if (!parse_help_command(input) &&
                !parse_send_command(input) &&
                !parse_ps_command(input) &&
                !parse_mem_command(input))
            {
                serial_puts("You typed: ");
                serial_puts(input);
                serial_puts("\n");
            }
        }

        scheduler_yield();
    }
}

void kmain(void)
{
    serial_init();
    memory_init();
    process_init();
    scheduler_init();

    serial_puts("\n");
    serial_puts("========================================\n");
    serial_puts("    kacchiOS - Minimal Baremetal OS\n");
    serial_puts("========================================\n");
    serial_puts("Hello from kacchiOS!\n");
    serial_puts("Starting scheduler demo...\n\n");

    ipc_init(&global_queue);
    process_create(shell_process, 0, SHELL_STACK);
    process_create(heartbeat_process, 0, WORKER_STACK);
    process_create(receiver_process, 0, WORKER_STACK);
    process_create(idle_process, 0, WORKER_STACK);

    scheduler_start();

    for (;;)
    {
        __asm__ volatile("hlt");
    }
}