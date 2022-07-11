/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_LayerItem.cpp
 *
 *  $Id: SS_LayerItem.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_LayerItem.h"

#include "SS_Game.h"
#include "SS_Files.h"
#include "SS_ItemGroup.h"
#include "SS_Layer.h"
#include "SS_Utilities.h"
#include "SS_World.h"


//--------------------------------------------------------------
// SS_LayerItem
// A graphical object
//--------------------------------------------------------------

SS_LayerItem::SS_LayerItem()
{
    DEBUGF(1, "[%08X] SS_LayerItem() CONSTRUCTOR\n", this);

    Init();
}

SS_LayerItem::~SS_LayerItem()
{
    DEBUGF(1, "[%08X] ~SS_LayerItem()\n", this);

    RemoveSelf();
}


//
// Init()
//
void SS_LayerItem::Init()
{
    DEBUGF(1, "[%08X] SS_LayerItem::Init()\n", this);

    // Helpful name
    name            = NULL;

    // System connections
    world           = NULL;
    layer           = NULL;
    group           = NULL;
//  mainNode        = NULL;

    oldW            = 0.0f;
    oldH            = 0.0f;

    // Size
    width           = 0.0f;
    height          = 0.0f;

    // Handle
    xhandle         = 0.0f;
    yhandle         = 0.0f;

    // Death Fuse
    lifespan        = 0;        // not dying

    // Frame information
    frameCount      = 0;
    currFrame       = 0;
    animFirst       = 0;
    animLast        = 0;

    // Animation behavior
    animIncrement   = 1;
    animInterval    = 20;
    lastAnimTime    = 0;
    animProc        = defaultAnimProc;

    // Motion behavior
    moveInterval    = 20;
    lastMoveTime    = 0;
    moveProc        = NULL;

    // Tint
    SetTint(0xFF, 0xFF, 0xFF, 0xFF);

    // Orientation, Scale, Position, and motion
    SetRotation(0.0f);

    xscale          = 1.0f;
    yscale          = 1.0f;

    xpos            = 0.0f;
    ypos            = 0.0f;

    xvel            = 0.0f;
    yvel            = 0.0f;
    spin            = 0.0f;

    // Flags
    flags           = SS_AUTOMOVE;
    removeFlag      = false;
    hideFlag        = false;

    // Radar appearance
    radarSize       = 2.0f;
    SetRadarColor(0xF0, 0xF0, 0xF0, 0xFF);

    // Scratchpad Data
    for (int i=SS_MISC_FIELDS; i--;) {
        misc[i] = 0.0f;
        yesno[i] = false;
    }
}


//
// SetWorld w
// Set this layer item's world
//
void SS_LayerItem::SetWorld(SS_World *w)
{
    DEBUGF(1, "[%08X] SS_LayerItem::SetWorld(%08X)\n", this, w);

    if (world = w) {
        oldW = w->ZoomWidth();
        oldH = w->ZoomHeight();
    }
}


//
// SetLayer(l)
// Set this sprite's layer and world
//
void SS_LayerItem::SetLayer(SS_Layer *l)
{
    DEBUGF(1, "[%08X] SS_LayerItem::SetLayer(%08X)\n", this, l);

    SetWorld((layer = l) ? l->World() : NULL);
}


//
// SetGroup(l)
// Set this sprite's layer and world
//
void SS_LayerItem::SetGroup(SS_ItemGroup *g)
{
    DEBUGF(1, "[%08X] SS_LayerItem::SetGroup(%08X)\n", this, g);

    SetLayer((group = g) ? g->layer : NULL);
}


