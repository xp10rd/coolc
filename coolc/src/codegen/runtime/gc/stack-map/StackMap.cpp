#include "StackMap.hpp"
#include <cassert>
#include <cstdio>

using namespace gc::stackmap;

StackMap *StackMap::Map = nullptr;

StackMap::StackMap()
{
    Header *hdr = (Header *)&__LLVM_StackMaps;

    assert(hdr->_version == 3);
    assert(hdr->_num_constants == 0);
    assert(hdr->_reserved0 == 0);
    assert(hdr->_reserved1 == 0);

    StkSizeRecord *funcs = (StkSizeRecord *)((address)&__LLVM_StackMaps + sizeof(Header));
    address recrds = (address)(funcs + hdr->_num_functions);

    for (int i = 0; i < hdr->_num_functions; i++)
    {
        StkSizeRecord &func = funcs[i];

        for (int j = 0; j < func._record_count; j++)
        {
            StkMapRecord *stkmap = (StkMapRecord *)recrds;

            assert(stkmap->_reserved == 0);
            recrds += sizeof(StkMapRecord);

            AddrInfo &info = _stack_maps[func._func_address + stkmap->_instruction_offset];
            info._stack_size = func._stack_size;

            assert(stkmap->_num_locations >= 3); // skip the first three locs

            for (int k = 0; k < 3; k++)
            {
                Location *lock = (Location *)recrds;

                assert(lock->_reserved1 == 0);
                assert(lock->_type == LocationType::Constant);
                assert(lock->_offset_or_small_constant == 0);

                recrds += sizeof(Location);
            }

            assert((stkmap->_num_locations - 3) % 2 == 0);
            for (int k = 3; k < stkmap->_num_locations; k += 2)
            {
                Location *lock1 = (Location *)recrds;
                Location *lock2 = (Location *)(recrds + sizeof(Location));

                assert(lock1->_reserved1 == 0);
                assert(lock1->_type == LocationType::Indirect);
                assert(lock1->_dwarf_reg_num == DWARFRegNum::SP);
                assert(lock1->_location_size == 8);

                assert(lock2->_reserved1 == 0);
                assert(lock2->_type == LocationType::Indirect);
                assert(lock2->_dwarf_reg_num == DWARFRegNum::SP);
                assert(lock2->_location_size == 8);

                LocInfo info1;

                info1._base_offset = lock1->_offset_or_small_constant;
                info1._offset = lock1->_offset_or_small_constant;

                info._offsets.push_back(info1);

                if (lock1->_offset_or_small_constant != lock2->_offset_or_small_constant)
                {
                    LocInfo info2;

                    info2._base_offset = lock1->_offset_or_small_constant;
                    info2._offset = lock2->_offset_or_small_constant;

                    info._offsets.push_back(info2);
                }

                recrds += 2 * sizeof(Location);
            }

            recrds = (address)(((uint64_t)recrds + 0x7) & ~0x7);

            StkMapRecordTail *tail = (StkMapRecordTail *)(recrds);
            assert(tail->_num_live_outs == 0); // skip all LiveOuts
            recrds += sizeof(StkMapRecordTail);

            recrds = (address)(((uint64_t)recrds + 0x7) & ~0x7);
        }
    }

#ifdef DEBUG
    if (PrintStackMaps)
    {
        for (const auto &safepoint : _stack_maps)
        {
            fprintf(stderr, "Safepoint address: %p\n", safepoint.first);
            fprintf(stderr, "Stack size: %d\n", safepoint.second._stack_size);
            int i = 0;
            for (const auto &offset : safepoint.second._offsets)
            {
                fprintf(stderr, "Offset %d: 0x%x, base offset = 0x%x\n", i, offset._offset, offset._base_offset);
                i++;
            }
            fprintf(stderr, "\n");
        }
    }
#endif // DEBUG
}

const AddrInfo *StackMap::info(address ret) const
{
    const auto info = _stack_maps.find(ret);
    if (info != _stack_maps.end())
    {
        return &info->second;
    }

    return nullptr;
}

void StackMap::init()
{
    Map = new StackMap;
}

void StackMap::release()
{
    delete Map;
}