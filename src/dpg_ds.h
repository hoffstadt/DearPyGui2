/*
   pl_ds.h
     * data structures
*/

/*
Index of this file:
// [SECTION] header mess
// [SECTION] includes
// [SECTION] public api
// [SECTION] internal
*/

#ifndef DPG_DS_H
#define DPG_DS_H

//-----------------------------------------------------------------------------
// [SECTION] header mess
//-----------------------------------------------------------------------------

#ifndef DPG_ASSERT
#include <assert.h>
#define DPG_ASSERT(x) assert((x))
#endif

//-----------------------------------------------------------------------------
// [SECTION] includes
//-----------------------------------------------------------------------------

#include <stdint.h> // uint32_t
#include <stdlib.h> // malloc, free
#include <string.h> // memset

//-----------------------------------------------------------------------------
// [SECTION] public api
//-----------------------------------------------------------------------------

#define dpg__sb_header(buf) ((dpgSbHeader_*)(((char*)(buf)) - sizeof(dpgSbHeader_)))
#define dpg_sb_capacity(buf) ((buf) ? dpg__sb_header((buf))->uCapacity : 0u)
#define dpg_sb_size(buf) ((buf) ? dpg__sb_header((buf))->uSize : 0u)
#define dpg_sb_pop(buf) (buf)[--dpg__sb_header((buf))->uSize]
#define dpg_sb_top(buf) ((buf)[dpg__sb_header((buf))->uSize-1])
#define dpg_sb_free(buf) if((buf)){ free(dpg__sb_header(buf));} (buf) = NULL;
#define dpg_sb_reset(buf) if((buf)){ dpg__sb_header((buf))->uSize = 0u;}
#define dpg_sb_back(buf)  dpg_sb_top((buf))
#define dpg__sb_may_grow(buf, s, n, m) dpg__sb_may_grow_((void**)&(buf), (s), (n), (m))
#define dpg_sb_push(buf, v) (dpg__sb_may_grow((buf), sizeof(*(buf)), 1, 8), (buf)[dpg__sb_header((buf))->uSize++] = (v))
#define dpg_sb_reserve(buf, n) (dpg__sb_may_grow((buf), sizeof(*(buf)), (n), (n)))
#define dpg_sb_resize(buf, n) (dpg__sb_may_grow((buf), sizeof(*(buf)), (n), (n)), memset((buf), 0, sizeof(*(buf)) * (n)), dpg__sb_header((buf))->uSize = (n))

//-----------------------------------------------------------------------------
// [SECTION] internal
//-----------------------------------------------------------------------------

typedef struct
{
    uint32_t uSize;
    uint32_t uCapacity;
} dpgSbHeader_;

static void
dpg__sb_grow(void** ptrBuffer, size_t elementSize, size_t newItems)
{

    dpgSbHeader_* ptrOldHeader = dpg__sb_header(*ptrBuffer);

    dpgSbHeader_* ptrNewHeader = malloc((ptrOldHeader->uCapacity + newItems) * elementSize + sizeof(dpgSbHeader_));
    if(ptrNewHeader)
    {
        ptrNewHeader->uSize = ptrOldHeader->uSize;
        ptrNewHeader->uCapacity = ptrOldHeader->uCapacity + (uint32_t)newItems;
        memcpy(&ptrNewHeader[1], *ptrBuffer, ptrOldHeader->uSize * elementSize);
        free(ptrOldHeader);
        *ptrBuffer = &ptrNewHeader[1];
    }
}

static void
dpg__sb_may_grow_(void** ptrBuffer, size_t elementSize, size_t newItems, size_t minCapacity)
{
    if(*ptrBuffer)
    {   
        dpgSbHeader_* ptrOriginalHeader = dpg__sb_header(*ptrBuffer);
        if(ptrOriginalHeader->uSize + elementSize > ptrOriginalHeader->uCapacity)
        {
            dpg__sb_grow(ptrBuffer, elementSize, newItems);
        }
    }
    else // first run
    {
        dpgSbHeader_* ptrHeader = malloc(minCapacity * elementSize + sizeof(dpgSbHeader_));
        if(ptrHeader)
        {
            *ptrBuffer = &ptrHeader[1]; 
            ptrHeader->uSize = 0u;
            ptrHeader->uCapacity = (uint32_t)minCapacity;
        }
    }     
}

#endif // DPG_DS_H