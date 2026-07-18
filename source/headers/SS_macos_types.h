/*
 *  SS_macos_types.h
 *  Minimal stand-in for the old Carbon/MacTypes.h integer typedefs that
 *  SS_Templates.h relied on. Carbon is gone from modern 64-bit macOS, so we
 *  just define the few fixed-width types the template code actually uses.
 *  (c) 2026 Scott Lahteine.
 */

#ifndef SS_MACOS_TYPES_H
#define SS_MACOS_TYPES_H

#include <cstdint>

typedef int16_t  SInt16;
typedef uint16_t UInt16;
typedef int32_t  SInt32;
typedef uint32_t UInt32;

#endif // SS_MACOS_TYPES_H
