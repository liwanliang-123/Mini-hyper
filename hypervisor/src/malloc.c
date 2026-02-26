#include "malloc.h"
#include "utils.h"
#include "printf.h"
#include "hyper_log.h"
#include <errno.h>

extern blk_pool_t blk_pool_start[];
blk_pool_t  *sys_blk;

int blk_pool_init(blk_pool_t *pool, const char *name,
                  void *pool_start, size_t pool_size)
{
    uint32_t    blk_type;
    uint8_t     align_mask;
    blk_list_t *blk_list;

    if(pool == NULL || name == NULL || pool_start == NULL) {
        return -EINVAL;
    }

    memset(pool_start, 0, pool_size);

    align_mask = sizeof(uintptr_t) - 1u;

    if(((size_t)pool_start & align_mask) || (pool_size & align_mask)) {
        return -EINVAL;
    }

    if(pool_size % BLK_SLICE_SIZE) {
        return -EINVAL;
    }

    arch_spinlock_init(&pool->blk_lock);

    pool->pool_name  = name;
    pool->pool_start = (uintptr_t)pool_start;
    pool->pool_end   = (uintptr_t)(pool_start + pool_size);
    pool->slice_cnt    = 0;

    memset(pool->slice_type, 0, sizeof(pool->slice_type));

    for(blk_type = 0; blk_type < BLK_SLICE_BIT; blk_type++) {
        blk_list = &pool->blk_list[blk_type];
        memset(blk_list, 0, sizeof(*blk_list));
        blk_list->blk_size = BLK_TYPE2SIZE(blk_type);
    }

    return 0;
}

int malloc_init(void)
{
    int ret = blk_pool_init(blk_pool_start, "xmalloc-pool",
                       (void *)((size_t)blk_pool_start + ALIGN_UP(sizeof(blk_pool_t))),
                       CONFIG_BLK_SIZE);
    if (!ret) {
        // LOG_INFO("malloc have been initialized: \n");
        // LOG_INFO("malloc: pool_start : %x\n", blk_pool_start);
        // LOG_INFO("malloc: pool_size  : %x\n", CONFIG_BLK_SIZE);
        sys_blk = blk_pool_start;
    }

    return ret;
}

void *blk_alloc(blk_pool_t *pool, uint32_t size)
{
    uint32_t     blk_type;
    blk_list_t  *blk_list = NULL;
    uintptr_t    avail_blk = (uintptr_t)NULL;

    size = size < sizeof(uintptr_t) ? sizeof(uintptr_t) : size;
    /* 计算size所属的blk_type */
    blk_type = BLK_SIZE2TYPE(size);

    /* 遍历blk_list */
    while(blk_type < BLK_SLICE_BIT) {
        blk_list = &(pool->blk_list[blk_type]);

        /* 当前blk_type中有free object     */
        if((avail_blk = blk_list->free_head) != (uintptr_t)NULL) {
            blk_list->free_head = *(uintptr_t *)avail_blk;
            blk_list->freelist_cnt--;
            break;
        }

        /* 判断是否需要新的slice (该blk_type没有分配过slice或者之前分配的slice已经用完了) */
        if(blk_list->slice_addr == 0 || blk_list->slice_offset == BLK_SLICE_SIZE) {
            /* 内存池切片用完了，则使用更大的blk */
            if(pool->slice_cnt ==  BLK_SLICE_NUM) {
                blk_type++;
                continue;
            }

            /* 获取该blk type下的新的slice切片 */
            blk_list->slice_addr = pool->pool_start + pool->slice_cnt * BLK_SLICE_SIZE;
            /* 记录该切片属于哪个blk type */
            pool->slice_type[pool->slice_cnt] = blk_type;
            blk_list->slice_offset = 0;
            pool->slice_cnt++;
            blk_list->slice_cnt++;
        }

        /* cut blk from slice */
        avail_blk = blk_list->slice_addr + blk_list->slice_offset;
        blk_list->slice_offset += blk_list->blk_size;
        break;
    }

    if(blk_list) {
        if(avail_blk == (uintptr_t)0) {
            abort("malloc failed to get available blk");
        } else {
            blk_list->nofree_cnt++;
        }
    }

    return (void*)avail_blk;
}

int blk_free(blk_pool_t *pool, void *blk)
{
    uint32_t    slice_idx;
    uint32_t    blk_type;
    blk_list_t *blk_list;

    /* 找到释放的内存地址属于哪个切片 */
    slice_idx = ((uintptr_t)blk - pool->pool_start) >> BLK_SLICE_BIT;
    if(slice_idx >= BLK_SLICE_NUM) {
        return -EPERM;
    }

    blk_type = pool->slice_type[slice_idx];
    if(blk_type >= BLK_SLICE_BIT) {
        return -EPERM;
    }

    blk_list = &(pool->blk_list[blk_type]);
    *((uintptr_t *)blk) = blk_list->free_head;
    blk_list->free_head = (uintptr_t)blk;
    blk_list->nofree_cnt--;
    blk_list->freelist_cnt++;

    return 0;
}

int malloc_blk_free(blk_pool_t *pool, void *blk)
{
    int ret;
    if(pool == NULL || blk == NULL) {
        return -EINVAL;
    }

    arch_spin_lock(&pool->blk_lock);

    ret = blk_free(pool, blk);

    arch_spin_unlock(&pool->blk_lock);
    return ret;
}

void *malloc_blk_alloc(blk_pool_t *pool, uint32_t size)
{
    uintptr_t  avail_blk;

    if(pool == NULL) {
        return NULL;
    }

    arch_spin_lock(&pool->blk_lock);

    avail_blk = (uintptr_t)blk_alloc(pool, size);

    arch_spin_unlock(&pool->blk_lock);

    return (void *)avail_blk;
}

void *malloc(uint32_t size)
{
    return malloc_blk_alloc(sys_blk, size);
}

int free(void *ptr)
{
    return malloc_blk_free(sys_blk, ptr);
}
