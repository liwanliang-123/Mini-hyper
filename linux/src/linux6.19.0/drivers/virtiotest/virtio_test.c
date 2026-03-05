#include <linux/virtio.h>
#include <linux/virtio_ids.h>

#include "virtio_test.h"

static unsigned int features[] = {
    VIRTIO_TEST_F_CAN_PRINT,
};

static struct virtio_device_id id_table[] = {
    { VIRTIO_ID_TEST, VIRTIO_DEV_ANY_ID },
    { 0 },
};

static int virttest_validate(struct virtio_device *vdev)
{
    dev_info(&vdev->dev, "==========> func: %s line: %d\n", __func__, __LINE__);
    return 0;
}

static int virttest_probe(struct virtio_device *vdev)
{
    dev_info(&vdev->dev, "==========> func: %s line: %d\n", __func__, __LINE__);
    return 0;
}

static void virttest_remove(struct virtio_device *vdev)
{
    dev_info(&vdev->dev, "==========> func: %s line: %d\n", __func__, __LINE__);
}

static struct virtio_driver virtio_test_driver = {
    .feature_table = features,
    .feature_table_size = ARRAY_SIZE(features),
    .driver.name =  KBUILD_MODNAME,
    .driver.owner = THIS_MODULE,
    .id_table = id_table,
    .validate = virttest_validate,
    .probe = virttest_probe,
    .remove = virttest_remove,
};

module_virtio_driver(virtio_test_driver);

MODULE_DEVICE_TABLE(virtio, id_table);
MODULE_DESCRIPTION("Virtio test driver");
MODULE_LICENSE("GPL");
