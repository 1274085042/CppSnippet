#include "DispatchStub.h"

using copy_fn = void (*)(int &, bool non_blocking);

void copy();

DECLARE_DISPATCH(copy_fn, copy_stub);