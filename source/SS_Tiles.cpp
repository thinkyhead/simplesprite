/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Tiles.cpp
 *
 *  $Id: SS_Tiles.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_Tiles.h"

#include "SS_Files.h"
#include "SS_Game.h"
#include "SS_LayerItem.h"
#include "SS_Utilities.h"
#include "SS_World.h"

#include <stdlib.h>

//--------------------------------------------------------------
// SS_World ... TileLayer methods
//--------------------------------------------------------------

//
// NewTileLayer(map, flags)
// Create a tile layer, set its map and flags, add it to the world, and return the layer pointer
//
SS_TileLayer* SS_World::NewTileLayer(SS_TileMap *map, Uint32 f)
{
    DEBUGF(1, "[%08X] SS_World::NewTileLayer(%08X, %04X)\n", this, map, f);

    SS_TileLayer *layer = new SS_TileLayer(map, f);

    AddLayer(layer);

    return layer;
}

//
// NewTileLayer(map)
//
SS_TileLayer* SS_World::NewTileLayer(SS_TileMap *map)
{
    DEBUGF(1, "[%08X] SS_World::NewTileLayer(%08X)\n", this, map);

    SS_TileLayer *layer = new SS_TileLayer(map);

    AddLayer(layer);

    return layer;
}

//
// NewTileLayer
//
SS_TileLayer* SS_World::NewTileLayer()
{
    DEBUGF(1, "SS_World::NewTileLayer()\n");

    SS_TileLayer *layer = new SS_TileLayer();

    AddLayer(layer);

    return layer;
}



#pragma mark -
//--------------------------------------------------------------
// SS_TilePalette
// An image containing regularly-sized tiles for use in tile-maps
//--------------------------------------------------------------

SS_TilePalette::SS_TilePalette(char *filename, int tw, int th)
{
    DEBUGF(1, "SS_TilePalette::SS_TilePalette(\"%s\", %d, %d)\n", filename, tw, th);

    ::GetTextureFromImage(filename, NULL, NULL, &gl_texture, &image_w, &image_h);

    tile_w      = tw;
    tile_h      = th;
    tile_pw     = (float)tw / image_w;
    tile_ph     = (float)th / image_h;
    tiles_across    = image_w / tw;
    tiles_down  = image_h / th;
}

SS_TilePalette::~SS_TilePalette()
{
    tile_w = 0;
    tile_h = 0;
    glDeleteTextures(1, &gl_texture);
}



#pragma mark -
//--------------------------------------------------------------
//  SS_TileMap
//  A tile map, which consists of an array of tile numbers
//--------------------------------------------------------------
SS_TileMap::SS_TileMap(int w, int h)
{
    DEBUGF(1, "[%08X] SS_TileMap::SS_TileMap(%d, %d)\n", this, w, h);

    Init();
    InitMap(w, h);
}

SS_TileMap::SS_TileMap(char *filename)
{
    Init();
    if (filename)
        LoadMap(filename);
}

SS_TileMap::~SS_TileMap()
{
    DisposePalette();
    DisposeMap();
}

//
// Init
//
void SS_TileMap::Init()
{
    DEBUGF(1, "[%08X] SS_TileMap::Init()\n", this);

    tilePalette = NULL;
    theMap      = NULL;
    xpos        = 0;
    ypos        = 0;
    columns     = 0;
    rows        = 0;
    tile_w      = 0;
    tile_h      = 0;
    pix_w       = 0;
    pix_h       = 0;
}

//
// LoadPalette(filename, tw, th)
//
void SS_TileMap::LoadPalette(char *filename, int tw, int th)
{
    DEBUGF(1, "[%08X] SS_TileMap::LoadPalette()\n", this);

    SS_TilePalette *palette;

    DisposePalette();

    palette = new SS_TilePalette(filename, tw, th);

    if (!palette)
        throw "Can't get Tile Palette image.";

    SetPalette(palette);
}

