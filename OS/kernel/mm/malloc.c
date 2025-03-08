#include "os.h"

#define MEMORY_POOL_SIZE 1024 * 1024
// 假设有1MB的内存池

#define ALIGNMENT 16
// 定义内存对齐为16字节

#define MIN_BLOCK_SIZE 32

#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
// ALIGN宏确保给定的大小按照ALIGNMENT对齐

#define block_get_size(block_ptr) ((mem_block *)(block_ptr))->size
#define block_get_next(block_ptr) ((mem_block *)(block_ptr))->next
#define block_is_used(block_ptr) !((mem_block *)(block_ptr))->free

/*
 * Following functions SHOULD be called ONLY ONE time here,
 * so just declared here ONCE and NOT included in file os.h.
 */

typedef struct mem_block
{
    size_t size;            // 块的大小
    struct mem_block *next; // 指向下一个块的指针
    int free;               // 是否空闲
} mem_block;

static char memory_pool[MEMORY_POOL_SIZE];
static mem_block *free_list = (void *)memory_pool;

void *memset(void *ptr, int value, size_t num)
{
    unsigned char *p = ptr;
    while (num--)
    {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

void memory_init(void)
{
    free_list->size = MEMORY_POOL_SIZE - sizeof(mem_block);
    free_list->next = NULL;
    free_list->free = 1;
}

void *malloc(size_t size)
{
    size_t best_fit_size = MEMORY_POOL_SIZE;
    mem_block *best_fit_block = NULL;
    mem_block *current = free_list;

    size = ALIGN(size + sizeof(mem_block)); // 包括管理结构的大小

<<<<<<< Updated upstream:OS/malloc.c
    printf("请求分配 %d 字节的内存\n", size);

    while (current)
    {
        printf("检查块：地址=%p，大小=%d\n", (void *)current, current->size);
=======
    // printf("请求分配 %d 字节的内存\n", size);

    while (current)
    {
        // printf("检查块：地址=%p，大小=%d\n", (void *)current, current->size);
>>>>>>> Stashed changes:OS/kernel/mm/malloc.c
        if (current->free && current->size >= size)
        {
            size_t current_block_free_space = current->size - size;
            if (current_block_free_space < best_fit_size)
            {
                best_fit_size = current_block_free_space;
                best_fit_block = current;
            }
        }
        current = current->next;
<<<<<<< Updated upstream:OS/malloc.c
        printf("移动到下一个块：地址=%p\n", (void *)current);
=======
        // printf("移动到下一个块：地址=%p\n", (void *)current);
>>>>>>> Stashed changes:OS/kernel/mm/malloc.c
    }

    if (!best_fit_block)
    {
        printf("错误：没有足够的空间分配 %d 字节的内存\n", size);
        return NULL; // 没有足够的空间
    }

    if (best_fit_size <= sizeof(mem_block) + MIN_BLOCK_SIZE)
    {
        // 如果剩余空间不足以创建一个新的mem_block，则不分割，直接分配整个块
        best_fit_block->free = 0;
        best_fit_block->next = NULL;
    }
    else
    {
        // 分割内存块
        mem_block *new_block = (mem_block *)((char *)best_fit_block + size);
        new_block->size = best_fit_block->size - size;
        new_block->next = best_fit_block->next;
        new_block->free = 1;

        best_fit_block->size = size - sizeof(mem_block); // 更新当前块的大小，减去管理结构的大小
        best_fit_block->free = 0;
        best_fit_block->next = new_block;
    }

    // 初始化分配的内存块（不包括管理结构）
    void *allocated_memory = (void *)(best_fit_block + 1);
    memset(allocated_memory, 0, best_fit_block->size - sizeof(mem_block));

<<<<<<< Updated upstream:OS/malloc.c
    printf("分配了 %d 字节的内存\n", size);
    printf("分配后块：地址=%p，大小=%d\n", (void *)best_fit_block, best_fit_block->size);
    printf("新块：地址=%p，大小=%d\n\n", (void *)best_fit_block->next, best_fit_block->next ? best_fit_block->next->size : 0);
=======
    // printf("分配了 %d 字节的内存\n", size);
    // printf("分配后块：地址=%p，大小=%d\n", (void *)best_fit_block, best_fit_block->size);
    // printf("新块：地址=%p，大小=%d\n\n", (void *)best_fit_block->next, best_fit_block->next ? best_fit_block->next->size : 0);
>>>>>>> Stashed changes:OS/kernel/mm/malloc.c
    return allocated_memory;
}

void free(void *ptr)
{
    if (!ptr)
    {
        printf("警告：尝试释放NULL指针\n");
        return;
    }
    mem_block *block_to_free = (mem_block *)((char *)ptr - sizeof(mem_block));
    block_to_free->free = 1;

<<<<<<< Updated upstream:OS/malloc.c
    printf("释放块：地址=%p，大小=%d\n\n", (void *)block_to_free, block_to_free->size);
=======
    // printf("释放块：地址=%p，大小=%d\n\n", (void *)block_to_free, block_to_free->size);
>>>>>>> Stashed changes:OS/kernel/mm/malloc.c

    // 合并空闲块
    mem_block *current = free_list;
    mem_block *prev = NULL;
    while (current)
    {
        if (current == block_to_free)
        {
            // 如果前一个块是空闲的，则合并
            if (prev && prev->free)
            {
                prev->size += current->size + sizeof(mem_block);
                prev->next = current->next;
                current = prev; // 更新当前指针以指向合并后的块
            }
            // 检查并合并下一个空闲块
            if (current->next && current->next->free)
            {
                current->size += current->next->size + sizeof(mem_block);
                current->next = current->next->next;
            }
            break;
        }
        prev = current;
        current = current->next;
    }
}

void print_blocks(void)
{
    void *block_ptr = memory_pool;
    printf("-- start to print blocks --\n");
    do
    {
        printf("\tblock: %p, size: %d, used: %d\n", block_ptr,
               block_get_size(block_ptr), block_is_used(block_ptr));
        block_ptr = block_get_next(block_ptr);
    } while (block_ptr);
    printf("-- end to print blocks --\n");
}

void print_block(void *block_ptr)
{
    mem_block *block_info = (mem_block *)(block_ptr - sizeof(mem_block));
    void *block_end = block_ptr + block_info->size;
    int byte_count = 1;
    int *int_block_ptr = (int *)block_ptr;
    for (; int_block_ptr < block_end; int_block_ptr++, byte_count++)
    {
        printf("%d", (*int_block_ptr));
        if (byte_count % 4 == 0)
            printf(" ");
        if (byte_count % 32 == 0)
            printf("\n");
    }
    printf("\n\n");
}