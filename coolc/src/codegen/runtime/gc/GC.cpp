#include "GC.hpp"

using namespace gc;

GC *GC::Gc = NULL; // TODO: or nullptr?

ObjectLayout *GC::allocate(int tag, size_t size, void *disp_tab)
{
    ObjectLayout *object = _allocator->allocate(tag, size, disp_tab);
    if (object == NULL)
    {
        collect();
        object = _allocator->allocate(tag, size, disp_tab);
    }

    if (object == NULL)
    {
        _allocator->exit_with_error("cannot allocate memory for object!");
    }

    return object;
}

ObjectLayout *GC::copy(const ObjectLayout *obj)
{
    ObjectLayout *new_obj = allocate(obj->_tag, obj->_size, obj->_dispatch_table);
    assert(new_obj);

    size_t size = std::min(obj->_size, new_obj->_size) - Allocator::HEADER_SIZE; // because of allignemnt
    memcpy(new_obj->fields_base(), obj->fields_base(), size);

    return new_obj;
}

void GC::init(const GcType &type, const size_t &heap_size)
{
    switch (type)
    {
    case ZEROGC:
        Gc = new ZeroGC(heap_size);
        break;
    default:
        assert(false);
    };
}

// -------------------------------------- ZeroGC --------------------------------------
ZeroGC::ZeroGC(const size_t &size) : GC()
{
    _allocator = new Allocator(size);

    _heap_start = _allocator->start();
    _heap_end = _allocator->end();

    _marker = new ShadowStackMarkerFIFO(_heap_start, _heap_end);
}

ZeroGC::~ZeroGC()
{
    delete _allocator;
    delete _marker;
}