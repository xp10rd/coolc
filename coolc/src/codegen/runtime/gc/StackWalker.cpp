#include "StackWalker.hpp"
#include <cassert>

using namespace gc;

StackWalker *StackWalker::Walker = nullptr;

#ifdef LLVM_SHADOW_STACK
void ShadowStackWalker::process_roots(void *obj, void (*visitor)(void *obj, address *root, const address *meta),
                                      bool records_derived_ptrs)
{
    StackEntry *r = llvm_gc_root_chain;

    while (r)
    {
        assert(r->_map->_num_meta == 0); // we don't use metadata

        int num_roots = r->_map->_num_roots;
        for (int i = 0; i < num_roots; i++)
        {
            (*visitor)(obj, (address *)(r->_roots + i), NULL);
        }

        r = r->_next;
    }
}
#endif // LLVM_SHADOW_STACK

void StackWalker::init()
{
#ifdef LLVM_SHADOW_STACK
    Walker = new ShadowStackWalker;
#endif // LLVM_SHADOW_STACK

#ifdef LLVM_STATEPOINT_EXAMPLE
    Walker = new StackMapWalker;
#endif // LLVM_STATEPOINT_EXAMPLE
}

void StackWalker::release()
{
#if defined(LLVM_SHADOW_STACK) || defined(LLVM_STATEPOINT_EXAMPLE)
    delete Walker;
    Walker = nullptr;
#endif // LLVM_SHADOW_STACK || LLVM_STATEPOINT_EXAMPLE
}

#ifdef LLVM_STATEPOINT_EXAMPLE
StackMapWalker::StackMapWalker() : StackWalker()
{
    stackmap::StackMap::init();
}

StackMapWalker::~StackMapWalker()
{
    stackmap::StackMap::release();
}

void StackMapWalker::process_roots(void *obj, void (*visitor)(void *obj, address *root, const address *meta),
                                   bool records_derived_ptrs)
{
    if (records_derived_ptrs)
    {
        _derived_ptrs.clear();
    }

    address *stacktop = (address *)_stack_pointer;
    assert(stacktop);

#ifdef DEBUG
    if (TraceStackWalker)
    {
        fprintf(stderr, "\nStack pointer: %p\n", stacktop);
    }
#endif // DEBUG

    auto *const stackmap = stackmap::StackMap::map();
    assert(stackmap);

    // one slot after the top is return address of the gc_alloc
    auto *stackinfo = stackmap->info((address)stacktop[-1]);
    assert(stackinfo);

    int i = 0;
    while (stackinfo)
    {
#ifdef DEBUG
        if (TraceStackWalker)
        {
            fprintf(stderr, "%d: ret addr: %p, stack size 0x%x\n", i++, stacktop[-1], stackinfo->_stack_size);
        }
#endif // DEBUG

        for (const auto &offset : stackinfo->_offsets)
        {
            address *base_ptr_slot = (address *)((address)stacktop + offset._base_offset);
            address *derived_ptr_slot = (address *)((address)stacktop + offset._offset);

            if (base_ptr_slot == derived_ptr_slot)
            {
                (*visitor)(obj, base_ptr_slot, NULL);
            }
            else if (records_derived_ptrs)
            {
                _derived_ptrs.push_back({base_ptr_slot, derived_ptr_slot, (int)(*derived_ptr_slot - *base_ptr_slot)});
#ifdef DEBUG
                if (TraceStackWalker)
                {
                    fprintf(stderr, "Found derived ptr in %p, base ptr is in %p. ",
                            _derived_ptrs.back()._derived_ptr_slot, _derived_ptrs.back()._base_ptr_slot);
                    fprintf(stderr, "Derived ptr is %p, base is %p, offset = 0x%x\n",
                            *_derived_ptrs.back()._derived_ptr_slot, *_derived_ptrs.back()._base_ptr_slot,
                            _derived_ptrs.back()._offset);
                }
#endif // DEBUG
            }
        }

        stacktop = (address *)((address)stacktop + stackinfo->_stack_size +
                               sizeof(address)); // one slot shift to get return address
        stackinfo = stackmap->info((address)stacktop[-1]);
    }
}

void StackMapWalker::fix_derived_pointers()
{
    for (const auto &reloc : _derived_ptrs)
    {
        *(reloc._derived_ptr_slot) = *(reloc._base_ptr_slot) + reloc._offset;
    }
}

#endif // LLVM_STATEPOINT_EXAMPLE
