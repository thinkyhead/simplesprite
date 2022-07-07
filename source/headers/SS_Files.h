/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Files.h
 *
 *  $Id: SS_Files.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_FILES_H__
#define __SS_FILES_H__

#include "SS_Templates.h"
#include "SS_Types.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>

#ifndef WIN32
#include <libgen.h>
#endif

#include <sys/stat.h>


class SS_File
{
    protected:
        FILE        *stream;
        char        *path;
        size_t      bufsize;
        char        *buffer;
        struct stat itemstat;
        bool        exists;
        bool        is_open;
        bool        eof;

    public:

        SS_File();
        SS_File(const char *path);

        ~SS_File();

        void            SetPath(const char *path);
        char*           PrepareBuffer(int size);

        // File Open and Close
        bool            Open(const char *p, const char *mode);
        inline bool     Open(const char *p=NULL) { return OpenRead(p); }
        inline bool     OpenRead(const char *p=NULL) { return Open(p, "r"); }
        inline bool     OpenWrite(const char *p=NULL) { return Open(p, "w"); }
        inline bool     OpenRW(const char *p=NULL) { return Open(p, "r+"); }
        inline bool     OpenAppend(const char *p=NULL) { return Open(p, "a"); }
        inline bool     OpenRWAppend(const char *p=NULL) { return Open(p, "a+"); }
        inline void     Close() { if (stream != NULL) fclose(stream), stream = NULL; GetStatus(); }
        inline void     Touch() { OpenWrite(); Close(); }
        inline bool     Exists() { return exists; }

        // File Status
        void            GetStatus();
        inline struct stat* Status() { return exists ? &itemstat : NULL; }
        inline bool     GetEOF() { return ((stream != NULL) ? (feof(stream) != 0) : false); }
        inline void     ClearError() { if (stream != NULL) clearerr(stream); }
        inline char*    BaseName() { return SS_File::basename(path); }
        inline char*    DirName() { return SS_File::dirname(path); }
        inline char*    Path() { return path; }

        static char* basename(const char *path)
        {
            int i=0, s=0, e=strlen(path);
            char *temp = new char[e+1];

            while (i < e)
                if (path[i++] == '/')
                    s = i;

            i = 0;
            while (s <= e)
                temp[i++] = path[s++];

            return temp;
        }

        static char* dirname(const char *path)
        {
            int i=0, s=0, e=strlen(path);
            char *temp = new char[e+1];

            while (i < e)
                if (path[i++] == '/')
                    s = i-1;

            i = 0;
            while (i < s) {
                temp[i] = path[i];
                i++;
            }

            temp[i] = 0;

            return temp;
        }

        // File Reading
        size_t          Read(const char *buf, size_t len=0);
        size_t          ReadLine();
        size_t          ReadAll();
        char            GetChar() { Read(PrepareBuffer(1), 1); return buffer[0]; }
        char*           GetLine() { return ReadLine() ? buffer : NULL; }
        char*           GetAll() { ReadAll(); return buffer; }
        void            Dump();

        // File Writing
        size_t          Write(const char *buf, size_t len=0);
        inline size_t   Print(const char *text) { return Write(text, strlen(text)); }
        inline size_t   PrintLine(const char *text) { size_t q=0; q = Print(text); q += Print("\n"); return q; }

        // File Seeking
        int             Seek(long offset);

//      bool            Duplicate(char *copyPath);

        // Status Accessors
        inline FILE*    Stream() { return stream; }
        inline size_t   Size() { return (exists ? itemstat.st_size : 0); }

    private:
        void            Init();
};


#pragma mark -
//--------------------------------------------------------------
// SS_Folder
//--------------------------------------------------------------

class SS_Folder
{
    protected:
        char                *path;
        char                *pathStorage;
        int                 dir_count;
        struct dirent       **dir_entries;

        static char         *workingDir;                // Declare static members (init elsewhere)
        static SS_CharList  directoryStack;

    public:
        SS_Folder();
        SS_Folder(const char *path);
        ~SS_Folder();

        SS_FolderIterator*  GetIterator();

        static void         SetWorkingDir(const char *dir);
        static inline char* WorkingDir() { return workingDir; }

        static void         cd(const char *dir);
        static void         cdAppFolder(const char *dir=NULL);
        static void         cdDataFolder(const char *dir=NULL);

        static void         push(const char *dir);
        static void         pushAppFolder(const char *dir=NULL);
        static void         pushDataFolder(const char *dir=NULL);
        static char*        pop();

        static char*        FullPath(const char *file);