//
// SetPalette(palette)
//
void SS_TileMap::SetPalette(SS_TilePalette *palette)
{
    DEBUGF(1, "[%08X] SS_TileMap::SetPalette(%08X)\n", this, palette);

    if (tilePalette)
        tilePalette->Release();

    if ((tilePalette = palette))
    {
        palette->Retain("Tile Palette");
        tile_w  = palette->tile_w;
        tile_h  = palette->tile_h;
        pix_w   = columns * tile_w;     // map's pixel size based on tile size
        pix_h   = rows * tile_h;
    }
}

//
// DisposePalette
//
void SS_TileMap::DisposePalette()
{
    DEBUGF(1, "SS_TileMap::DisposePalette()\n");

    if (tilePalette) {
        (void) tilePalette->Release();
        tilePalette = NULL;
        tile_w = 0;
        tile_h = 0;
    }
}

//
// InitMap(w, h)
//
void SS_TileMap::InitMap(int w, int h)
{
    DEBUGF(1, "[%08X] SS_TileMap::InitMap(%d, %d)\n", this, w, h);

    if (theMap != NULL)
        DisposeMap();

    if (w || h)
    {
        if(!(theMap = (unsigned char*)calloc(w, h)))
            throw "Can't allocate memory for map.";
    }

    columns = w;
    rows = h;

    pix_w = tile_w * w;
    pix_h = tile_h * h;
}

//
// LoadMap(filename)
// Load a tile map file
//
void SS_TileMap::LoadMap(char *filename)
{
    DEBUGF(1, "SS_TileMap::LoadMap(%s)\n", filename);

    SS_FlatFile   mapFile;
    if (mapFile.Import(filename))
    {
        mapFile.EnterContext("TileMap");
        InitMap(mapFile.GetInteger("Width"), mapFile.GetInteger("Height"));
        mapFile.CopyTokenData("Buffer", theMap);
    }
}

//
// SaveMap(filename)
// Save a tile map file
//
bool SS_TileMap::SaveMap(char *filename)
{
    DEBUGF(1, "SS_TileMap::SaveMap(%s)\n", filename);

    SS_FlatFile   mapFile;
    mapFile.EnterContext("TileMap");
    mapFile.SetToken("Width", columns);
    mapFile.SetToken("Height", rows);
    mapFile.SetToken("Buffer", theMap, columns * rows);
    return mapFile.Export(filename);
}

//
// DisposeMap
//
void SS_TileMap::DisposeMap()
{
    DEBUGF(1, "SS_TileMap::DisposeMap()\n");

    if (theMap != NULL) {
        free(theMap);
        theMap = NULL;
    }

    columns = 0;
    rows = 0;
    pix_w = 0;
    pix_h = 0;
}

//
// Render(x, y, w, h)
// Render a piece of the tile map into the current context
//
void SS_TileMap::Render(float x, float y, float w, float h)
{
    Uint16  xx, yy;
    float   rx, ry;
    Sint16  rclip, lclip, tclip, bclip, ll, rr, tt, bb;
    char    tile;

    // determine if any of the map is showing
    if ((xpos + pix_w < x) || (xpos > x + w) || (ypos + pix_h < y) || (ypos > y + h))
        return;

    // initial left, top, right, bottom
    ll = 0; tt = 0;
    rr = columns - 1;
    bb = rows - 1;

    // clip the left, top, right, bottom
    lclip = (int)(x - xpos + tile_w - 0.01) / tile_w;
    if (lclip >= 2) ll = lclip - 1;

    tclip = (int)(y - ypos + tile_w - 0.01) / tile_h;
    if (tclip >= 2) tt = tclip - 1;

    rclip = (int)((xpos + pix_w) - (x + w) + tile_w - 0.01) / tile_w;
    if (rclip >= 2) rr -= rclip - 1;

    bclip = (int)((ypos + pix_h) - (y + h) + tile_h - 0.01) / tile_h;
    if (bclip >= 2) bb -= bclip - 1;

    gl_do_texture(1);
    gl_do_blend(0);
    gl_bind_texture(tilePalette->gl_texture);

    // draw the visible tiles
    ry = ypos + tt * tile_h;
    for (yy=tt; yy<=bb; ++yy)
    {
        rx = xpos + ll * tile_w;
        for (xx=ll; xx<=rr; ++xx) {
            if ((tile = theMap[yy * columns + xx]))
                DrawTile(tile, rx, ry);
            rx += tile_w;
        }
        ry += tile_h;
    }
}

