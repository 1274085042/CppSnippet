#include "DispatchStub.h"

using copy_fn = void (*)(int &, bool non_blocking);

void copy();

DECLARE_DISPATCH(copy_fn, copy_stub);

/*
struct copy_stub : DispatchStub<copy_fn> 
{
    copy_stub() = default; 
    copy_stub(const copy_stub&) = delete; 
    copy_stub& operator=(const copy_stub&) = delete; 
}; 

extern struct copy_stub copy_stub;
*/