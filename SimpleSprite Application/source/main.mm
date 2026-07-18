// Thin macOS main for the SimpleSpriteTest SDL3 sample app.
// chdir to Resources/ so data/... resolves, then enter the SDL_main entry.
#define SDL_MAIN_HANDLED 1
#include <CoreFoundation/CoreFoundation.h>
#include "My_Game.h"

My_Game *theGame = NULL;

int main(int argc, char *argv[]) {
    CFURLRef url = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
    if (url) {
        char path[1024];
        if (CFURLGetFileSystemRepresentation(url, true, (UInt8*)path, sizeof(path)))
            chdir(path);
        CFRelease(url);
    }
    try { theGame = new My_Game(); theGame->Run(); }
    catch (const char* err) { fprintf(stderr, err, SDL_GetError()); }
    if (theGame) delete theGame;
    fprintf(stderr, "\nExiting Cleanly...\n");
    return 0;
}
