/* This file is here to prevent a file conflict on multiarch systems. A
 * conflict will occur because ext2_types.h has arch-specific definitions.
 *
 * DO NOT INCLUDE THE NEW FILE DIRECTLY -- ALWAYS INCLUDE THIS ONE INSTEAD. */

#if defined(__i386__)
#include "ext2_types-i386.h"
#elif defined(__powerpc64__)
#include "ext2_types-ppc64.h"
#elif defined(__powerpc__)
#include "ext2_types-ppc.h"
#elif defined(__s390x__)
#include "ext2_types-s390x.h"
#elif defined(__s390__)
#include "ext2_types-s390.h"
#elif defined(__x86_64__)
#include "ext2_types-x86_64.h"
#elif defined(__alpha__)
#include "ext2_types-alpha.h"
#elif defined(__arm__)
#include "ext2_types-arm.h"
#elif defined(__sparc__) && defined(__arch64__)
#include "ext2_types-sparc64.h"
#elif defined(__sparc__)
#include "ext2_types-sparc.h"
#else
#error "This e2fsprogs-devel package does not work your architecture?"
#endif

