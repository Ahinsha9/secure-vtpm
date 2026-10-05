#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/cred.h>
#include <linux/uidgid.h>

#include "vtpm_ioctl.h"


/*
 * ============================================================
 * VTPM DRIVER CONFIGURATION
 * ============================================================
 */

#define DEVICE_NAME "vtpm_secure"
#define CLASS_NAME  "vtpm"

/*
 * Linux group ID used to access the VTPM device.
 *
 * Your Ubuntu system:
 *
 * vtpm:x:1001:vboxuser
 *
 * Therefore the vtpm group ID is 1001.
 */

#define VTPM_GID 1001


/*
 * ============================================================
 * GLOBAL DRIVER VARIABLES
 * ============================================================
 */

static dev_t vtpm_device_number;

static struct cdev vtpm_cdev;

static struct class *vtpm_class;

static struct device *vtpm_device;


/*
 * ============================================================
 * ACCESS CONTROL
 * ============================================================
 *
 * Root users are allowed.
 *
 * Members of the vtpm Linux group are also allowed.
 */

static bool vtpm_is_authorized(void)
{
    kgid_t vtpm_gid;

    /*
     * Root is always authorized.
     */

    if (current_euid().val == 0)
    {
        return true;
    }

    /*
     * Convert the numeric group ID into
     * a kernel group ID.
     */

    vtpm_gid = make_kgid(
        current_user_ns(),
        VTPM_GID
    );

    /*
     * Check whether the current process
     * belongs to the vtpm group.
     */

    if (gid_valid(vtpm_gid) &&
        in_group_p(vtpm_gid))
    {
        return true;
    }

    /*
     * Access denied.
     */

    pr_warn(
        "vtpm_secure: "
        "unauthorized access attempt by UID %u\n",
        __kuid_val(current_euid())
    );

    return false;
}


/*
 * ============================================================
 * DEVICE OPEN
 * ============================================================
 */

static int vtpm_open(
    struct inode *inode,
    struct file *file
)
{
    pr_info(
        "vtpm_secure: device opened\n"
    );

    return 0;
}


/*
 * ============================================================
 * DEVICE RELEASE
 * ============================================================
 */

static int vtpm_release(
    struct inode *inode,
    struct file *file
)
{
    pr_info(
        "vtpm_secure: device closed\n"
    );

    return 0;
}


/*
 * ============================================================
 * IOCTL HANDLER
 * ============================================================
 */

