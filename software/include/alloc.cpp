#include "alloc.h"

#include "sys.h"
#include "../../shared/memory.h"

#define MAGIC 42859

// Overload heap allocation operators.
void* operator new(size_t size) {
    return malloc(size);
}

void* operator new[](size_t size) {
    return malloc(size);
}

void operator delete(void* ptr) {
    free(ptr);
};

void operator delete(void* ptr, size_t size) {
    free(ptr);
}
namespace heap {
    void init() {
        // In case the defaults for this weren't initialised.
        head_block = reinterpret_cast<BlockMeta*>(314572800);

        // Allocate an initial page.
        // Warning - temporarily changed to fixed address so malloc is confined to a specific process
        void* page = speedyos::alloc_virtual_pages((void*)314572800, 1, speedyos::PageAllocationFlag::ALLOCATE_IMMEDIATELY);
        if (page == nullptr) {
            speedyos::speedyshell::printf("[FATAL] Allocator could not allocate initial page.");
            speedyos::end_process(1);
            __builtin_unreachable();
        }


        // Create an initial head and tail entry for the page.
        BlockMeta* meta_head = (BlockMeta*)page;
        meta_head->magic = MAGIC;
        meta_head->size = 200;
        meta_head->allocated = false;

        BlockMeta* meta_tail = (BlockMeta*)((uint8_t*)page + (sizeof(BlockMeta) + meta_head->size));
        meta_head->magic = MAGIC;
        meta_head->size = 200; 
        meta_head->allocated = false;

        // Set the pointers.
        meta_head->next = meta_tail;
        meta_tail->prev = meta_head;
        meta_head->prev = nullptr;
        meta_tail->next = nullptr;

        // Set the appropriate entries.
        //head_block = meta_head;
        //tail_block = meta_tail;
    }
}

using namespace heap;

void* malloc(uint32_t size, bool reset) {
    BlockMeta* meta = head_block;
    
    // Find any suitable existing block.
    while (true) {
        // Check if block is suitable.
        if (meta->allocated || (size > meta->size || meta->size - size > 60)) {
            if (meta->next == nullptr) break;
            meta = meta->next;
            continue;
        }

        // Allocate the block.
        meta->allocated = true;
        
        // Clear data area if required.
        if (reset) memset(meta->data, 0, meta->size);

        // Return location.
        return meta->data;
    }

    uint8_t* next_free_area = meta->data + meta->size; //tail_block->data + tail_block->size;
    uint32_t remaining_page_size = 4096 - (reinterpret_cast<uint32_t>(next_free_area) % 4096);

    // If the remaining page area can fit the new block.
    if (sizeof(BlockMeta) + size <= remaining_page_size) {
        BlockMeta* new_meta = (BlockMeta*)next_free_area;
        new_meta->size = size;
        new_meta->allocated = true;
        new_meta->magic = MAGIC;
        new_meta->next = nullptr;
        new_meta->prev = meta;

        // Set new tail block.
        //tail_block->next = new_meta;
        //tail_block = new_meta;

        // Clear data area if required.
        if (reset) memset(new_meta->data, 0, new_meta->size);

        // Return location.
        return new_meta->data;
    } else {
        // Create a new block if remaining size is large enough.
        if (remaining_page_size >= sizeof(BlockMeta) + 20) {
            BlockMeta* new_meta = (BlockMeta*)next_free_area;
            new_meta->size = remaining_page_size - sizeof(BlockMeta);
            new_meta->allocated = true;
            new_meta->magic = MAGIC;
            new_meta->next = nullptr;
            new_meta->prev = meta; //tail_block;

            // Set new tail block.
            //tail_block->next = new_meta;
            //tail_block = new_meta;

            
        }

        // Calculate the minimum amount of pages required to fit.
        uint32_t required_pages = 
            ((size + sizeof(BlockMeta)) / 4096) + (((size + sizeof(BlockMeta)) % 4096) != 0);

        BlockMeta* new_meta = (BlockMeta*)speedyos::alloc_virtual_pages(nullptr, required_pages);
        if (new_meta == nullptr) [[unlikely]] return nullptr;

        new_meta->magic = MAGIC;
        new_meta->allocated = true;
        new_meta->size = size;
        new_meta->prev = meta; //tail_block;
        new_meta->next = nullptr;

        // Set new tail block.
        //tail_block->next = new_meta;
        //tail_block = new_meta;

        // Clear data area if required.
        if (reset) memset(new_meta->data, 0, new_meta->size);

        // Return location.
        return new_meta->data;
    }
}

bool free(void* address) {
    // Grab the block meta data.
    BlockMeta* old_meta = (BlockMeta*)(reinterpret_cast<uint8_t*>(address) - sizeof(BlockMeta));
    
    if (old_meta->magic != MAGIC) [[unlikely]] {
        speedyos::speedyshell::printf("[FATAL] Allocator or deallocation appears to be corrupted.");
        speedyos::end_process(1);
        __builtin_unreachable();
    }

    bool old_alloc_state = old_meta->allocated;

    // Set block as free.
    old_meta->allocated = false;

    // Check if page needs to be freed.

    /*void* page_start = (void*)(reinterpret_cast<uint32_t>(old_meta) - (reinterpret_cast<uint32_t>(old_meta) % 4096));
    void* page_end = (void*)((reinterpret_cast<uint32_t>(old_meta) + old_meta->size) + (4096 - ((reinterpret_cast<uint32_t>(old_meta) + old_meta->size) % 4096)));

    BlockMeta* meta = old_meta->next;

    while (meta != nullptr && (uint8_t*)meta + meta->size < page_end) {
        // If object inside of page is allocated.
        if (meta->allocated) {
            return old_alloc_state;
        }
    }*/

    return old_alloc_state;
}

bool allocated(void* address) {
    BlockMeta* meta = (BlockMeta*)(reinterpret_cast<uint8_t*>(address) - sizeof(BlockMeta));
    
    if (meta->magic != MAGIC) [[unlikely]] {
        speedyos::speedyshell::printf("[FATAL] Allocator or address appears to be corrupted.");
        speedyos::end_process(1);
        __builtin_unreachable();
    }

    return meta->allocated;
}

void* realloc(void* address, uint32_t size) {
    BlockMeta* meta = (BlockMeta*)(reinterpret_cast<uint8_t*>(address) - sizeof(BlockMeta));

    // Allocate new block.
    void* new_block = malloc(size);

    // Copy old data to new address.
    memcpy(new_block, address, meta->size);

    // Set remaining area to zero.
    memset(new_block, 0, size - meta->size);

    // Free old block.
    free(address);

    // Return new address.
    return new_block;
}