//
// operator=
//
const SS_LayerItem& SS_LayerItem::operator=(const SS_LayerItem &src)
{
    DEBUGF(1, "[%08X] SS_LayerItem::operator=(%08X)\n", this, &src);

    if (&src != this)
    {
        SS_Broadcaster::operator=(src);
        SS_Listener::operator=(src);
        SS_RefCounter::operator=(src);

        // Initialize some to default values
        world           = NULL;
        layer           = NULL;
        group           = NULL;

        oldW            = 0.0f;
        oldH            = 0.0f;

        frameCount      = 0;
        currFrame       = 0;

        removeFlag      = false;

        lastMoveTime    = 0;
        lastAnimTime    = 0;

        // Some are copied directly
        animProc        = src.animProc;
        animFirst       = src.animFirst;
        animLast        = src.animLast;
        animIncrement   = src.animIncrement;
        animInterval    = src.animInterval;

        moveProc        = src.moveProc;
        moveInterval    = src.moveInterval;

        flags           = src.flags;
        hideFlag        = src.hideFlag;
        lifespan        = src.lifespan;

        xscale          = src.xscale;
        yscale          = src.yscale;

        xhandle         = src.xhandle;
        yhandle         = src.yhandle;

        rotation        = src.rotation;
        rotindex        = src.rotindex;

        xpos            = src.xpos;
        ypos            = src.ypos;
        xvel            = src.xvel;
        yvel            = src.yvel;
        spin            = src.spin;
        tint            = src.tint;

        width           = src.width;
        height          = src.height;

        tint            = src.tint;
        radarSize       = src.radarSize;
        radarColor      = src.radarColor;

        for (int i=SS_MISC_FIELDS; i--;) {
            misc[i]     = src.misc[i];
            yesno[i]    = src.yesno[i];
        }

        // The name needs to be copied
        if (name) delete[] name;
        name = src.name ? newstring(src.name) : NULL;

        src.AddPeer(this);
    }

    return *this;
}


//
// Tokenize(datafile)
//
void SS_LayerItem::Tokenize(SS_FlatFile &dataFile)
{
    dataFile.EnterContext("LayerItem");

    dataFile.SetToken("oldW", oldW);
    dataFile.SetToken("oldH", oldH);

    dataFile.SetToken("frameCount", frameCount);

//  dataFile.SetToken("token", animProc);           // procs will have names
    dataFile.SetToken("animFirst", animFirst);
    dataFile.SetToken("animLast", animLast);
    dataFile.SetToken("animInterval", animInterval);
    dataFile.SetToken("animIncrement", animIncrement);

//  dataFile.SetToken("token", moveProc);           // procs will have names
    dataFile.SetToken("moveInterval", moveInterval);

    dataFile.SetToken("flags", flags);          // flags subject to change

    dataFile.SetToken("xscale", xscale);
    dataFile.SetToken("yscale", yscale);

    dataFile.SetToken("xhandle", xhandle);
    dataFile.SetToken("yhandle", yhandle);

    dataFile.SetToken("rotation", rotation);
//  dataFile.SetToken("token", rotindex);           // derive from rotation

    dataFile.SetToken("xpos", xpos);
    dataFile.SetToken("ypos", ypos);
    dataFile.SetToken("xvel", xvel);
    dataFile.SetToken("yvel", yvel);
    dataFile.SetToken("tint", tint);

    dataFile.SetToken("width", width);          // during load, set after loading frames
    dataFile.SetToken("height", height);            // during load, set after loading frames

    dataFile.SetToken("radarSize", radarSize);
    dataFile.SetToken("radarColor", radarColor);

    dataFile.SetToken("yesno", yesno, SS_MISC_FIELDS * sizeof(bool));
    dataFile.SetToken("misc", misc, SS_MISC_FIELDS * sizeof(float));
}


//
// RemoveSelf
//
// Unlink this item from its group's or layer's linked list
//
// TODO: Give layeritems a list of node references that allow it
//  to track its membership in all lists. This method will then
//  simply remove the item from all lists.
//
void SS_LayerItem::RemoveSelf() // aka "Unlink()"
{
    DEBUGF(1, "[%08X] SS_LayerItem::RemoveSelf()\n", this);

    if (group) {
        group->Remove(dynamic_cast<SS_Collider*>(this));
        SetGroup(NULL);
    }
    else if (layer) {
//      layer->visibleList.Remove(this);
        layer->Remove(this);
        SetLayer(NULL);
    }
}


//
// AddPeer
//
void SS_LayerItem::AddPeer(SS_LayerItem *item) const
{
    if (layer != NULL)
        layer->AddItem(item);
}


//
// PrependPeer
//
void SS_LayerItem::PrependPeer(SS_LayerItem *item) const
{
    if (layer != NULL)
        layer->PrependItem(item);
}


#pragma mark -
//
// Move(x, y, h, v, a)
//
void SS_LayerItem::Move(float x, float y, float h, float v, float a)
{
    SS_Game::Rotate(h, v, a);
    Move(x + h, y + v);
}


//
// Center
// Move the sprite to the center of the screen
// (even if the world has been scrolled)
//
void SS_LayerItem::Center()
{
    Uint32      f = layer->flags;
    SS_World    *w = World();
    float       x, y;

    if (f & SS_NOZOOM) {
        x = w->ViewWidth() / 2.0f;
        y = w->ViewHeight() / 2.0f;
    } else {
        x = w->ZoomWidth() / 2.0f;
        y = w->ZoomHeight() / 2.0f;
    }

    if (!(f & SS_NOSCROLL)) {
        x += w->left;
        y += w->top;
    }

    Move(x, y);
}

