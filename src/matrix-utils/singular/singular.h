#ifndef _SINGULAR_SINGULAR_H
#define _SINGULAR_SINGULAR_H

/** The version of the singular library. */
#define SINGULAR_VERSION  "@PROJECT_VERSION@"

/**
 * Whether rvalue references are supported.
 *
 * Visual Studio 2010 and lower do not have rvalue references so far.
 */
#if defined(_MSC_VER) && _MSC_VER < 1700
#define SINGULAR_RVALUE_REFERENCE_SUPPORTED	0
#else
#define SINGULAR_RVALUE_REFERENCE_SUPPORTED	1
#endif

/**
 * Whether function deletions are supported.
 *
 * Visual Studio 2012 and lower do not like "delete" stuff so far.
 */
#if defined(_MSC_VER) && _MSC_VER < 1800
#define SINGULAR_FUNCTION_DELETION_SUPPORTED	0
#else
#define SINGULAR_FUNCTION_DELETION_SUPPORTED	1
#endif

/**
 * Whether template friend operator overalodings are supported.
 *
 * Visual Studio 2012 and lower do not like overloading a template firend
 * operators.
 * Neither does GCC.
 */
#if (defined(_MSC_VER) && _MSC_VER < 1800) || (defined(__GNUC__) && !defined(__clang__))
#define SINGULAR_TEMPLATE_FRIEND_OPERATOR_OVERLOADING_SUPPORTED	0
#else
#define SINGULAR_TEMPLATE_FRIEND_OPERATOR_OVERLOADING_SUPPORTED	1
#endif

#endif
