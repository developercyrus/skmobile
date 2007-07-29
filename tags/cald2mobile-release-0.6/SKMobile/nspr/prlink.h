#pragma once

typedef struct PRStaticLinkTable {
	const char *name;
	void (*fp)();
} PRStaticLinkTable;

struct PRLibrary {
    char*                       name;  /* Our own copy of the name string */
    PRLibrary*                  next;
    int                         refCount;
    const PRStaticLinkTable*    staticTable;

#ifdef XP_PC
#ifdef XP_OS2
    HMODULE                     dlh;
#else
    HINSTANCE                   dlh;
#endif
#endif

#ifdef XP_MACOSX
    CFragConnectionID           connection;
    CFBundleRef                 bundle;
    Ptr                         main;
    CFMutableDictionaryRef      wrappers;
    const struct mach_header*   image;
#endif

#ifdef XP_UNIX
#if defined(USE_HPSHL)
    shl_t                       dlh;
#elif defined(USE_MACH_DYLD)
    NSModule                    dlh;
#else
    void*                       dlh;
#endif 
#endif 

#ifdef XP_BEOS
    void*                       dlh;
    void*                       stub_dlh;
#endif
};

