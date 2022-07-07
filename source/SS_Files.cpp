/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Files.cpp
 *
 *  $Id: SS_Files.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_Files.h"
#include "SS_Utilities.h"

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>


//--------------------------------------------------------------
// SS_Folder
// A folder with a listing of its contents
//--------------------------------------------------------------

// Static Initializers
char*       SS_Folder::workingDir = NULL;
SS_CharList SS_Folder::directoryStack;

SS_Folder::SS_Folder()
{
    Init();
}

SS_Folder::SS_Folder(const char *path)
{
    Init();
    SetPath(path);
}

SS_Folder::~SS_Folder()
{
    FreeEntries();

    if (path)
        delete [] path;

    if (pathStorage)
        delete [] pathStorage;
}

//
// Init
//
void SS_Folder::Init()
{
    path        = NULL;
    pathStorage = NULL;
    dir_count   = 0;
    dir_entries = NULL;
}

//
// GetIterator
// Get an iterator
//
SS_FolderIterator* SS_Folder::GetIterator()
{
    return new SS_FolderIterator(this);
}

//
// SetPath
// Set the path of the folder and get its contents
//
void SS_Folder::SetPath(const char *p)
{
    if (path) delete[] path;
    path = newstring(p);
    RefreshListing();
}

//
// RefreshListing
// Get the directory entries into a list
// Return the length of the list or -1 if the path is unset
//
int SS_Folder::RefreshListing()
{
    int r = -1;

    FreeEntries();

    if (path != NULL)
        r = scandir((const char *)path, (struct dirent ***)&dir_entries, (int (*)(const struct dirent *))_select_all, (int (*)(const struct dirent **, const struct dirent **))alphasort);

    dir_count = r;
    return r;
}

//
// FreeEntries
// Free the entry list
//
void SS_Folder::FreeEntries()
{
    if (dir_count > 0)
    {
        for (int i=dir_count; i--;)
            free(dir_entries[i]);

        free(dir_entries);

        dir_count = 0;
        dir_entries = NULL;
    }
}

//
// GetFullPathOfEntry
//
char* SS_Folder::GetFullPathOfEntry(int index)
{
    if (pathStorage != NULL)
        delete [] pathStorage;

    struct dirent *de = dir_entries[index];

    pathStorage = new char[strlen(path)+de->d_namlen+1];
    strcpy(pathStorage, path);
    strcat(pathStorage, "/");
    strncat(pathStorage, de->d_name, de->d_namlen);

    return pathStorage;
}

//
// cdAppFolder()
// The application folder is the root on Windows and Linux
// but on Mac OS X it lies three levels up from Resources
//
void SS_Folder::cdAppFolder(const char *dir)
{
#if defined(__APPLE__)
    SetWorkingDir("../../..");
#else
    SetWorkingDir("");
#endif

    if (dir != NULL) cd(dir);
}

//
// cdDataFolder()
// Game data lives in a folder named "data" which is:
// - On Mac OS X, in the application package Contents folder
// - On Windows and Linux alongside the Application folder
//
void SS_Folder::cdDataFolder(const char *dir)
{
    SetWorkingDir("data");
    if (dir != NULL) cd(dir);
}

//
// SetWorkingDir(dir)
//
void SS_Folder::SetWorkingDir(const char *dir)
{
    if (workingDir) delete[] workingDir;
    workingDir = newstring(dir);
}

//
// cd
//
void SS_Folder::cd(const char *dir)
{
    if (strcmp("..", dir) == 0)
    {
        char *c = strrchr(workingDir, '/');     // get the address of last slash

        if (c == NULL)                          // no slashes left?
            SetWorkingDir("");
        else if (c == workingDir)               // starts with a slash?
            SetWorkingDir("/");                 // then keep the slash
        else {
            int l = c - workingDir + 1;
            char *par = new char[l + 1];
            strncpy(par, workingDir, l);
            par[l] = '\0';
            SetWorkingDir(par);
            delete [] par;
        }
    }
    else if (strncmp("/", workingDir, 1) == 0)
        SetWorkingDir(dir);
    else
        SetWorkingDir(FullPath(dir));
}

