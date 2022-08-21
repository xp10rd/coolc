#pragma once

#include "ShadowStack.hpp"
#include "codegen/runtime/ObjectLayout.hpp"
#include <queue>

namespace gc
{
/**
 * @brief Marker marks live objects
 *
 */
class Marker
{
  protected:
    gc_address _heap_start;
    gc_address _heap_end;

    virtual void mark() = 0;

    inline bool is_heap_addr(gc_address addr) const
    {
        return addr >= _heap_start && addr <= _heap_end;
    }

  public:
    /**
     * @brief Construct a new Marker object
     *
     * @param heap_start Start of the heap to be marked
     * @param heap_end End of the heap to be marked
     */
    Marker(gc_address heap_start, gc_address heap_end);

    virtual ~Marker()
    {
    }
};

class ShadowStackMarker : public Marker
{
  public:
    ShadowStackMarker(gc_address heap_start, gc_address heap_end) : Marker(heap_start, heap_end)
    {
    }

    /**
     * @brief Mark live objects from roots
     *
     */
    virtual void mark_from_roots() = 0;

    ~ShadowStackMarker()
    {
    }
};

class ShadowStackMarkerFIFO : public ShadowStackMarker
{
  protected:
    std::queue<ObjectLayout *> _worklist;

    void mark() override;

  public:
    ShadowStackMarkerFIFO(gc_address heap_start, gc_address heap_end) : ShadowStackMarker(heap_start, heap_end)
    {
    }

    void mark_from_roots() override;
};
} // namespace gc