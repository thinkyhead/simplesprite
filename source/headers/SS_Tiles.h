/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Tiles.h
 *
 *  $Id: SS_Tiles.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_TILES_H__
#define __SS_TILES_H__

#include <SDL.h>
#include <SDL_opengl.h>
#include "SS_Types.h"
#include "SS_Layer.h"
#include "SS_RefCounter.h"


//--------------------------------------------------------------
// SS_TilePalette
// A tile palette, the source of textures for a tile map
//
class SS_TilePalette : public SS_RefCounter
{
    private:
        Uint16      image_w, image_h, image_depth;  // image characteristics

    public:
        GLuint      gl_texture;                     // the texture name in gl
        Uint16      tile_w, tile_h;                 // tile size in pixels
        float       tile_pw, tile_ph;               // tile size as percentage of palette size
        Uint16      tiles_across, tiles_down;       // how many tiles across and down

                    SS_TilePalette(char *filename, int tw, int th);
                    ~SS_TilePalette();
};



#pragma mark -
//--------------------------------------------------------------
// SS_TileMap
// A tile map, which consists of an array of indices into a tile array
//
class SS_TileMap
{
    private:
        SS_TilePalette  *tilePalette;               // The tile palette to use for drawing
        Uint16          tile_w, tile_h;             // The size of the current tiles
        Uint16          columns;                    // How many tiles across and down
        Uint16          rows;

    public:
        Uint8           *theMap;                    // The map itself
        float           xpos, ypos;                 // The map's position in the universe
        float           pix_w, pix_h;               // pixel width and height
        SScolorb        radarColor;                 // The map's color on radar

                    SS_TileMap(int w, int h);
                    SS_TileMap(char* filename);
                    ~SS_TileMap();

        void        InitMap(int w, int h);
        void        LoadMap(char *filename);
        bool        SaveMap(char *filename);
        void        DisposeMap();

        void        SetRadarColor(GLuint r, GLuint g, GLuint b, GLuint a) { radarColor.r = r; radarColor.g = g; radarColor.b = b; radarColor.a = a; }

        void        LoadPalette(char *filename, int tw, int th);
        void        SetPalette(SS_TilePalette *palette);
        void        DisposePalette();

        void        Move(float x, float y) { xpos = x; ypos = y; }

        void        Render(float x, float y, float width, float height);
        void        Render(SS_World *world, Uint32 flags);
        void        DrawTile(Uint8 tile, GLfloat x, GLfloat y);

    private:
        void        Init();
};



#pragma mark -
//--------------------------------------------------------------
// SS_TileLayer
// A tilemap that is included as a layer in the world
//
class SS_TileLayer : public SS_Layer
{
    public:
        SS_TileMap      *tileMap;                   // the layer's tile map

                    SS_TileLayer();
                    SS_TileLayer(Uint32 f);
                    SS_TileLayer(SS_TileMap *map);
                    SS_TileLayer(SS_TileMap *map, Uint32 f);

                    ~SS_TileLayer();
        virtual inline layerType   Type() { return SS_LAYER_TILE; }
        void        SetTileMap(SS_TileMap *map);

        void        Process();
        void        Animate();
        void        Render();

    private:
        void        Init(Uint32 f=SS_NONE);
};

#endif