//
// push
//
void SS_Folder::push(const char *dir)
{
    directoryStack.Append(newstring(workingDir));
    cd(dir);
}

//
// pop
//
char* SS_Folder::pop()
{
    char *dir = NULL;

    SS_CharIterator itr = directoryStack.GetIterator();
    itr.End();
    if ((dir = itr.Item())) {
        SetWorkingDir(dir);
        free(dir);
        directoryStack.RemoveTail();
    }

    return workingDir;
}

//
// FullPath(filename)
//
char* SS_Folder::FullPath(const char *file)
{
    static char pathString[1024];

    if (workingDir != NULL)
    {
        int l = strlen(workingDir);
        strcpy(pathString, workingDir);
        if (l > 0 && pathString[l - 1] != '/')
            strcat(pathString, "/");
    }
    else
        pathString[0] = 0;

    strcat(pathString, file);

    return pathString;
}

//
// _select_all(dirent*)
// Return 1 if it's not a dot-file
//
int SS_Folder::_select_all(struct dirent *entry)
{
    return entry->d_name[0] != '.' ? 1 : 0;
}

//
// _select_file(dirent*)
// Return 1 if it's not a dir
//
int SS_Folder::_select_file(struct dirent *entry)
{
    return (entry->d_name[0] != '.' && entry->d_type != DT_DIR) ? 1 : 0;
}


#pragma mark -
//--------------------------------------------------------------
// SS_FolderIterator
// Given a folder this provides
// position tracking.
//--------------------------------------------------------------

bool SS_FolderIterator::NextFolder()
{
    while(Next()) {
        if (IsCurrentFolder())
            return true;
    }

    return false;
}

bool SS_FolderIterator::NextFile()
{
    while(Next()) {
        if (IsCurrentFile())
            return true;
    }

    return false;
}

#pragma mark -
//--------------------------------------------------------------
// SS_File
// A file specifier which represents a file that may or may not exist
//--------------------------------------------------------------

SS_File::SS_File()
{
    Init();
}

SS_File::SS_File(const char *path)
{
    Init();
    SetPath(path);
}

SS_File::~SS_File()
{
    if (is_open)
        Close();
}

//
// Init
//
void SS_File::Init()
{
    path    = NULL;
    stream  = NULL;
    buffer  = NULL;
    bufsize = 0;
    exists  = false;
    is_open = false;
}

//
// SetPath
//
void SS_File::SetPath(const char *p)
{
    if (path) delete [] path;
    path = newstring(SS_Folder::FullPath(p));

    GetStatus();
}

//
// Seek
//
int SS_File::Seek(long offset)
{
    return fseek(stream, offset, SEEK_SET);
}

//
// GetStatus
//
void SS_File::GetStatus()
{
    int s = stat(path, &itemstat);

    exists = (s == 0);
}

//
// PrepareBuffer(size)
//
char* SS_File::PrepareBuffer(int size)
{
    if (buffer != NULL)
        delete [] buffer;

    buffer = new char[size+1];  // allocate +1 byte for terminating null
    bufsize = size;

    buffer[size] = 0;               // null to terminate string input

    return buffer;
}

//
// Open
//
bool SS_File::Open(const char *p, const char *mode)
{
    if (is_open)
        Close();

    if (p != NULL)
        SetPath(p);

    if (stream = fopen(path, mode))
        is_open = true;

    return is_open;
}

//
// Read(buffer, length)
//
size_t SS_File::Read(const char *buf, size_t len)
{
    size_t  s = 0;

    if (is_open) {
        s = fread((char*)buf, len, 1, stream);
        GetEOF();
    }

    return s;
}

//
// ReadAll
//
size_t SS_File::ReadAll()
{
    const char *buf = PrepareBuffer(Size());
    return Read(buf, Size());
}

