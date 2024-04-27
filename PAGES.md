# stt-pages
Fixed size allocation system, header only library

This is a optional sub-system of `stt-stl`. You must include `stt-pages.h` manually to use the system.


## Motivation
Often when storing data in a list or array we don't need the data to be stored contigiously. Just blindly stuffing things into a `std::vector` like container will result in allocations of varying sizes.

A solution to this problem is to:
1. Use allocations of fixed size blocks
2. Store data within the blocks
3. Store and reusue blocks in a pool 

By using fixed size blocks we prevent memory fragmentation. By linking them together in a linked list each individual block can be used as a local contigious store of data that then overflows into a next block.

Why not just use something like `tcmalloc`? The problem is that even really clever allocators still fragment memory - they allocate slabs and divide them up but when you free you can still have large slabs being kept alive by lingering small allocations. A better approach is to carve up the data into max size blocks and just use blocks

If an allocation is permanent than a fancy allocation system is not needed - just allocated once. So the strategy looks like as follows:
1. Is the allocation permanent? Then use the system allocator
2. IS the allocation huge? Then use the system allocator
3. Is the allocation very small? Then use Small Size Optimisations (eg, packing the data into a pointer like with most `std::string` implementations)
3. Is the allocation temporary? Then grab a 4k page and use that and return to the pool when done. "Wasteage" doesn't matter as the object has a short lifetime
4. Is the allocation medium term? Then you have to pack data into a page somehow. 


## Pages
I supply two kinds of pages with this library - regular (about 4k in size) and jumbo (64k). The reason is that I want to force you to use fixed size blocks of a known size and live with discontigious data. You can configure 

You can change the size of the pages by defining the `STT_PAGE_SIZE` and `STT_JUMBO_PAGE_SIZE` macros.

## Allocating 
The main interface for allocating and freeing pages is the `ThreadSafePageAllocator`. Internally this uses a thread_local pool of pages per thread and a global pool that is shared. When a thread_local pool gets exhausted then it'll move pages from the global pool to the thread_local pool, and if the thread_local pool has a surplus of pages then they will be returned to the global pool.

You can allocate (fetch from the pool) a page by calling `ThreadSafePageAllocator::allocPage` and `ThreadSafePageAllocator::freePage`. You can also use `allocPages/freePages` to batch allocate. 

*VERY IMPORTANT* - when you get a page from the allocator it is unitialzed data - this includes it's own header! Be sure to call `mPage->initHeader()` with your freshly allocated page!
A `page` is a union of `uint8_t[SIZE]` and a `pageHeader` class. When you



## Containers
* pageQueue<T> - Like a `std::vector` but discontigious and does not allow random access. You can `push_back` and use ranged itteration to access. 
* pageBump - A basic bump allocator, designed to be a bucket for objects with the same or similar lifetime

##

STT_PASSTHROUGH_TL_PAGE_ALLOCATOR

## Requirements:
C++17


