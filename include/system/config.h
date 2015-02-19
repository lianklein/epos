// EPOS Configuration Engine

#ifndef __config_h
#define __config_h

//============================================================================
// DEFINITIONS
//============================================================================
#define __BEGIN_SYS             namespace EPOS {
#define __END_SYS               }
#define _SYS                    ::EPOS

#define ASM                     __asm__ __volatile__

#define __HEADER_ARCH(X)            <arch/ARCH/X.h>
#define __HEADER_MACH(X)            <mach/MACH/X.h>
#define __HEADER_APPLICATION_T(X)   <../app/X##_traits.h>
#define __HEADER_APPLICATION(X)     __HEADER_APPLICATION_T(X)

//============================================================================
// ARCHITECTURE, MACHINE, AND APPLICATION SELECTION
// This section is generated automatically from makedefs
//============================================================================
#define ARCH ia32
#define __ARCH_TRAITS_H	        __HEADER_ARCH(traits)

#define MACH pc
#define __MACH_TRAITS_H	        __HEADER_MACH(traits)

#define APPLICATION ping
#define __APPLICATION_TRAITS_H  __HEADER_APPLICATION(APPLICATION)

//============================================================================
// ASSERT (for pre and post conditions)
//============================================================================
#define assert(expr)    ((expr) ? static_cast<void>(0) : Assert::fail (#expr, __FILE__, __LINE__, __PRETTY_FUNCTION__))
//#define assert(expr)    (static_cast<void>(0))

//============================================================================
// CONFIGURATION
//============================================================================
#include <system/types.h>
#include <system/meta.h>
#include __APPLICATION_TRAITS_H
#include <system/info.h>

//============================================================================
// THINGS EVERBODY NEEDS
//============================================================================
#include <utility/ostream.h>
#include <utility/debug.h>

#endif
