#pragma once
#include "ATen/CUDAHooksInterface.h"
#include <iostream>

struct CUDAHooks : public CUDAHooksInterface 
{
  CUDAHooks() {}
  void initCUDA() const override;
};

const CUDAHooksInterface& getCUDAHooks();