static long vtpm_ioctl(
    struct file *file,
    unsigned int command,
    unsigned long argument
)
{
    struct vtpm_status status;

    struct vtpm_command vtpm_cmd;


    /*
     * --------------------------------------------------------
     * GET STATUS
     * --------------------------------------------------------
     */

    if (command == VTPM_IOCTL_GET_STATUS)
    {
        pr_info(
            "vtpm_secure: GET_STATUS received\n"
        );

        status.active = 1;

        status.driver_version = 1;


        /*
         * Copy status from kernel space
         * to user space.
         */

        if (copy_to_user(
                (void __user *)argument,
                &status,
                sizeof(status)))
        {
            pr_err(
                "vtpm_secure: "
                "copy_to_user failed\n"
            );

            return -EFAULT;
        }

        return 0;
    }


    /*
     * --------------------------------------------------------
     * PING
     * --------------------------------------------------------
     */

    if (command == VTPM_IOCTL_PING)
    {
        pr_info(
            "vtpm_secure: PING received\n"
        );

        return 0;
    }


    /*
     * --------------------------------------------------------
     * VTPM COMMAND
     * --------------------------------------------------------
     *
     * Sensitive VTPM operations require authorization.
     */

    if (command == VTPM_IOCTL_COMMAND)
    {
        /*
         * Check user authorization first.
         */

        if (!vtpm_is_authorized())
        {
            return -EPERM;
        }


        /*
         * Copy command from user space
         * into kernel space.
         */

        if (copy_from_user(
                &vtpm_cmd,
                (void __user *)argument,
                sizeof(vtpm_cmd)))
        {
            pr_err(
                "vtpm_secure: "
                "copy_from_user failed\n"
            );

            return -EFAULT;
        }


        /*
         * Log the command.
         */

        pr_info(
            "vtpm_secure: "
            "command received: %u\n",
            vtpm_cmd.command_id
        );


        /*
         * ----------------------------------------------------
         * COMMAND VALIDATION
         * ----------------------------------------------------
         */

        switch (vtpm_cmd.command_id)
        {

        /*
         * ----------------------------------------------------
         * STATUS
         * ----------------------------------------------------
         */

        case VTPM_CMD_STATUS:

            pr_info(
                "vtpm_secure: "
                "STATUS command authorized\n"
            );

            vtpm_cmd.result = 0;

            break;


        /*
         * ----------------------------------------------------
         * GENERATE KEY
         * ----------------------------------------------------
         */

        case VTPM_CMD_GENERATE_KEY:

            pr_info(
                "vtpm_secure: "
                "GENERATE_KEY command authorized\n"
            );

            vtpm_cmd.result = 0;

            break;


        /*
         * ----------------------------------------------------
         * LIST KEYS
         * ----------------------------------------------------
         */

        case VTPM_CMD_LIST_KEYS:

            pr_info(
                "vtpm_secure: "
                "LIST_KEYS command authorized\n"
            );

            vtpm_cmd.result = 0;

            break;


        /*
         * ----------------------------------------------------
         * READ PCR
         * ----------------------------------------------------
         */

        case VTPM_CMD_READ_PCR:

            /*
             * Valid PCR range:
             *
             * PCR0 through PCR7
             */

            if (vtpm_cmd.parameter < 0 ||
                vtpm_cmd.parameter > 7)
            {
                pr_warn(
                    "vtpm_secure: "
                    "invalid PCR index\n"
                );

                vtpm_cmd.result = -1;

                break;
            }

            pr_info(
                "vtpm_secure: "
                "READ_PCR command authorized "
                "for PCR%d\n",
                vtpm_cmd.parameter
            );

            vtpm_cmd.result = 0;

            break;


        /*
         * ----------------------------------------------------
         * EXTEND PCR
         * ----------------------------------------------------
         */

        case VTPM_CMD_EXTEND_PCR:

            /*
             * Valid PCR range:
             *
             * PCR0 through PCR7
             */

            if (vtpm_cmd.parameter < 0 ||
                vtpm_cmd.parameter > 7)
            {
                pr_warn(
                    "vtpm_secure: "
                    "invalid PCR index\n"
                );

                vtpm_cmd.result = -1;

                break;
            }

            pr_info(
                "vtpm_secure: "
                "EXTEND_PCR command authorized "
                "for PCR%d\n",
                vtpm_cmd.parameter
            );

            vtpm_cmd.result = 0;

            break;


        /*
         * ----------------------------------------------------
         * SIGN
         * ----------------------------------------------------
         */

        case VTPM_CMD_SIGN:

            pr_info(
                "vtpm_secure: "
                "SIGN command authorized\n"
            );

            vtpm_cmd.result = 0;

            break;


        /*
         * ----------------------------------------------------
         * VERIFY
         * ----------------------------------------------------
         */

        case VTPM_CMD_VERIFY:

            pr_info(
                "vtpm_secure: "
                "VERIFY command authorized\n"
            );

            vtpm_cmd.result = 0;

            break;


        /*
         * ----------------------------------------------------
         * DELETE KEY
         * ----------------------------------------------------
         */

        case VTPM_CMD_DELETE_KEY:

            pr_info(
                "vtpm_secure: "
                "DELETE_KEY command received "
                "for key %d\n",
                vtpm_cmd.parameter
            );


            /*
             * Key IDs must be positive.
             */

            if (vtpm_cmd.parameter <= 0)
            {
                pr_warn(
                    "vtpm_secure: "
                    "invalid key ID\n"
                );

                vtpm_cmd.result = -1;

                break;
            }


            pr_info(
                "vtpm_secure: "
                "DELETE_KEY command authorized\n"
            );

            vtpm_cmd.result = 0;

            break;


        /*
         * ----------------------------------------------------
         * UNKNOWN COMMAND
         * ----------------------------------------------------
         */

        default:

            pr_warn(
                "vtpm_secure: "
                "unknown VTPM command: %u\n",
                vtpm_cmd.command_id
            );

            vtpm_cmd.result = -1;

            break;
        }


        /*
         * ----------------------------------------------------
         * Return command result to user space.
         * ----------------------------------------------------
         */

        if (copy_to_user(
                (void __user *)argument,
                &vtpm_cmd,
                sizeof(vtpm_cmd)))
        {
            pr_err(
                "vtpm_secure: "
                "copy_to_user failed\n"
            );

            return -EFAULT;
        }


        /*
         * If command validation failed,
         * return Invalid Argument.
         */

        if (vtpm_cmd.result != 0)
        {
            return -EINVAL;
        }


        return 0;
    }


    /*
     * --------------------------------------------------------
     * UNKNOWN IOCTL
     * --------------------------------------------------------
     */

    pr_warn(
        "vtpm_secure: "
        "unknown ioctl command\n"
    );

    return -EINVAL;
}


