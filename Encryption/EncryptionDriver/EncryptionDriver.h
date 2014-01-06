#include <fltKernel.h>
#include <suppress.h>
#include <Ntstrsafe.h>
#include "DriverShared.h"

#define UCA_POOL_TAG 'FacU'

#define ASSERT_IRQL_LESS_OR_EQUAL(x)    FLT_ASSERT(KeGetCurrentIrql()<=(x))
#define ASSERT_IRQL_EQUAL(x)            FLT_ASSERT(KeGetCurrentIrql()==(x))
#define ASSERT_IRQL_LESS(x)             FLT_ASSERT(KeGetCurrentIrql()<(x))