#pragma mark -
//
// IsVisible()
//
bool SS_LayerItem::IsVisible() const
{
    if (hideFlag)
        return false;
    else if (group)
        return group->IsVisible();
    else
        return true;
}


//
// GlobalPosition(*point)
// Return the sprite's world-relative position
// Accounts for spatially-scaled (parallax) layers
// - always returns coordinates relative to scale=1
//
void SS_LayerItem::GlobalPosition(SS_Point *point, bool layerOnly) const
{
    //
    // 1. Get the base coordinates
    //
    float   outx = xpos;                // starting position (local to my container)
    float   outy = ypos;
    float   outr = rotation;            // starting rotation

    SS_LayerItem *grp, *nxt;
    SS_LayerItem const *outer;

    //
    // 2. Adjust for group nesting
    //
    if (nxt = group)
    {
        float   ucos, usin, rotx, roty;
        Uint16  i;

        // adjust for the parent rotation and position
        do {
            grp = nxt;
            i = grp->rotindex;
            ucos = SS_Game::Cos(i);
            usin = SS_Game::Sin(i);
            rotx = (outx * ucos - outy * usin);
            roty = (outy * ucos + outx * usin);
            outx = rotx + grp->xpos - grp->xhandle;
            outy = roty + grp->ypos - grp->yhandle;
            outr += grp->rotation;
        } while (nxt = grp->group);

        outer = grp;
    }
    else
        outer = this;

/*
    float   s = layer->spatialScale;
    if (s != 1.0f) {
        SS_World    *w = World();
        outx += w->left - (w->left / s);
        outy += w->top - (w->top / s);
    }
*/

    //
    // Continue for any world-relative scaling
    //
    if (!layerOnly)
    {
        outx += layer->xoffset;
        outy += layer->yoffset;

        Uint32 f = outer->flags | layer->flags;

        //
        // If this layer doesn't scroll we need to make
        // the coordinates world-relative
        //
        if (f & SS_NOSCROLL) {
            if (f & SS_NOZOOM) {
                outx /= world->Zoom();  // adjust for view zoom
                outy /= world->Zoom();
            }

            outx += world->left;            // adjust for view scroll
            outy += world->top;
        }
    }

    point->x = outx;
    point->y = outy;
    point->r = outr;
    point->i = SS_ROTINDEX(outr);
}


//
// GlobalVelocity(*xv, *yv)
// Return the sprite's world-relative velocity
//
void SS_LayerItem::GlobalVelocity(float *xv, float *yv) const
{
    //
    // 1. Get the base coordinates
    //
    float   outx = xvel;                // starting velocity (local to my container)
    float   outy = yvel;

    SS_LayerItem *grp, *nxt;
    SS_LayerItem const *outer;

    //
    // 2. Adjust for group nesting
    //
    if (nxt = group)
    {
        float   ucos, usin, rotx, roty;
        Uint16  i;

        // adjust for the parent rotation and position
        do {
            grp = nxt;
            i = grp->rotindex;
            ucos = SS_Game::Cos(i);
            usin = SS_Game::Sin(i);
            rotx = (outx * ucos - outy * usin);
            roty = (outy * ucos + outx * usin);
            outx = rotx + grp->xvel;
            outy = roty + grp->yvel;
        } while (nxt = grp->group);

        outer = grp;
    }
    else
        outer = this;

/*
    float   s = layer->spatialScale;
    if (s != 1.0f) {
        SS_World    *w = World();
        outx += w->left - (w->left / s);
        outy += w->top - (w->top / s);
    }
*/

    *xv = outx;
    *yv = outy;
}


//
// LocalPosition(*point)
// Accessor for the sprite's local position
//
void SS_LayerItem::LocalPosition(SS_Point *point) const
{
    point->x = xpos;
    point->y = ypos;
    point->r = rotation;
    point->i = SS_ROTINDEX(rotation);
}


//
// LocalAngle
// Get a local angle from a global angle with respect to this item's container(s)
//
float SS_LayerItem::LocalAngle(float angle) const
{
    SS_LayerItem *grp;

    if (grp = group)
        do { angle -= grp->rotation; } while (grp = grp->group);

    return BASEANGLE(angle);
}