        void                SetPath(const char *path);
        int                 RefreshListing();
        void                FreeEntries();
        inline int          GetNumEntries() { return dir_count; }
        struct dirent*      GetEntry(int index) { return dir_entries[index]; }
        u_int32_t           GetFileNo(int index) { return dir_entries[index]->d_fileno; }
        u_int16_t           GetLength(int index) { return dir_entries[index]->d_reclen; }
        u_int8_t            GetType(int index) { return dir_entries[index]->d_type; }
        bool                IsFile(int index) { return (dir_entries[index]->d_type != DT_DIR); }
        bool                IsFolder(int index) { return (dir_entries[index]->d_type == DT_DIR); }
        char*               GetName(int index) { return dir_entries[index]->d_name; }
        u_int8_t            GetNameLen(int index) { return dir_entries[index]->d_namlen; }
        char*               GetFullPathOfEntry(int index);
        SS_File*            GetFileForEntry(int index) { return new SS_File(GetFullPathOfEntry(index)); }
        SS_Folder*          GetFolderForEntry(int index) { return new SS_Folder(GetFullPathOfEntry(index)); }

        // Select method for scandir
        static int          _select_all(struct dirent *);
        static int          _select_file(struct dirent *);

    private:
        void                Init();
};


#pragma mark -

//--------------------------------------------------------------
// SS_FolderIterator
//--------------------------------------------------------------

class SS_FolderIterator
{
    private:
        int             iter_pos;
        SS_Folder       *folder;

    public:
        SS_FolderIterator(SS_Folder *f) { Init(); folder = f; }
        ~SS_FolderIterator() { delete folder; }

        inline void         Reset() { iter_pos = -1; }
        int                 MoveTo(int pos) { iter_pos = pos >= -1 ? ( pos < folder->GetNumEntries() ? pos : folder->GetNumEntries()-1) : -1; return iter_pos; }
        bool                Next() { if (iter_pos < folder->GetNumEntries()-1) { ++iter_pos; return true; } return false; }
        bool                Prev() { if (iter_pos >= 0) { --iter_pos; return true; } return false; }
        bool                NextFolder();
        bool                NextFile();
        SS_FolderIterator*  EnterFolder();   // get an iterator for the current folder { return new SS_FolderIterator(CurrentFolder()); }
        bool                ProcessContents(int RECURSE_ORDER /* testProc, actionProc */);

        struct dirent*      CurrentEntry()      { return folder->GetEntry(iter_pos); }
        u_int32_t           CurrentFileNo()     { return folder->GetFileNo(iter_pos); }
        u_int16_t           CurrentLength()     { return folder->GetLength(iter_pos); }
        u_int8_t            CurrentType()       { return folder->GetType(iter_pos); }
        bool                IsCurrentFile()     { return folder->IsFile(iter_pos); }
        bool                IsCurrentFolder()   { return folder->IsFolder(iter_pos); }
        char*               CurrentName()       { return folder->GetName(iter_pos); }
        u_int8_t            CurrentNameLen()    { return folder->GetNameLen(iter_pos); }
        char*               CurrentFullPath()   { return folder->GetFullPathOfEntry(iter_pos); }
        SS_File*            CurrentFile()       { return folder->GetFileForEntry(iter_pos); }
        SS_Folder*          CurrentFolder()     { return folder->GetFolderForEntry(iter_pos); }

    private:
        void                Init() { Reset(); folder = NULL; }
};

#pragma mark -


//----------------------------------------------------------------------------------
//
// SS_DataToken
//
// A simple data unit with a key/value pair encoded as strings
//
//----------------------------------------------------------------------------------

enum {
    kSSError    =   1000,
    kSSErrorCantWrite,
    kSSErrorCantRead,
    kSSErrorCantFind
};

typedef TLinkedList<SS_DataToken*>  SS_TokenList;
typedef TIterator<SS_DataToken*>        SS_TokenIterator;
typedef TLinkedList<SS_DataContext*>  SS_ContextList;
typedef TIterator<SS_DataContext*>  SS_ContextIterator;

//
// SS_DataToken
// A simple type-specifier for a given token string
//
class SS_DataToken
{
    public:
        char            *key;
        char            *value;
        bool            isRaw;
        int             rawLen;

        SS_DataToken(const char *k, const char *v);
        SS_DataToken(const char *k, int v);
        SS_DataToken(const char *k, Uint32 v);
        SS_DataToken(const char *k, float v);
        SS_DataToken(const char *k, SScolorb &color);
        SS_DataToken(const char *k, void *data, int len);
        ~SS_DataToken();

        void    Set(const char *k, const char *v);
        bool    Match(const char *k) { return strcmp(key, k) == 0; }