//
// Render(world)
// Render tiles in the visible part of an SS_World
//
void SS_TileMap::Render(SS_World *world, Uint32 flags)
{
    float   x, y;

    if (flags & SS_NOSCROLL) {
        x = 0; y = 0;
    } else {
        x = world->Left();
        y = world->Top();
    }

    if (flags & SS_NOZOOM)
        Render(x, y, world->ViewWidth(), world->ViewHeight());
    else
        Render(x, y, world->ZoomWidth(), world->ZoomHeight());
}

//
// DrawTile
// Draw a tile at a given screen position
//
void SS_TileMap::DrawTile(Uint8 tile, GLfloat x1, GLfloat y1)
{
    // screen coordinates, not world coordinates
    SS_TilePalette  *t = tilePalette;
    GLfloat x2 = x1 + t->tile_w;
    GLfloat y2 = y1 + t->tile_h;

    if (tile)
    {
        tile--;

        // x,y of source tile image as percentages
        GLfloat tx1 = (float)(tile % t->tiles_across) / t->tiles_across;
        GLfloat ty1 = (float)(tile / t->tiles_across) / t->tiles_down;

        // add on the size of a tile as a percentage
        GLfloat tx2 = tx1 + t->tile_pw;
        GLfloat ty2 = ty1 + t->tile_ph;

        // Make a textured rectangle
        glColor4ubv((GLubyte*)&SS_WHITE_B);
        glBegin(GL_QUADS);
        glTexCoord2f(tx1, ty1); glVertex2f(x1, y1);
        glTexCoord2f(tx2, ty1); glVertex2f(x2, y1);
        glTexCoord2f(tx2, ty2); glVertex2f(x2, y2);
        glTexCoord2f(tx1, ty2); glVertex2f(x1, y2);
        glEnd();
    }
    else
    {
        // make a black rectangle
        glColor3ubv((GLubyte*)&SS_BLACK_B);
        glRectf(x1, y1, x2, y2);
    }
}



#pragma mark -
//--------------------------------------------------------------
// SS_TileLayer
// A layer containing a tilemap
//--------------------------------------------------------------

SS_TileLayer::SS_TileLayer()
{
    DEBUGF(1, "[%08X] SS_TileLayer::SS_TileLayer()\n", this);

    Init();
}

SS_TileLayer::SS_TileLayer(SS_TileMap *map, Uint32 f)
{
    DEBUGF(1, "[%08X] SS_TileLayer::SS_TileLayer(%08X, %04X)\n", this, map, f);

    Init(f);
    SetTileMap(map);
}

SS_TileLayer::SS_TileLayer(SS_TileMap *map)
{
    DEBUGF(1, "[%08X] SS_TileLayer::SS_TileLayer(%08X)\n", this, map);

    Init();
    SetTileMap(map);
}

SS_TileLayer::SS_TileLayer(Uint32 f)
{
    DEBUGF(1, "[%08X] SS_TileLayer::SS_TileLayer(%04X)\n", this, f);

    Init(f);
}

SS_TileLayer::~SS_TileLayer()
{
}

//
// Init([flags])
//
void SS_TileLayer::Init(Uint32 f)
{
    tileMap = NULL;
    flags = f;
}

//
// SetTileMap
//
void SS_TileLayer::SetTileMap(SS_TileMap *map)
{
    DEBUGF(1, "[%08X] SS_TileLayer::SetTileMap(%08X)\n", this, map);

    if (tileMap != NULL)
        delete tileMap;

    tileMap = map;
}

//
// Process
//
void SS_TileLayer::Process()
{
}

//
// Animate
//
void SS_TileLayer::Animate()
{
}

//
// Render
//
void SS_TileLayer::Render()
{
    PrepareMatrix();
    tileMap->Render(world, flags);
}
