#include "Copy.h"


void copy()
{
    int i = 10;
    bool b = true;
    copy_stub(DeviceType::CPU,i, b); 
}

DEFINE_DISPATCH(copy_stub);
/*
struct copy_stub copy_stub;
*/