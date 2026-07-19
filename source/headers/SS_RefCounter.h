/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_RefCounter.h
 *
 *  $Id: SS_RefCounter.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_REFCOUNTER_H__
#define __SS_REFCOUNTER_H__

#define REF_DEBUG 0

#include <string.h>
#include <string>
#include <stdio.h>

// SS_ASSERT / SS_ASSERT_FINITE are defined in SS_Config.h. Include it so this
// header is self-contained regardless of include order in the TU.
#include "SS_Config.h"

class SS_RefCounter
{
    protected:
        int     refCount;
        std::string name;

    public:
                    SS_RefCounter() : refCount(0), name("unnamed") {}

        virtual     ~SS_RefCounter()
                    {
                        #if REF_DEBUG
                        if (refCount > 0)
                            printf("%s Prematurely Deleted! (%d)\n", name.c_str(), refCount);
                        else if (refCount < 0)
                            printf("[%p] %s RefCount Below Zero! (%d)\n", this, name.c_str(), refCount);
                        #endif
                    }

        inline int  RefCount()  { return refCount; }

        inline void Retain()
                    {
                        refCount++;
                        #if REF_DEBUG
                        if (!name.empty()) printf("[%p] Retain %s (%d)\n", this, name.c_str(), refCount);
                        #endif
                    }

        virtual int Release()
                    {
                        // SS_ASSERT_ON: a release when count is already <= 0 means
                        // a double-release / premature free — the usual source of a
                        // dangling pointer that later crashes mid-emit (use-after-free).
                        SS_ASSERT(refCount > 0);

                        int ref = --refCount;

                        #if REF_DEBUG
                        printf("[%p] Release %s (%d)\n", this, name.c_str(), ref);
                        #endif

                        if (ref == 0) delete this;
                        return ref;
                    }

        void        Retain(const char *n)
                    {
                        if (n) name = n;
                        Retain();
                    }

        const SS_RefCounter& operator=(const SS_RefCounter& src)
        {
            refCount = 0;
            name = src.name.empty() ? "(no name)" : src.name + "*";
            return *this;
        }
};

#endif
