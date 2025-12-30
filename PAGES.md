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
I supply three kinds of pages with this library - regular (about 4k in size), jumbo (64k) and mega(2M). The reason is that I want to force you to use fixed size blocks of a known size. 

You can change the size of the pages by defining the `STT_PAGE_SIZE`, `STT_JUMBO_PAGE_SIZE` and `STT_MEGA_PAGE_SIZE` macros.


## Allocating 
The main interface for allocating and freeing pages is the `ThreadSafePageAllocator`. Internally this uses a thread_local pool of pages per thread and a global pool that is shared. When a thread_local pool gets exhausted then it'll move pages from the global pool to the thread_local pool, and if the thread_local pool has a surplus of pages then they will be returned to the global pool.

You can allocate (fetch from the pool) a page by calling `ThreadSafePageAllocator::allocPage` and `ThreadSafePageAllocator::freePage`. You can also use `allocPages/freePages` to batch allocate. 

*VERY IMPORTANT* - when you get a page from the allocator it is unitialzed data - this includes it's own header! Be sure to call `mPage->initHeader()` with your freshly allocated page!
A `page` is a union of `uint8_t[SIZE]` and a `pageHeader` class. When you allocate the a page the header is full of uninitialised garbage, this is so that you can initialise when you actually use it to prevent unnessecary access from system memory. Also when you free the page object you must make sure that the `pageHeader::next` pointer is not garbage (`initHeader` will set this to zero)

## Automatic Allocation
I've provided an allocator `AutoPageAllocator` (file: `self_releasing_page.lzz`) that can create a page object (either a Page, JumboPage or MegaPage) or a heap allocation for really big allocations, and wraps it in a uint8_t* pointer. This is good for storing data with a medium lifetime. This allocator has no internal state and is thread safe - internally it wraps `stt::ThreadSafePageAllocator`.

```
// Using directly
uint8_t* mem = stt::AutoPageAllocator::I.allocate(size_bytes);
bool success = stt::AutoPageAllocator::I.try_realloc(mem, size_bytes, new_size_bytes);
stt::AutoPageAllocator::I.free(mem);

// Using with containers
stt::vector32 vec;
vec.setAllocator(&stt::AutoPageAllocator::I);
```

How it works is that when you call `allocate(..)`, a Page is pulled from ThreadSafePageAllocator, various metadata is set and then `Page->ptr()` is returned. When you call `try_realloc(...)` or `free(...), the pointer to the Page is reconstructed and we can act on it accordingly. For really large allocations (bigger than Mega) then a heap buffer with enough room for a `pageHeader` at the start.


## Per Thread Tuning
Thread_local pools can have custom sized pools. By default they request 10 pages at a time and a hold a max of 20 pages. If this max is reached everything above the lower limit is returned to the global pool, if an allocation is requested then and the less than request. You can set custom values for this, This is useful if you have threads that are mainly producers (use high requestSize) and threads that are mainly consumers (use a low maxCacheSize).

Requesting from the global backend is a mutex locked operation so try to avoid it where possible. If you do a bulk allocation that then it will request the minimum request PLUS your bulk, so if the request size is 10 and you request 31 pages then an empty thread local pool will request 10+31 pages from the backend. Using `stt::ThreadSafePageAllocator::bulkAllocate` should be done if you know how many pages you need in advance.

You can access the thread_local pool objects with:
```
PATL_Data* PA = stt::ThreadSafePageAllocator::getThreadLocalAllocators()
//or
ThreadLocalPagePool* pageAlloc = ThreadSafePageAllocator::getThreadLocalPool(pageTypeEnum::PAGE_TYPE_NORMAL);
```
And the global backend pool with:
```
BackendPagePool* BP = ThreadSafePageAllocator::getBackendPool(pageTypeEnum::PAGE_TYPE_NORMAL);
```

And you can tune them with, eg:
```
PA->pageAlloc.setMinMaxFreelistPages(requestAmount, maxCacheSize); // setting values
PA->pageAlloc.getMinMaxFreelistPages(requestAmount, maxCacheSize); // reading values
PA->pageAlloc.returnAllPages(); // flush everything now
```

You can flush the global pool back to system memory with `stt::ThreadSafePageAllocator::cleanupBackendPools()`. Flushing pages is a series of lock-free atomic operation.

Finally you can change the (minimum) batch size for the `BackendPagePool` by the `BackendPagePool::batchSize` variable. The backend is never flushed back to system memory by default, allocated pages live forever (by design we are trying to avoid system `malloc`/`free` by recycling pages). But they can be forced flushed back to system memory by calling `BackendPagePool::freeAllToSystem()` 


## Containers
* `pageQueue<T>` - Like a `std::vector` but discontigious and does not allow random access. You can `push_back` and use ranged itteration to access. 
* `pageQueueBumpStorage<P>` - A basic bump allocator, designed to be a bucket for objects with the same or similar lifetime. (This is perfect for interned strings)! P here is the page type (regular or jumbo). This also has an overflow mechanisim to store objects that are individually too big to fit in the allocator (these will use the STT_STL_DEFAULT_ALLOCATOR) 


## Debugging
You can define `STT_PASSTHROUGH_TL_PAGE_ALLOCATOR` or `STT_PASSTHROUGH_TL_PAGE_ALLOCATOR_BACKEND` to bypass the thread_local or backend page pools and just call `new`/`delete` directly


## Requirements:
C++17