//
// SetGlobalRotation
// Set the sprite's local angle to reflect a global angle
//
void SS_LayerItem::SetGlobalRotation(float rot)
{
    SS_LayerItem *grp;

    if (grp = group)
        do { rot -= grp->rotation; } while (grp = grp->group);

    SetRotation(BASEANGLE(rot));
}


//
// DistanceSquaredTo(x, y)
// Ask Pythagoras for the distance
//
double SS_LayerItem::DistanceSquaredTo(float x2, float y2) const
{
    SS_Point    this_pos;
    GlobalPosition(&this_pos);

    float a = x2 - this_pos.x;
    float b = y2 - this_pos.y;
    return a * a + b * b;
}


//
// DistanceSquared(item)
// Ask Pythagoras for the distance
//
double SS_LayerItem::DistanceSquaredTo(SS_LayerItem *other) const
{
    SS_Point    this_pos;
    SS_Point    other_pos;

    GlobalPosition(&this_pos);
    other->GlobalPosition(&other_pos);

    float a = other_pos.x - this_pos.x;
    float b = other_pos.y - this_pos.y;
    return a * a + b * b;
}


//
// AngleTo(x, y)
//
float SS_LayerItem::AngleTo(float x, float y) const
{
    SS_Point    this_pos;
    GlobalPosition(&this_pos);
    return SS_Game::AngleOfXY(x - this_pos.x, y - this_pos.y);
}


//
// AngleToSprite(sprite)
//
float SS_LayerItem::AngleTo(SS_LayerItem *other) const
{
    SS_Point    this_pos;
    SS_Point    other_pos;
    GlobalPosition(&this_pos);
    other->GlobalPosition(&other_pos);
    return SS_Game::AngleOfXY(other_pos.x - this_pos.x, other_pos.y - this_pos.y);
}


//
// AngleToPointer
// Find the angle between an item and the pointer
//
float SS_LayerItem::AngleToPointer() const
{
    if (world)
        return AngleTo( world->mousex / world->Zoom() + world->left,
                        world->mousey / world->Zoom() + world->top  );
    else
        return 0.0f;
}


//
// SetMoveProc
//
void SS_LayerItem::SetMoveProc(spriteProcPtr proc)
{
    DEBUGF(1, "[%08X] SS_LayerItem::SetMoveProc(%08X)\n", this, proc);

    lastMoveTime = 0;
    moveProc = proc;
}


//
// SetProcessInterval(ms)
//
void SS_LayerItem::SetProcessInterval(Uint32 ms)
{
    moveInterval = 0;
    lastMoveTime = world ? world->GetWorldTime() + RANDINT(0, ms) : 0;
    moveInterval = ms;
}

//
// PrimeProcessor
//
void SS_LayerItem::PrimeProcessor()
{
    lastMoveTime = world ? world->GetWorldTime() + RANDINT(0, moveInterval) : 0;
}

//
// SetAnimateProc(proc)
//
void SS_LayerItem::SetAnimateProc(spriteProcPtr proc)
{
    DEBUGF(1, "[%08X] SS_LayerItem::SetAnimateProc(%08X)\n", this, proc);

    lastAnimTime = 0;
    animProc = proc;
}


//
// SetAngularVelocity(angle, velocity)
//
void SS_LayerItem::SetAngularVelocity(float ang, float vel)
{
    DEBUGF(1, "[%08X] SS_LayerItem::SetAngularVelocity(%.2f, %.2f)\n", this, ang, vel);

    Uint16  i = SS_ROTINDEX(ang);
    xvel = SS_Game::Sin(i) * vel;
    yvel = -SS_Game::Cos(i) * vel;
}


//
// HeadTowards(angle)
//
void SS_LayerItem::HeadTowards(float ang, float rate)
{
    Uint16  i = SS_ROTINDEX(ang + 270.0f);
    xvel += SS_Game::Cos(i) * rate;
    yvel += SS_Game::Sin(i) * rate;
}


//
// PushAndPrepareMatrix
// Save the current model matrix and transform it into our local coordinate system
//
void SS_LayerItem::PushAndPrepareMatrix()
{
    float   x = xpos, y = ypos;
    Uint32  f = flags;

    //
    // IF
    //  - the sprite is scroll-immune
    //  - the sprite is not zoom-immune
    //  - the layer is scrolling
    //
    // THEN
    //  - subtract half of the difference between the current zoomed size
    //      and the zoomed size when the sprite was created
    //      It works, think about it....
    //
    if ((f & SS_NOSCROLL) && !(f & SS_NOZOOM) && !(layer->flags & SS_NOSCROLL)) {
        x -= (world->ZoomWidth() - oldW) / 2;
        y -= (world->ZoomHeight() - oldH) / 2;
    }

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(x, y, 0.0f);
    glRotatef(rotation, 0.0f, 0.0f, 1.0f);
    glScalef(xscale, yscale, 1.0f);
}


