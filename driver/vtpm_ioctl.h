#ifndef VTPM_IOCTL_H
#define VTPM_IOCTL_H

#include <linux/ioctl.h>

/*
 * Virtual TPM ioctl interface
 */

#define VTPM_MAGIC 'V'


/* ============================================================
 * VTPM COMMAND IDs
 * ============================================================ */

#define VTPM_CMD_STATUS        1
#define VTPM_CMD_GENERATE_KEY  2
#define VTPM_CMD_LIST_KEYS     3
#define VTPM_CMD_READ_PCR      4
#define VTPM_CMD_EXTEND_PCR    5
#define VTPM_CMD_SIGN          6
#define VTPM_CMD_VERIFY        7
#define VTPM_CMD_DELETE_KEY    8


/* ============================================================
 * DRIVER STATUS
 * ============================================================ */

struct vtpm_status
{
    int active;
    int driver_version;
};


/* ============================================================
 * GENERIC VTPM COMMAND
 * ============================================================ */

struct vtpm_command
{
    unsigned int command_id;

    /*
     * Generic integer parameter.
     *
     * Examples:
     *   PCR index
     *   Key ID
     */
    int parameter;

    /*
     * 0  = success
     * -1 = command rejected
     */
    int result;
};


/* ============================================================
 * IOCTL DEFINITIONS
 * ============================================================ */

#define VTPM_IOCTL_GET_STATUS \
    _IOR(VTPM_MAGIC, 1, struct vtpm_status)

#define VTPM_IOCTL_PING \
    _IO(VTPM_MAGIC, 2)

#define VTPM_IOCTL_COMMAND \
    _IOWR(VTPM_MAGIC, 3, struct vtpm_command)


#endif