/*
 * ============================================================
 * FILE OPERATIONS
 * ============================================================
 */

static const struct file_operations vtpm_fops =
{
    .owner = THIS_MODULE,

    .open = vtpm_open,

    .release = vtpm_release,

    .unlocked_ioctl = vtpm_ioctl
};


/*
 * ============================================================
 * DRIVER INITIALIZATION
 * ============================================================
 */

static int __init vtpm_driver_init(void)
{
    int result;


    pr_info(
        "vtpm_secure: "
        "initializing driver\n"
    );


    /*
     * --------------------------------------------------------
     * Allocate device number
     * --------------------------------------------------------
     */

    result = alloc_chrdev_region(
        &vtpm_device_number,
        0,
        1,
        DEVICE_NAME
    );

    if (result < 0)
    {
        pr_err(
            "vtpm_secure: "
            "failed to allocate device number\n"
        );

        return result;
    }


    pr_info(
        "vtpm_secure: "
        "major=%d minor=%d\n",
        MAJOR(vtpm_device_number),
        MINOR(vtpm_device_number)
    );


    /*
     * --------------------------------------------------------
     * Initialize character device
     * --------------------------------------------------------
     */

    cdev_init(
        &vtpm_cdev,
        &vtpm_fops
    );


    vtpm_cdev.owner = THIS_MODULE;


    /*
     * --------------------------------------------------------
     * Add character device
     * --------------------------------------------------------
     */

    result = cdev_add(
        &vtpm_cdev,
        vtpm_device_number,
        1
    );

    if (result < 0)
    {
        pr_err(
            "vtpm_secure: "
            "failed to add character device\n"
        );

        unregister_chrdev_region(
            vtpm_device_number,
            1
        );

        return result;
    }


    /*
     * --------------------------------------------------------
     * Create device class
     * --------------------------------------------------------
     */

    vtpm_class = class_create(
        CLASS_NAME
    );

    if (IS_ERR(vtpm_class))
    {
        pr_err(
            "vtpm_secure: "
            "failed to create device class\n"
        );

        cdev_del(&vtpm_cdev);

        unregister_chrdev_region(
            vtpm_device_number,
            1
        );

        return PTR_ERR(vtpm_class);
    }


    /*
     * --------------------------------------------------------
     * Create device
     * --------------------------------------------------------
     */

    vtpm_device = device_create(
        vtpm_class,
        NULL,
        vtpm_device_number,
        NULL,
        DEVICE_NAME
    );

    if (IS_ERR(vtpm_device))
    {
        pr_err(
            "vtpm_secure: "
            "failed to create device\n"
        );

        class_destroy(vtpm_class);

        cdev_del(&vtpm_cdev);

        unregister_chrdev_region(
            vtpm_device_number,
            1
        );

        return PTR_ERR(vtpm_device);
    }


    pr_info(
        "vtpm_secure: "
        "driver loaded successfully\n"
    );

    pr_info(
        "vtpm_secure: "
        "device created: /dev/%s\n",
        DEVICE_NAME
    );


    return 0;
}


/*
 * ============================================================
 * DRIVER CLEANUP
 * ============================================================
 */

static void __exit vtpm_driver_exit(void)
{
    pr_info(
        "vtpm_secure: "
        "removing driver\n"
    );


    /*
     * Remove device
     */

    device_destroy(
        vtpm_class,
        vtpm_device_number
    );


    /*
     * Remove class
     */

    class_destroy(
        vtpm_class
    );


    /*
     * Remove character device
     */

    cdev_del(
        &vtpm_cdev
    );


    /*
     * Free device number
     */

    unregister_chrdev_region(
        vtpm_device_number,
        1
    );


    pr_info(
        "vtpm_secure: "
        "driver removed\n"
    );
}


/*
 * ============================================================
 * MODULE INFORMATION
 * ============================================================
 */

module_init(vtpm_driver_init);

module_exit(vtpm_driver_exit);


MODULE_LICENSE("GPL");

MODULE_AUTHOR("Secure VTPM Project");

MODULE_DESCRIPTION(
    "Secure Virtual TPM Linux Character Device Driver"
);

MODULE_VERSION("1.0");