//
// ReadLine
//
#define MAX_BUFSIZE 1000000
size_t SS_File::ReadLine()
{
    size_t  s = 0;
    char    *tbuf;
    char    *buf;

    if (is_open)
    {

#ifdef WIN32

        tbuf = new char[MAX_BUFSIZE];
        fgets(tbuf, MAX_BUFSIZE, stream);

#else

        tbuf = fgetln(stream, &s);

#endif

        buf = PrepareBuffer(s);

        if (tbuf != NULL && s > 0)
            bcopy(tbuf, buf, s);

#ifdef WIN32
        delete [] tbuf;
#endif

        GetEOF();
    }

    return s;
}

//
// Write(buffer, length)
//
size_t SS_File::Write(const char *buf, size_t len)
{
    if (is_open)
        return fwrite(buf, len, 1, stream);
    else
        return 0;
}

//
// Dump()
//
void SS_File::Dump()
{
    Open();

    int l = 1;
    while(!GetEOF())
        printf("%04d: %s\r", l++, GetLine());

    Close();
}


#pragma mark -
//--------------------------------------------------------------
// SS_DataToken
// Description of a single basic unit of data
//--------------------------------------------------------------

SS_DataToken::SS_DataToken(const char *k, const char *v)
{
    Set(k, v);
}

SS_DataToken::SS_DataToken(const char *k, int v)
{
    char    *temp = new char[50];
    sprintf(temp, "%d", v);
    Set(k, temp);
    delete [] temp;
}

SS_DataToken::SS_DataToken(const char *k, Uint32 v)
{
    char    *temp = new char[50];
    sprintf(temp, "%u", v);
    Set(k, temp);
    delete [] temp;
}

SS_DataToken::SS_DataToken(const char *k, float v)
{
    char    *temp = new char[100];
    sprintf(temp, "%f", v);
    Set(k, temp);
    delete [] temp;
}

SS_DataToken::SS_DataToken(const char *k, SScolorb &color)
{
    key = newstring(k);

    value = new char[sizeof(SScolorb)];
    bcopy(&color, value, sizeof(SScolorb));

    isRaw = true;
    rawLen = sizeof(SScolorb);
}

SS_DataToken::SS_DataToken(const char *k, void *data, int len)
{
    key = newstring(k);
    value = new char[len];
    bcopy(data, value, len);

    isRaw = true;
    rawLen = len;
}

SS_DataToken::~SS_DataToken()
{
    delete [] key;
    delete [] value;
}

void SS_DataToken::Set(const char *k, const char *v)
{
    key = newstring(k);

    if (v[0] == '{')
    {
        isRaw = true;
        rawLen = strlen(v) / 2 - 1;
        value = new char[rawLen];
        for (int i=0; i<rawLen; i++) {
            char h = v[i*2+1], l = v[i*2+2];
            value[i] = (h-(h>='A'?'A'-10:'0'))*16+(l-(l>='A'?'A'-10:'0'));
        }
    }
    else
    {
        isRaw = false;
        rawLen = 0;
        value = newstring(v);
    }
}

void SS_DataToken::Write(FILE *file)
{
    fprintf(file, "%s=", key);
    if (isRaw)
    {
        fprintf(file, "{");

        for (int i=0; i<rawLen; i++)
            fprintf(file, "%02X", value[i]);

        fprintf(file, "}\n");
    }
    else
        fprintf(file, "%s\n", value);
}

#pragma mark -
//--------------------------------------------------------------
// SS_DataContext
// A list of data tokens
//--------------------------------------------------------------

SS_DataContext::SS_DataContext(const char *k)
{
    key = newstring(k);
}

SS_DataContext::~SS_DataContext()
{
    delete [] key;
}

SS_DataToken* SS_DataContext::GetToken(const char *k)
{
    SS_TokenIterator    itr = tokenList.GetIterator();
    SS_DataToken        *tok;

    while (tok = itr.NextItem())
        if (tok->Match(k))
            return tok;

    return NULL;
}

char* SS_DataContext::GetTokenValue(const char *k)
{
    SS_DataToken *tok = GetToken(k);

    if (tok != NULL)
        return tok->Value();
    else
        return "";
}

