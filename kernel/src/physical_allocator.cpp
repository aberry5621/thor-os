//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "physical_allocator.hpp"
#include "e820.hpp"
#include "paging.hpp"

//For problems during boot
#include "kernel.hpp"
#include "console.hpp"

namespace {

const e820::mmapentry* current_mmap_entry = 0;
uintptr_t current_mmap_entry_position = 0;

size_t allocated_memory = 0;

} //End of anonymous namespace

void physical_allocator::early_init(){
    e820::finalize_memory_detection();

    if(e820::mmap_failed()){
        k_print_line("e820 failed, no way to allocate memory");
        suspend_boot();
    }
}

size_t physical_allocator::early_allocate(size_t blocks){
    if(!current_mmap_entry){
        for(uint64_t i = 0; i < e820::mmap_entry_count(); ++i){
            auto& entry = e820::mmap_entry(i);

            if(entry.type == 1 && entry.base >= 0x100000 && entry.size >= 16384){
                current_mmap_entry = &entry;
                current_mmap_entry_position = entry.base;

                break;
            }
        }
    }

    if(!current_mmap_entry){
        return 0;
    }

    allocated_memory += blocks * paging::PAGE_SIZE;

    auto address = current_mmap_entry_position;

    current_mmap_entry_position += blocks * paging::PAGE_SIZE;

    //TODO If we are at the end of the block, we gonna have a problem

    return address;
}

void physical_allocator::init(){
    //TODO
}

size_t physical_allocator::allocate(size_t blocks){
    return early_allocate(blocks);
}

size_t physical_allocator::available(){
    return e820::available_memory();
}

size_t physical_allocator::allocated(){
    return allocated_memory;
}

size_t physical_allocator::free(){
    return available() - allocated();
}
