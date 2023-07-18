#include "jit_type.h"
#include <iostream>

using namespace std;

int main()
{
    // Type * rp= new Type();
    // shared_ptr<Type> sp1(new Type());
    // shared_ptr<Type> sp2 = sp1->withContained();
    // shared_ptr<Type> sp3 = rp->withContained();

    shared_ptr<TensorType> sp1 = TensorType::create();
    shared_ptr<TensorType> sp2 = sp1->withContained<TensorType>();
    sp2->show();
}