void* SS_DataContext::GetTokenRaw(const char *k)
{
    SS_DataToken *tok = GetToken(k);

    if (tok != NULL)
        return tok->Raw();
    else
        return NULL;
}

void SS_DataContext::CopyTokenData(const char *k, void *dest)
{
    SS_DataToken *tok = GetToken(k);
    if (tok != NULL) tok->Copy(dest);
}

void SS_DataContext::Write(FILE *file)
{
    fprintf(file, "[%s]\n", key);

    SS_DataToken        *tok;
    SS_TokenIterator    itr = tokenList.GetIterator();

    itr.Start();
    while (tok = itr.NextItem())
        tok->Write(file);
}

#pragma mark -
//--------------------------------------------------------------
// SS_FlatFile
// A file that contains structured data
//--------------------------------------------------------------

SS_FlatFile::SS_FlatFile()
{
    Init();
}

SS_FlatFile::SS_FlatFile(const char *filename)
{
    Init();
    (void)Import(filename);
}

SS_FlatFile::SS_FlatFile(char *def[][2])
{
    Init();
}

SS_FlatFile::~SS_FlatFile()
{
}

//
// Init
// Create an empty-string context for early lines
//
void SS_FlatFile::Init()
{
    EnterContext("");
}

//
// GetContext(name)
//
// Get the context by name
//
SS_DataContext* SS_FlatFile::GetContext(const char *k)
{
    SS_ContextIterator  itr = contextList.GetIterator();
    SS_DataContext      *ctx;

    itr.Start();
    while (ctx = itr.NextItem())
        if (ctx->Match(k))
            return ctx;

    return NULL;
}

//
// EnterContext(name)
//
// This sets the current context to an existing context
// or if there is no such context it creates it for you
// and then sets the context there.
//
// The idea is that you can use this as a general way to
// store relatively flat data. Specifically a shallow tree.
//
// A structured XML datafile class will use expat and
// a callback-parser stack scheme.
//
void SS_FlatFile::EnterContext(const char *k)
{
    if (!(currContext = GetContext(k)))
    {
        currContext = new SS_DataContext(k);
        contextList.Append(currContext);
    }
}

//
// Import a data file with the given name
// Returns true if successful
//
// TODO: More serious error-checking and correction
//
bool SS_FlatFile::Import(const char *fileName)
{
    char    *line;

    currFile = new SS_File(fileName);       // Stand-in for a file which doesn't have to exist

    if (currFile->OpenRead())
    {
        while(line = currFile->GetLine()) // get the buffer managed by the file class
            ParseLine(line);

        currFile->Close();
        return true;
    }
    else
        return false;
}

//
// Export
//
bool SS_FlatFile::Export()
{
    SS_DataContext      *ctx;
    SS_ContextIterator  itr = contextList.GetIterator();

    if (currFile->OpenWrite())
    {
        FILE    *f = currFile->Stream();
        itr.Start();
        while (ctx = itr.NextItem())
            if (ctx->Size())
                ctx->Write(f);

        currFile->Close();
        return true;
    }
    else
        return false;
}

//
// Export(filepath)
//
// Writes the DataFile tree out as a file.
//
bool SS_FlatFile::Export(const char *fileName)
{
    currFile = new SS_File(fileName);
    return Export();
}

//
// ParseLine
// Take a line and do the appropriate action
//
void SS_FlatFile::ParseLine(const char *input)
{
    int len = strlen(input);

    if (len)
    {
        char *part, *open, *clos;
        char *line = newstring(input);
        char *tline = trim(line);

            // Lines with braces are context lines
            //
        if ( (open = strchr(line, '[')) && (clos = strchr(tline, ']')) ) {
            *clos = '\0';
            EnterContext(trim(open+1));
        }
            // Lines with an = sign are token assignments
            //
        else if (part = strchr(tline, '=')) {
            *part = '\0';
            SetToken(tline, trim(part+1));
        }

        // Any other line is ignored (i.e., behaves like a comment

        delete [] line;
    }
}
