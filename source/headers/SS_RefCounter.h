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

#include <stdio.h>
#include <string.h>

// SS_ASSERT / SS_ASSERT_FINITE are defined in SS_Config.h. Include it so this
// header is self-contained regardless of include order in the TU.
#include "SS_Config.h"

class SS_RefCounter
{
    private:
        int     refCount;
        char    *name;

    public:
                    SS_RefCounter() { refCount = 0; name = new char[20]; strcpy(name, "unnamed"); }

        virtual     ~SS_RefCounter()
                    {
                        #if REF_DEBUG
                        if (refCount > 0)
                        {
                            printf("%s Prematurely Deleted! (%d)\n", name, refCount);
                        }
                        else if (refCount < 0)
                        {
                            printf("[%p] %s RefCount Below Zero! (%d)\n", this, name, refCount);
                        }
                        #endif

                        if (name)
                            delete name;
                    }

        inline int  RefCount()  { return refCount; }

        inline void Retain()
                    {
                        refCount++;
                        #if REF_DEBUG
                        if (name) printf("[%p] Retain %s (%d)\n", this, name, refCount);
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
                        printf("[%p] Release %s (%d)\n", this, name, ref);
                        #endif

                        if (ref == 0) delete this;
                        return ref;
                    }

        void        Retain(const char *n)
                    {
                        if (name != NULL) {
                            delete name;
                            name = NULL;
                        }

                        if (n) {
                            name = new char[strlen(n)+1];
                            strcpy(name, n);
                            }
                        Retain();
                    }

        const SS_RefCounter& operator=(const SS_RefCounter& src)
        {
            refCount = 0;

            if (name) delete name;

            if (src.name)
            {
                name = new char[strlen(src.name) + 10];
                strcpy(name, src.name);
                strcat(name, "*");
            }
            else
            {
                name = new char[10];
                strcpy(name, "(no name)");
            }
            return *this;
        }
};

#endif