        char*   Value() { return value; }
        void*   Raw() { return (void*)value; }
        void    Copy(void *dest) { if (rawLen) bcopy(value, dest, rawLen); }

        void    Write(FILE *file);      // write this token out to a file
};

#pragma mark -
//----------------------------------------------------------------------------------
//
// SS_DataContext
//
// Maintains a simple list of DataTokens under a single name
//
//----------------------------------------------------------------------------------

class SS_DataContext
{
    public:
        char            *key;
        SS_TokenList    tokenList;  // list of nodes, each one having a token pointer

        SS_DataContext();
        SS_DataContext(const char *k);
        ~SS_DataContext();

        SS_DataToken*   GetToken(const char *k);
        char*           GetTokenValue(const char *k);
        void*           GetTokenRaw(const char *k);
        void            CopyTokenData(const char *k, void *dest);

        void            AddToken(SS_DataToken *token) { tokenList.Append(token); }

        void            Write(FILE *file);   // write the context and tell all tokens to write

        bool            Match(const char *k) { return strcmp(key, k) == 0; }
        int             Size() { return tokenList.Size(); }
};

#pragma mark -
//----------------------------------------------------------------------------------
//
// SS_FlatFile
//
// - Represents the contents of a flat data file, such as a config file
// - Provides methods to read and write such files
//
// TODO: Store the contexts and data in a fast-access binary tree
//
//----------------------------------------------------------------------------------

class SS_FlatFile
{
    private:
        SS_ContextList  contextList;        // All the root-level contexts

        SS_File         *currFile;
        SS_DataContext  *currContext;
        SS_DataToken    *currToken;
        char            *currData;

    public:
        SS_FlatFile();
        SS_FlatFile(const char *filename);
        SS_FlatFile(char *def[][2]);
        ~SS_FlatFile();

        inline bool         Exists() { return currFile ? currFile->Exists() : false; }

        // Searches the list
        SS_DataContext*     GetContext(const char *k);

        // EnterContext selects an existing context or creates a new one
        void                EnterContext(const char *k);

        // Exit the current context - not implemented for flat files
        void                ExitContext(const char *k=NULL) {}

        // SetToken adds a new token to the last-added context
        inline void         SetToken(const char *k, bool v)                 { SetToken(k, v ? 1 : 0); }
        inline void         SetToken(const char *k, const char *v)          { currContext->AddToken(new SS_DataToken(k, v)); }
        inline void         SetToken(const char *k, int v)                  { currContext->AddToken(new SS_DataToken(k, v)); }
        inline void         SetToken(const char *k, Uint32 v)               { currContext->AddToken(new SS_DataToken(k, v)); }
        inline void         SetToken(const char *k, float v)                { currContext->AddToken(new SS_DataToken(k, v)); }
        inline void         SetToken(const char *k, SScolorb &color)        { currContext->AddToken(new SS_DataToken(k, color)); }
        inline void         SetToken(const char *k, void *data, int len)    { currContext->AddToken(new SS_DataToken(k, data, len)); }

        // GetTokenValue gets a token's raw string value
        inline char*        GetTokenValue(const char *k) { return currContext->GetTokenValue(k); }
        inline void*        GetTokenRaw(const char *k) { return currContext->GetTokenRaw(k); }
        inline void         CopyTokenData(const char *k, void *dest) { currContext->CopyTokenData(k, dest); }

        // Import reads a file's contents and populates
        // a context-token tree structure
        bool                Import(const char *fileName);

        // Export writes the context-token data to a file
        bool                Export(const char *fileName);
        bool                Export();

        void                ParseLine(const char *line);

        inline bool         GetBoolean(const char *k) { int i; sscanf(GetTokenValue(k), "%d", &i); return (i != 0); }
        inline int          GetInteger(const char *k) { int i; sscanf(GetTokenValue(k), "%d", &i); return i; }
        inline char*        GetString(const char *k) { return GetTokenValue(k); }
        inline float        GetFloat(const char *k) { float f; sscanf(GetTokenValue(k), "%f", &f); return f; }
        inline void*        GetRaw(const char *k) { return GetTokenRaw(k); }

    private:
        void                Init();
};

#pragma mark -
//----------------------------------------------------------------------------------
//
// SS_FlatFile
//
// - Represents the contents of a structured data file, such as a level file
// - May use the expat library to read and write XML files
// - Or, might implement its own format similar to XML or plist files
//
// TODO: Determine how to build and use expat on Windows
//
//----------------------------------------------------------------------------------

class SS_StructuredFile : public SS_FlatFile
{
};

#endif
