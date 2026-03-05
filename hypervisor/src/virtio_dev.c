#include "types.h"
#include "hyper_log.h"
#include "printf.h"
#include "vmmio.h"
#include "virtio_dev.h"

static struct {
    u32 device_features_sel;   /* 当前选择的设备特性字索引 */
    u32 driver_features_sel;   /* 当前选择的驱动特性字索引 */
    u32 driver_features[2];    /* 驱动写入的特性值*/
    u32 guest_page_size;       /* 保存客户页面大小 */
    u32 queue_num;             /* 队列大小 */
    u32 status;                /* 设备状态 */
} virtio_state = {0};

static int virtio_dev_read(struct vcpu *vcpu, u64 offset, u64 *val, struct vmmio_access *vmmio)
{
    //LOG_INFO("MMIO read offset=%d\n", offset);

    switch (offset) {
        case VIRTIO_MMIO_MAGIC_VALUE:
            *val = ('v' | 'i' << 8 | 'r' << 16 | 't' << 24);
            break;
        case VIRTIO_MMIO_VERSION:
            *val = 0x1;
            break;
        case VIRTIO_MMIO_DEVICE_ID:  // vierio test device
            *val = 46;
            break;
        case VIRTIO_MMIO_VENDOR_ID:
            *val = 0x554D4551;
            break;
        case VIRTIO_MMIO_DEVICE_FEATURES:
            if (virtio_state.device_features_sel == 0) {
                *val = 1;
            } else {
                *val = 0;
            }
            break;
        case VIRTIO_MMIO_STATUS:
            *val = virtio_state.status;
            break;
        default:
            abort("Unhandled MMIO read offset %d, returning 0\n", offset);
            *val = -1;
            break;
        }

    return 0;
}

static int virtio_dev_write(struct vcpu *vcpu, u64 offset, u64 val, struct vmmio_access *vmmio)
{
    //LOG_INFO("MMIO write offset=%d val=%d\n", offset, val);

    switch (offset) {
        case VIRTIO_MMIO_DEVICE_FEATURES_SEL:
            virtio_state.device_features_sel = val;
            break;
        case VIRTIO_MMIO_DRIVER_FEATURES:
            if (virtio_state.driver_features_sel < 2)
                virtio_state.driver_features[virtio_state.driver_features_sel] = val;
            break;
        case VIRTIO_MMIO_DRIVER_FEATURES_SEL:
            virtio_state.driver_features_sel = val;
            break;
        case VIRTIO_MMIO_GUEST_PAGE_SIZE:
            virtio_state.guest_page_size = val;
            LOG_INFO("Guest page size set to %d (legacy mode)\n", val);
            break;
        case VIRTIO_MMIO_QUEUE_NUM:
            virtio_state.queue_num = val;
            break;
        case VIRTIO_MMIO_STATUS:
            virtio_state.status = val;
            if (val & 0x8) {
                LOG_INFO("Driver OK - device ready\n");
            }
            break;
        default:
            abort("Unhandled MMIO write offset %d, returning 0\n", offset);
            break;
    }
    return 0;
}

void create_dev_mmio_trap(struct vm *vm)
{
    LOG_INFO("virtio mmio address: %d size: %d\n", VIRTIO_DEV_BASE, VIRTIO_DEV_SIZE);
    create_mmio_trap(vm, VIRTIO_DEV_BASE, VIRTIO_DEV_SIZE, virtio_dev_read, virtio_dev_write);
}
