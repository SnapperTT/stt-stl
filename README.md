# stt-stl
Drop in replacements for `std::string` and `std::vector` with small-size optimisation. Header only library


## Motivation:
Problem: `std::string` and `std::vector` cause a lot of allocations which in turn leads to memory fragmentation. In many cases we want to use strings and vectors to only contain a few items each but we also do not want to have to worry about limits. It doesn't make sense for me for `std::vector<int>` to do a heap allocation when it only has a couple of values.


Second problem: Temporaries. Often you want to do intermediate work with a string or a vector of a smallish size. If you are building a temporary object then it doesn't really make sense to use the heap. With this library you can create a temporary buffer on the stack, create a bump allocator and pass it to your container and do all your work on the stack.


Third problem: allocators. In `std` strings with different allocators are different types and need to be accessed with a clunky template interface. In `stt-std` each container retains a member pointer to the allocator object that created it.


## Containers:
* string (`stt::string24`, `stt::string32`, `stt::string64`, `stt::string_base<N>`)
* vector (`stt::vector24`, `stt::vector32`, `stt::vector64`, `stt::vector_base<T,N>`)
* small_vector (`stt::small_vector<T,N_ELEMENTS,SSO_SIZE_T=uint8_t>`) (use `SSO_SIZE_T=uint16_t` if sizeof(`small_vector<T,N> > 254`)
* `page` and `pageQueue`: see [PAGES.md](PAGES.md) for more details

## Requirements:
C++17


## Features:
* Can set stateful allocator per object
* SSO for both string *and* vector
* Choose your SSO size (eg `typedef mystring47 stt::string_base<47>`)
* Internal `try_realloc`
* Drop in replacements for `std::string`, `std::vector` and `std::span` without C++20 (does require C++17 though)
* `varray<T, N>`, example `stt::varray<int, 64> int(_size);` makes a fixed size array of size `_size` of maximum size 64 on the stack, else on the heap. Drop in replacement for Variable Length Arrays (vla).
* `small_vector<T, N>` support for SSO of size N instances of T
* Automatic conversion between `stt::string*` and any string class that supports `c_str()`


## Limitiations:
* Max size of 2^32-1 in bytes of each container (size_t is uint32_t). If you need more than 4GB then use another container.

## Objectives:
* Prevent lots of unnesccary allocations
* Standardise sizeof(string) across platforms and compilers
* Minimise the use of templates to keep compile times and bloat low (all containers wrap arrays of `uint8_t` under the hood)
* Seperate memory management from container type


## Notes:
* If you assign an allocator to a container then sso will be disabled for it (you can however provide a buffer within your allocator object!)
* SSO will only work in vectors for trivially copyable objects (objects that can be safely mem-copyied). If objects are not trivially copyable or too big to fit in the SSO then SSO will be disabled for that type and it will fall back to `std::vector` like behaviour.
* Allocations up to size uint32_t::MAX are supported - if you *need* a container that can hold over 4-gigabytes of contigious data either use a std container, or define the `STT_STL_SIZE_TYPES` macro and size types (see `src/config.lzz`)

## Move semantics with custom allocators:
As data structures use stateful allocators we must be aware of some ambiguities
* If two containers have different allocators assigned then you cannot `std::move` between them. You can change this behaviour with the `STT_STL_CONTAINER_ALLOC_MOVE_MODE` macro (see `src/config.lzz`). Alternatively you can call `dst.allocator_aware_move_assign(std::move(src), false)` to force a move and skip checking.
* `stt::vector<T>::push_back(T&& t)` will use `t`'s custom allocator if t is a stt-stl container (eg `stt::string`), and it has a custom allocator set. It will then do a move assign internally. You can disable this by using `push_back(const T& t)`, or by not using a custom allocator for t 
* We use the following convention:
	- Move Construct = intialise dst with src. Dst will be set to use src's allocator and the data will be transferred over.
	- Move Assign: Move the *data* from src to dst. This is invalid if the internal allocators don't match
* This behaviour is because move-construct is needed when a container is part of a larger struct, and that larger struct is pushed into a container itself. But the usual intent by move assign is to move the internal data from object src to object dst - it is not clear if the intent is to store the data with dst's allocator or src's allocator. So this is only permitted when the allocators are the same

```C++
	// either move or copy contained data
	stt::string a; a.setAllocator(alloc)
	stt::string b;
	if (a.getCustomAllocator() == b.getCustomAllocator())
		b = std::move(a); // ok!
	else
		b = a; // cannot std::move, instead copy b's contents into a using a's allocator
	
	// move by structure
	stt::string a; a.setAllocator(alloc);
	stt::string b(std::move(a)); // <--- make B the new A, move the structure. A is now invalid
	assert(b.getCustomAllocator() == alloc); // true;
	
	// force std::move by structure
	stt::string a; a.setAllocator(alloc)
	stt::string b;
	b.allocator_aware_move_assign(std::move(a), false); // <-- move A->B, skipping allocator checks. I assume you know what you're doing here
	assert(b.getCustomAllocator() == &alloc); // true;
```

None of this matters if you are not using custom allocators.

## Building:
This is a single header library. `#include "stt-stl.hh"`, and `#define STT_STL_IMPL 1` in ONE compilation unit.

To modify source you need [lzz](https://github.com/SnapperTT/lzz-bin).


## Usage:
Hello world example:

```C++
#define STT_STL_IMPL 1
#include "stt-stl.h"

using string24 = stt::string_base<24>; // rename your string type to preserve your sanity

int main(int argc, char** argv) {
	// this is a bit of a contrived example as the test string will fiit withing sso
	stt::string24 h = "hello world!"
	std::cout << h << std::endl;
	
	return 0;
	}
```


## Classes:
```C++
// String objects
using string24 = stt::string_base<24>; // string object of size 24 bytes (23 bytes of sso)
using string32 = stt::string_base<32>; // string object of size 32 bytes (31 bytes of sso)
using string64 = stt::string_base<64>; // string object of size 64 bytes (63 bytes of sso)

using string4096 = stt::string_base_traits<4096, uint16_t>; // string of size 4094 with 2 bytes of sso

// Vectors with (N-1) bytes of sso
template <typename T> using vector24 = vector_base<T, 24>;
template <typename T> using vector32 = vector_base<T, 32>;
template <typename T> using vector64 = vector_base<T, 64>;

// Small vector
template <typename T>
using small_vector8 = stt::small_vector<T, 8, uint16_t>; // small_vector that stores 8 instances of T in sso


```


## Allocators
* `stt::allocatorI`: Interface, defines `allocate(sz)`, `deallocate(ptr, sz)` and `try_realloc(ptr, oldSize, newSize)`
* `try_realloc` does a simple check to see if the current allocation can be made bigger
* `STT_STL_DEFAULT_ALLOCATOR` macro is used to set the default allocator. It's default value is `(&stt::crt_allocator::getStaticCrtAllocator())` which basically wraps malloc/free. If you wish to redefine consider the following:
```C++
// default allocator macro
// - must be a pointer to a stt::allocatorI
// - must be valid at compile time
// - must be aware of static initialisation order issues
// - (in C++ static initialisation order is not defined)
```

## Config
See config.lzz


# Examples:


## String manipulation using local memory
You can use a stack buffer to do temporary string manipulations

```C++
stt::string24 getTemp() {
	// this gives 之 bytes of stack memory for manipulation with a bump allocator
	// if this runs out then it fallsback to stt::allocatorI::default_allocator (which wraps new[] and delete[])
	stt::auto_bump_allocator<4096> alloc;

	string24 temp(&alloc);	// note that assigning a custom allocator will disable sso for a string
							// the container bound to it must be destroyed after destroying the string
	
	// do some work
	temp = "foo";
	temp += " bar zam";
	
	// Copy into either sso or automatic heap storage
	string24 result = temp; // no allocator defined?  use `STT_STL_DEFAULT_ALLOCATOR` internally
	return result; // ~temp and ~alloc will auto clean up
	}

```