//
// _Process
// Do all scheduled processing based on the time elapsed.
//
void SS_LayerItem::_Process()
{
    if (Flags(SS_AUTOMOVE) && world->fireAuto)
        AutoMove();

    if (moveInterval && (world->ticks - lastMoveTime >= moveInterval))
    {
        Process();

        if (lastMoveTime)
            lastMoveTime = world->ticks;
        else
            lastMoveTime = world->ticks + RANDINT(0, moveInterval);
    }
}


//
// Process
//
// Call the move proc if it's set.
// Subclasses will want to override this.
//
void SS_LayerItem::Process()
{
    if (moveProc) (moveProc)(this);
}


//
// Animate
//
void SS_LayerItem::_Animate()
{
    if (animInterval) {
        if (world->ticks - lastAnimTime >= animInterval)
        {
            Animate();

            if (lastAnimTime)
                lastAnimTime = world->ticks;
            else
                lastAnimTime = world->ticks + RANDINT(0, animInterval);
        }
    }
}


//
// Animate
//
void SS_LayerItem::Animate()
{
    if (animProc) (animProc)(this);
}


//
// Render(tint)
//
void SS_LayerItem::Render(const SScolorb &inTint)
{
    PushAndPrepareMatrix();

    gl_do_texture(0);
    gl_do_blend(1);
    gl_line_width(1);
    gl_antialias(false);

    SScolorb outTint;
    MultiplyColorQuads(tint, inTint, outTint);
    glColor4ubv((GLubyte*)&outTint);

    glRectf(-xhandle, -yhandle, -xhandle+width, -yhandle+height);

    RestoreMatrix();
}


//
// BounceOff(item)
// Bounce the item off another item
//
void SS_LayerItem::BounceOff(SS_LayerItem *item)
{
    float   h = xvel;
    float   v = yvel;

    SetVelocity(item->xvel * 0.9f, item->yvel * 0.9f);
    item->SetVelocity(h * 0.9f, v * 0.9f);
}

//
// SendCommand
//
void SS_LayerItem::SendCommand(long command)
{
    if (layer) layer->HandleCommand(command);
}

//
// TestPointCollision(point)
//
bool SS_LayerItem::TestPointCollision(SS_Point *globalPoint)
{
    return TestPointCollision(globalPoint->x, globalPoint->y);
}

//
// TestPointCollision(item)
//
bool SS_LayerItem::TestPointCollision(SS_LayerItem *item)
{
    SS_Point    otherPoint;
    item->GlobalPosition(&otherPoint);
    return TestPointCollision(otherPoint.x, otherPoint.y);
}

//
// TestPointCollision(x, y, local?)
//
// TODO: Test oval or rect intersection
//
bool SS_LayerItem::TestPointCollision(float x, float y, bool isLocal)
{
    SS_Point    mypos;
    Uint16      irot;
    float       usin, ucos;
    float       rx, ry;

    if (isLocal)
        LayerPosition(&mypos);
    else
        GlobalPosition(&mypos);

    x -= mypos.x;
    y -= mypos.y;

    // - unrotate and untranslate the point, then make corner-relative
    irot = -mypos.i;
    ucos = SS_Game::Cos(irot);
    usin = SS_Game::Sin(irot);
    rx = (x * ucos - y * usin) + xhandle;
    ry = (y * ucos + x * usin) + yhandle;

    // return if the point is in bounds
    return (rx >= 0.0f && rx < width && ry >= 0.0f && ry < height);
}


#pragma mark -
//--------------------------------------------------------------
//
//  Standard animation and motion procedures
//
//--------------------------------------------------------------

void SS_LayerItem::defaultAnimProc(SS_LayerItem *item)
{
    Sint16      inc     = item->animIncrement;
    Sint16      frame;

    if (inc)
    {
        frame = item->currFrame + inc;

        if (frame > item->animLast)
            frame -= (item->animLast - item->animFirst + 1);
        else if (frame < item->animFirst)
            frame += (item->animLast - item->animFirst + 1);

        item->currFrame = frame;
    }
}


