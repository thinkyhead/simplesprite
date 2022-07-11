/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_GUI.cpp
 *
 *  $Id: SS_GUI.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include <stdlib.h>

#define BORDERS_ARE_LINES 0

#include "SS_GUI.h"
#include "SS_LayerItem.h"
#include "SS_Sprite.h"
#include "SS_ItemGroup.h"
#include "SS_Utilities.h"
#include "SS_Vectors.h"
#include "SS_World.h"

SS_LayerItem        *SS_Checkbox::checkboxSprite = NULL;
SS_LayerItem        *SS_RadioButton::radioButtonSprite = NULL;
SS_ItemGroup        *SS_Slider::sliderSprite = NULL;
SS_Scrollbar        *SS_Scrollbar::defaultScrollbar = NULL;

/*
#undef SS_DEBUG
#define SS_DEBUG 1
*/

//--------------------------------------------------------------
// SS_World
//

//
// NewGUI
//
SS_GUI* SS_World::NewGUI()
{
    SS_GUI  *gui = new SS_GUI();
    AddLayer(gui);
    return gui;
}

SS_GUI* SS_World::NewGUI(SS_SFont *font)
{
    SS_GUI  *gui = new SS_GUI(font);
    AddLayer(gui);
    return gui;
}

#pragma mark -
//--------------------------------------------------------------
// SS_GUI
//

SS_GUI *SS_GUI::activeGUI = NULL;

//
// Constructors
//
SS_GUI::SS_GUI() : SS_Layer(SS_NOSCROLL|SS_NOZOOM)
{
    Init();
}

SS_GUI::SS_GUI(SS_SFont *font) : SS_Layer(SS_NOSCROLL|SS_NOZOOM)
{
    Init();

    SetFont(font);
}

//
// Destructor
//
SS_GUI::~SS_GUI()
{
    if (SS_GUI::activeGUI == this)
        SS_GUI::ActivateNone();
}

void SS_GUI::Init()
{
    DEBUGF(1, "[%08X] SS_GUI::Init()\n", this);

    // Event listeners
    clickedGadget   = NULL;
    keyFocusGadget  = NULL;
    dormant         = false;

    SetFont((SS_SFont*)NULL);
}

//
// SetGadgetSuite
// Set a single
//
void SS_GUI::SetGadgetSuite(SS_LayerItem *check, SS_ItemGroup *slider, SS_Scrollbar *scroller)
{
    SS_Checkbox::checkboxSprite = check;
    SS_Slider::sliderSprite = slider;
    SS_Scrollbar::defaultScrollbar = scroller;
}

/*
//
// LoadTheme
//
void SS_GUI::LoadTheme(char *theme)
{
    Folder::cdDataFolder(theme);

    SS_Checkbox::checkboxSprite = SS_Gadget::MakeSprite(
                                "checkbox_off.png",
                                "checkbox_on.png",
                                "checkbox_off_roll.png",
                                "checkbox_on_roll.png"
                                );

    SS_Slider::sliderSprite = SS_Gadget::MakeSpriteGroup(
                                "slider.png",
                                "thumb.png",
                                "thumb_clicked.png",
                                "thumb_hover.png"
                                );
}
*/

//
// Enable
//
void SS_GUI::SetEnabled(bool e)
{
    SS_Layer::SetEnabled(e);

    if (e)
    {
        SS_Gadget   *gad;

        rolloverList.Clear();

        clickedGadget = NULL;
        keyFocusGadget = NULL;

        SS_GadgetIterator itr = gadgetList.GetIterator();
        while ((gad = itr.NextItem()))
            gad->Reset();
    }
}


//
// Activate
//
void SS_GUI::Activate()
{
    if (SS_GUI::activeGUI != this)
    {
        if (SS_GUI::activeGUI != NULL)
            SS_GUI::activeGUI->ExitAllRollovers();

        SS_GUI::activeGUI = this;
    }
}


//
// ActivateNone
//
void SS_GUI::ActivateNone()
{
    if (SS_GUI::activeGUI != NULL) {
        SS_GUI::activeGUI->ExitAllRollovers();
        SS_GUI::activeGUI = NULL;
    }
}


//
// SetWorld(w)
//
void SS_GUI::SetWorld(SS_World *w)
{
    DEBUGF(1, "[%08X] SS_GUI::SetWorld(%08X)\n", this, w);

    SS_Layer::SetWorld(w);

    SS_Gadget   *gad;
    SS_GadgetIterator itr = gadgetList.GetIterator();
    while ((gad = itr.NextItem()))
        gad->SetWorld(w);

/*
    SS_Gadget   *gad = firstGadget;

    while (gad) {
        gad->SetWorld(w);
        gad = gad->nextGadget;
    }
*/
}

//
// AddGadget(gadget)
//
void SS_GUI::AddGadget(SS_Gadget *gadget)
{
    DEBUGF(1, "[%08X] SS_GUI::AddGadget(%08X)\n", this, gadget);

    gadget->SetGUI(this);
    gadget->guiNode = gadgetList.Append(gadget);
}

//
// SetKeyFocus(gadget)
//
void SS_GUI::SetKeyFocus(SS_Gadget *g)
{
    if (keyFocusGadget != g)
    {
        if (keyFocusGadget) {
            SS_Event ssevent;
            ssevent.type = SS_EVENT_LOSE_FOCUS;
            (void)keyFocusGadget->HandleEvent(&ssevent);
        }

        keyFocusGadget = g;

        if (g) {
            SS_Event ssevent;
            ssevent.type = SS_EVENT_GAIN_FOCUS;
            (void)g->HandleEvent(&ssevent);
        }
    }
}

//
// HandleEvent(event)
//
bool SS_GUI::HandleEvent(SDL_Event *e)
{
    //DEBUGF(1, "[%08X] SS_GUI::HandleEvent(e)\n", this);

    if (dormant)
        return false;

    SS_Event    ssevent;
    bool        did = false;
    SS_World    *w = world;
    float       gx = w->mousex;
    float       gy = w->mousey;
    float       px = gx - xoffset;
    float       py = gy - yoffset;
    bool        isInside;

    // Populate the event with mouse info
//  ssevent.xrel = w->mousexrel;            // relative mouse motion
//  ssevent.yrel = w->mouseyrel;
    ssevent.xglob = gx; ssevent.yglob = gy; // global coordinates
    ssevent.xgui = px;  ssevent.ygui = py;  // gui-local coordinates

    // Get the topmost gadget under the mouse
    SS_Gadget   *theGad = GadgetAtXY(px, py);

    // If a gadget was clicked then act on that instead
    if (clickedGadget) {
        isInside = (theGad == clickedGadget);
        theGad = clickedGadget;
    }
    else
        isInside = false;

    // Store gadget-local coordinates in the event
    if (theGad)
    {
        theGad->pointerIn = clickedGadget == NULL || isInside;

        // Deactivate the old GUI if there is one
        Activate();

        ssevent.x = px - theGad->xpos;
        ssevent.y = py - theGad->ypos;
    }

    // Continue based on the event type
    switch (e->type)
    {
        // Key pressed
        case SDL_KEYDOWN:

            // Send the event to the gadget with the keyboard focus.
            // If nothing has keyboard focus the event persists.
            //
            if (keyFocusGadget) {
                ssevent.type = SS_EVENT_KEYDOWN;
                ssevent.key = e->key.keysym.sym;
                ssevent.mod = e->key.keysym.mod;
                did = keyFocusGadget->HandleEvent(&ssevent);
            }

            break;

        // Mouse button clicked
        case SDL_MOUSEBUTTONDOWN:

            // Multiple Buttons
            switch(e->button.button)
            {
                // Left button
                case SDL_BUTTON_LEFT:

                    if (theGad)
                    {
                        bool take_click = true;

                        if (theGad->Flags(GAD_KEYFOCUS))
                        {
                            //
                            // Give keyboard focus to the clicked gadget
                            // Take the activating click if specified
                            //
                            if (keyFocusGadget != theGad)
                            {
                                SetKeyFocus(theGad);
                                take_click = theGad->Flags(GAD_ACTIVATECLICK);
                                did = true;
                            }
                        }

                        // Taking the click?
                        if (take_click)
                        {
                            // Store the clicked state in the gadget and gui
                            clickedGadget = theGad;
                            w->LatchLayer(this);

                            // Send a CLICK event to the gadget
                            ssevent.type = SS_EVENT_CLICK;
                            theGad->clickx = ssevent.x;
                            theGad->clicky = ssevent.y;
                            did = theGad->HandleEvent(&ssevent);
                        }
                    }
                    else if (clickedGadget)     // a click in no gadget
                    {
                        clickedGadget = NULL;
                        w->LatchLayer(NULL);
                    }
                    break;

                case SDL_BUTTON_MIDDLE:
                    break;

                case SDL_BUTTON_RIGHT:
                    break;

                case SDL_BUTTON_WHEELUP:
                    break;

                case SDL_BUTTON_WHEELDOWN:
                    break;
            }
            break;

        case SDL_MOUSEMOTION:

            if (clickedGadget)      // was something dragged?
            {
                ssevent.type = SS_EVENT_DRAG;
                did = clickedGadget->HandleEvent(&ssevent);
            }
            else
            {
                SS_GadgetIterator   itr = gadgetList.GetIterator();
                SS_Gadget   *gad;
                itr.End();

                // do rollover behavior
                while ((gad = itr.PreviousItem()))
                {
                    //
                    // Rollover Enters
                    //
                    if (gad->ContainsPoint(px, py))
                    {
                        if (!gad->IsEntered())
                        {
                            gad->MarkEntered();
                            ssevent.type = SS_EVENT_ENTER;
                            did = gad->HandleEvent(&ssevent);
                        }
                        else
                        {
                            ssevent.type = SS_EVENT_MOVE;
                            did = gad->HandleEvent(&ssevent);
                        }
                    }
                    //
                    // Rollover Exits
                    //
                    else
                    {
                        if (gad->IsEntered())
                        {
                            gad->MarkExited();
                            ssevent.type = SS_EVENT_EXIT;
                            did = gad->HandleEvent(&ssevent);
                        }
                    }
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:

            if (clickedGadget && isInside)
            {
                ssevent.type = SS_EVENT_UNCLICK;
                did = theGad->HandleEvent(&ssevent);
            }

            clickedGadget = NULL;
            w->LatchLayer(NULL);
            break;
    }

    return did;
}

//
// ExitAllRollovers
//
void SS_GUI::ExitAllRollovers()
{
    DEBUGF(1, "[%08X] SS_GUI::ExitAllRollovers()\n", this);

    SS_Event    ssevent;

    ssevent.type = SS_EVENT_EXIT;

    SS_GadgetIterator   itr = gadgetList.GetIterator();
    SS_Gadget   *gad;
    itr.End();

    while ((gad = itr.PreviousItem()))
    {
        if (gad->IsEntered())
        {
            gad->MarkExited();
            (void) gad->HandleEvent(&ssevent);
        }
    }
}


//
// GadgetAtPointer
//
SS_Gadget* SS_GUI::GadgetAtPointer()
{
    return MousePointer() ?
        GadgetAtXY(MousePointer()->xpos /* - xoffset */, MousePointer()->ypos /* - yoffset */) :
        NULL;
}

//
// GadgetAtXY(x, y)
//
SS_Gadget* SS_GUI::GadgetAtXY(float x, float y)
{
    SS_GadgetIterator   itr = gadgetList.GetIterator();
    itr.End();

    SS_Gadget   *gad;

    while ((gad = itr.PreviousItem()) && !gad->ContainsPoint(x, y)) {}

    return gad;
}

//
// Animate
//
void SS_GUI::Animate()
{
    SS_GadgetIterator   itr = gadgetList.GetIterator();
    SS_Gadget   *gad;

    while ((gad = itr.NextItem()))
        gad->Animate();
}

//
// Render
//
void SS_GUI::Render()
{
    PrepareMatrix();

    SS_GadgetIterator   itr = gadgetList.GetIterator();
    SS_Gadget   *gad;

    while ((gad = itr.NextItem()))
        gad->Render(tint);
}


#pragma mark -
//--------------------------------------------------------------
// SS_Gadget
//

SS_Gadget::SS_Gadget()
{
    DEBUGF(1, "[%08X] SS_Gadget() CONSTRUCTOR\n", this);

    Init();
}

SS_Gadget::SS_Gadget(SS_GUI *g)
{
    DEBUGF(1, "[%08X] SS_Gadget(%08X) CONSTRUCTOR\n", this, g);

    Init();
    g->AddGadget(this);
}

SS_Gadget::SS_Gadget(SS_GUI *g, float w, float h, Uint32 f)
{
    DEBUGF(1, "[%08X] SS_Gadget(%08X, %f, %f, %d) CONSTRUCTOR\n", this, g, w, h, f);

    Init();

    width = w;
    height = h;
    g->AddGadget(this);
    flags = f;
}

//
// SS_Gadget(gui, item)
//
//  Make a gadget that uses a clone of the specified layer-item
//  (presumed to be a multi-framed sprite or item-group)
//
SS_Gadget::SS_Gadget(SS_GUI *g, SS_LayerItem *item)
{
    DEBUGF(1, "[%08X] SS_Gadget(gui, item) CONSTRUCTOR\n", this);

    Init();

    if (item != NULL)
        SetItem(item->Clone());

    g->AddGadget(this);
}

SS_Gadget::SS_Gadget(SS_GUI *g, SS_ItemGroup *group)
{
    DEBUGF(1, "[%08X] SS_Gadget(gui, group) CONSTRUCTOR\n", this);

    Init();

    if (group != NULL)
        SetItemGroup(group->Clone());

    g->AddGadget(this);
}

SS_Gadget::SS_Gadget(SS_GUI *g, char *file1, char *file2, char *file3, char *file4)
{
    DEBUGF(1, "[%08X] SS_Gadget(gui, \"%s\", \"%s\", \"%s\", \"%s\") CONSTRUCTOR\n", this, file1, file2, file3, file4);

    Init();
    SetItemGroup(file1, file2, file3, file4);
    g->AddGadget(this);
}

SS_Gadget::~SS_Gadget()
{
    if (gadGroup)
        delete gadGroup;
    else if (gadSprite)
        delete gadSprite;

    if (labelString)
        delete labelString;

    RemoveSelf();
}

//
// Init
//
void SS_Gadget::Init()
{
    DEBUGF(1, "[%08X] SS_Gadget::Init()\n", this);

    gui             = NULL;

    guiNode         = NULL;
    rolloverNode    = NULL;

    lastClickTime   = 0;
    clickCount      = 0;
    maxClicks       = 1;

    gadGroup        = NULL;
    gadSprite       = NULL;
    bkgdSprite      = NULL;
    labelString     = NULL;
    labelAlignment  = (stringAlign)(SA_LEFT|SA_CENTERV);

    colorSet normColor = { 2,
            { 0xFF, 0xFF, 0xFF, 0xFF },     // border
            { 0x00, 0x00, 0x00, 0x80 },     // fill
            { 0xFF, 0xFF, 0xFF, 0xFF } };   // label

    colorSet hoverColor = { 2,
            { 0xFF, 0xFF, 0x00, 0xFF },     // border
            { 0x00, 0x00, 0x40, 0x80 },     // fill
            { 0xFF, 0xFF, 0x00, 0xFF } };   // label

    colorSet pressColor = { 2,
            { 0x00, 0xFF, 0x00, 0xFF },     // border
            { 0x00, 0x40, 0x40, 0xFF },     // fill
            { 0xFF, 0xFF, 0x00, 0xFF } };   // label

    setNormal   = normColor;
    setHover    = hoverColor;
    setPressed  = pressColor;
    setDisabled = normColor;
    setDisabled.fillColor.a = setDisabled.borderColor.a = setDisabled.labelColor.a = 0x7F;

    SetFont(NULL);

    enabled             = true;
    hideFlag            = false;
    SetState(0);
    SetFlags(GAD_DRAWBACKGROUND);

    Move(0, 0);
    SetRegion(0, 0);

    Command             = 0x00000000;
    mouseEnterProc      = NULL;
    mouseMoveProc       = NULL;
    mouseExitProc       = NULL;
    mouseDownProc       = NULL;
    mouseDragProc       = NULL;
    mouseUpProc         = NULL;
    keyDownProc         = NULL;
    keyUpProc           = NULL;
    idleProc            = NULL;
    gainFocusProc       = NULL;
    loseFocusProc       = NULL;
}

//
// Reset
//
void SS_Gadget::Reset()
{
    MarkExited();
    lastClickTime   = 0;
    clickCount      = 0;
}

//
// RemoveSelf
//
void SS_Gadget::RemoveSelf()
{
    DEBUGF(1, "[%08X] SS_Gadget::RemoveSelf()\n", this);

    if (gui) {
        MarkExited();
        guiNode->RemoveSelf();
        guiNode = NULL;
        gui = NULL;
    }
}

//
// SetWorld(w)
//
void SS_Gadget::SetWorld(SS_World *w)
{
    DEBUGF(1, "[%08X] SS_Gadget::SetWorld(%08X)\n", this, w);

    world = w;

    if (gadGroup)
        gadGroup->SetWorld(w);
    else if (gadSprite)
    {
        gadSprite->SetWorld(w);

        if (bkgdSprite)
            bkgdSprite->SetWorld(w);
    }
}

//
// SetGUI(g)
//
void SS_Gadget::SetGUI(SS_GUI *g)
{
    DEBUGF(1, "[%08X] SS_Gadget::SetLayer(%08X)\n", this, g);

    gui = g;

    if (gadGroup)
        gadGroup->SetLayer((SS_Layer*)g);
    else if (gadSprite)
        gadSprite->SetLayer((SS_Layer*)g);

    if (g)
        SetWorld(g->World());
    else
        SetWorld(NULL);
}

//
// MakeSprite(file1, file2, file3, file4)
//
SS_Sprite* SS_Gadget::MakeSprite(char *file1, char *file2, char *file3, char *file4)
{
    DEBUGF(1, "SS_Gadget::MakeSprite(\"%s\", \"%s\", \"%s\", \"%s\")\n", file1, file2, file3, file4);

    SS_Sprite   *sprite = new SS_Sprite();

    sprite->AddFrame(file1);

    if (file2 != NULL)
    {
        sprite->AddFrame(file2);

        if (file3 != NULL)
        {
            sprite->AddFrame(file3);

            if (file4 != NULL)
                sprite->AddFrame(file4);
        }
    }

    return sprite;
}

//
// MakeSpriteGroup(file1, [ [ [ file2, ] file3, ] file4])
//
//  Generate sprites from a set of files
//
//      file1 = background sprite image
//      file2 = gadget standard state
//      file3 = gadget clicked state
//      file4 = gadget hover state
//
SS_ItemGroup* SS_Gadget::MakeSpriteGroup(char *file1, char *file2, char *file3, char *file4)
{
    DEBUGF(1, "SS_GUI::MakeSpriteGroup(\"%s\", \"%s\", \"%s\", \"%s\")\n", file1, file2, file3, file4);

    SS_ItemGroup    *group = new SS_ItemGroup();
    SS_Sprite       *sprite;

    sprite = new SS_Sprite();
    sprite->AddFrame(file1);
    sprite->SetHandle(0,0);
    group->AddItem(sprite);

    if (file2 != NULL)
        group->AddItem(SS_Gadget::MakeSprite(file2, file3, file4));

    return group;
}

//
// SetItem(file1, file2, file3, file4)
// Set the sprite from a set of image files
//
void SS_Gadget::SetItem(char *file1, char *file2, char *file3, char *file4)
{
    SetItem(SS_Gadget::MakeSprite(file1, file2, file3, file4));
}

//
// SetItem(sprite)
//
void SS_Gadget::SetItem(SS_LayerItem *item)
{
    DEBUGF(1, "[%08X] SS_Gadget::SetItem(%08X)\n", this, item);

    gadGroup = NULL;
    bkgdSprite = NULL;
    gadSprite = item;

    item->SetHandle(0, 0);
    item->SetLayer(gui);        // Sprite sees layer - but layer doesn't see sprite

    width = item->width;
    height = item->height;

    DisableFlag(GAD_DRAWBACKGROUND|GAD_DRAWBORDER);

    colorSet normColor = { 2,
            { 0xFF, 0xFF, 0xFF, 0xFF },     // border
            { 0xFF, 0xFF, 0xFF, 0xFF },     // fill
            { 0xFF, 0xFF, 0xFF, 0xFF } };   // label

    setNormal   = normColor;
    setHover    = normColor;
    setPressed  = normColor;
}

//
// SetItemGroup(file1, file2, file3, file4)
// Set a sprite group from a set of image files
//
void SS_Gadget::SetItemGroup(char *file1, char *file2, char *file3, char *file4)
{
    SetItemGroup(SS_Gadget::MakeSpriteGroup(file1, file2, file3, file4));
}

//
// SetItemGroup(group)
//
void SS_Gadget::SetItemGroup(SS_ItemGroup *group)
{
    DEBUGF(1, "[%08X] SS_Gadget::SetItemGroup(%08X)\n", this, group);

    gadGroup = group;
    bkgdSprite = group->m_head->m_data;
    gadSprite = group->m_tail->m_data;

    bkgdSprite->SetHandle(0, 0);
    group->SetHandle(0, 0);
    group->SetLayer((SS_Layer*)gui);

    width = group->width;
    height = group->height;

    DisableFlag(GAD_DRAWBACKGROUND);
}

//
// SetLabel(text)
//
void SS_Gadget::SetLabel(char *text)
{
    DEBUGF(1, "[%08X] SS_Gadget::SetLabel(\"%s\")\n", this, text);

    if (!gui)
        throw "The gadget must be added to the gui before using labels.";

    if (!gui->Font())
        throw "The font must be set in the gui before using labels.";

    SetLabel(new SS_String(gui->Font(), text, 0, 0));
}

//
// SetLabel(string)
//
void SS_Gadget::SetLabel(SS_String *str)
{
    DEBUGF(1, "[%08X] SS_Gadget::SetLabel(SS_String)\n", this);

    if (!gui)
        throw "The gadget must be added to the gui before using labels.";

    if (labelString)
        delete labelString;

    labelString = str;

    RealignLabel();
}

//
// ContainsPoint(x, y)
//
// gui-local coordinates
//
bool SS_Gadget::ContainsPoint(float x, float y)
{
    bool in = false;

    if (!hideFlag)
    {
        if (gadSprite && !Flags(GAD_USERECT))
            in = gadSprite->TestPointCollision(x, y, true);
        else
            in = TestRectCollision(x, y);

        if (!in && Flags(GAD_LABELCLICK))
            in |= TestLabelCollision(x - gui->xoffset, y - gui->yoffset);
    }

    return in;
}

//
// MarkEntered()
//  Adds a gadget to the list of those entered
//
void SS_Gadget::MarkEntered()
{
    DEBUGF(1, "[%08X] SS_Gadget::MarkEntered(%08X)\n", this);

    if (!IsEntered() && gui) {
        rolloverNode = gui->rolloverList.Append(this);
    }
}

//
// MarkExited
// Remove from the rolled-over list
//
void SS_Gadget::MarkExited()
{
    DEBUGF(1, "[%08X] SS_Gadget::MarkExited()\n", this);

    if (IsEntered()) {
        gui->rolloverList.Remove(rolloverNode);
        rolloverNode = NULL;
    }
}

//
// SetHandler(type, proc)
//
void SS_Gadget::SetHandler(ssEventCode type, gadgetEventProcPtr proc)
{
    switch (type)
    {
        case SS_EVENT_ENTER:
            mouseEnterProc = proc;
            break;

        case SS_EVENT_MOVE:
            mouseMoveProc = proc;
            break;

        case SS_EVENT_EXIT:
            mouseExitProc = proc;
            break;

        case SS_EVENT_CLICK:
            mouseDownProc = proc;
            break;

        case SS_EVENT_DRAG:
            mouseDragProc = proc;
            break;

        case SS_EVENT_UNCLICK:
            mouseUpProc = proc;
            break;

        case SS_EVENT_KEYDOWN:
            keyDownProc = proc;
            break;

        case SS_EVENT_KEYUP:
            keyUpProc = proc;
            break;

        case SS_EVENT_IDLE:
            idleProc = proc;
            break;

        case SS_EVENT_GAIN_FOCUS:
            gainFocusProc = proc;
            break;

        case SS_EVENT_LOSE_FOCUS:
            loseFocusProc = proc;
            break;

        case SS_EVENT_CHANGE:
            onChangeProc = proc;
            break;
    }
}

//
// HandleEvent(ss_event)
//
bool SS_Gadget::HandleEvent(SS_Event *event)
{
    DEBUGF(1, "[%08X] SS_Gadget::HandleEvent(%d)\n", this, event->type);

    if (hideFlag) return false;

    switch (event->type)
    {
        case SS_EVENT_ENTER:
            if (mouseEnterProc) return (*mouseEnterProc)(this, event);
            break;

        case SS_EVENT_MOVE:
            if (mouseMoveProc) return (*mouseMoveProc)(this, event);
            break;

        case SS_EVENT_EXIT:
            if (mouseExitProc) return (*mouseExitProc)(this, event);
            break;

        case SS_EVENT_CLICK:
            if (mouseDownProc)
            {
                bool did = (*mouseDownProc)(this, event);
                lastClickTime = world->GetWorldTime();
                return did;
            }
            break;

        case SS_EVENT_DRAG:
            if (mouseDragProc) return (*mouseDragProc)(this, event);
            break;

        case SS_EVENT_UNCLICK:
            if (mouseUpProc) return (*mouseUpProc)(this, event);
            break;

        case SS_EVENT_KEYDOWN:
            if (keyDownProc) return (*keyDownProc)(this, event);
            break;

        case SS_EVENT_KEYUP:
            if (keyUpProc) return (*keyUpProc)(this, event);
            break;

        case SS_EVENT_IDLE:
            if (idleProc) return (*idleProc)(this, event);
            break;

        case SS_EVENT_GAIN_FOCUS:
            if (gainFocusProc) return (*gainFocusProc)(this, event);
            break;

        case SS_EVENT_LOSE_FOCUS:
            if (loseFocusProc) return (*loseFocusProc)(this, event);
            break;

        case SS_EVENT_CHANGE:
            if (onChangeProc) return (*onChangeProc)(this, event);
            break;
    }

    return true;
}

//
// CountClicks
// Return the number of close clicks in the same gadget
//
Uint16 SS_Gadget::CountClicks()
{
    DEBUGF(1, "[%08X] SS_Gadget::CountClicks()\n", this);

    Uint32  t = SDL_GetTicks();

    if (t - lastClickTime < 400)
    {
        if (++clickCount > maxClicks)
            ResetClicks();
    }
    else
        ResetClicks();

    lastClickTime = t;

    return clickCount;
}

//
// Move(x, y)
//
void SS_Gadget::Move(float x, float y)
{
    if (gadGroup)
    {
        gadGroup->Move(x, y);
        x -= gadGroup->xhandle;

        if (gadGroup->rotation != 90)
            y -= gadGroup->yhandle;
    }
    else if (gadSprite)
    {
        gadSprite->Move(x, y);
        x -= gadSprite->xhandle;

        if (gadSprite->rotation != 90)
            y -= gadSprite->yhandle;
    }

    xpos = x;
    ypos = y;

    RealignLabel();
}

//
// RealignLabel
//
void SS_Gadget::RealignLabel()
{
    DEBUGF(1, "[%08X] SS_Gadget::RealignLabel()\n", this);

    if (labelString)
    {
        if (gadSprite)
            labelString->AlignToSprite(gadSprite, labelAlignment);
        else
        {
            SS_Rect rect = { xpos - 3.0f, ypos - 3.0f, width + 6.0f, height + 6.0f };
            labelString->AlignToRect(rect, labelAlignment);
        }
    }

}

//
// Animate
//
void SS_Gadget::Animate()
{
    if (hideFlag) return;

    if (gadSprite)
        gadSprite->_Animate();
}

const colorSet* SS_Gadget::ColorSetForState() const
{
    return  !enabled ? &setDisabled : IsPressed() ? &setPressed : IsHover() ? &setHover : &setNormal;
}

//
// Render
//
//  The base render routine draws whatever elements
//  the gadget has in the order:
//
//  1. Render the background, if any
//  2. Render the border, if any
//  3. Set the frame of the Sprite (if any)
//  4. Render the ItemGroup or Sprite (if any)
//  5. Render the label (if any)
//
void SS_Gadget::Render(const SScolorb &inTint)
{
    if (hideFlag) return;

    const colorSet *col = ColorSetForState();

    SScolorb    fillTint;
    MultiplyColorQuads(col->fillColor, inTint, fillTint);

    if (Flags(GAD_DRAWBACKGROUND|GAD_DRAWBORDER))
    {
        gl_do_texture(0);
        gl_do_blend(1);
        glMatrixMode(GL_MODELVIEW);

        if (Flags(GAD_DRAWBACKGROUND)) {
            glColor4ubv((GLubyte*)&fillTint);
            glRectf(xpos, ypos, xpos + width, ypos + height);
        }

        if (Flags(GAD_DRAWBORDER))
        {
            float b = col->borderWeight;
            float x = xpos;     // - b / 2 - 1;
            float y = ypos;     // - b / 2 - 1;
            float w = width;    // + b + 2;
            float h = height;   // + b + 2;

            gl_antialias(false);
            SScolorb    borderTint;
            MultiplyColorQuads(col->borderColor, inTint, borderTint);
            glColor4ubv((GLubyte*)&borderTint);

    #if BORDERS_ARE_LINES

            gl_line_width(1);

            glBegin(GL_LINES);

            for (int q=1; q<=b; q++)
            {
    //          glRectf(x-q, y-q, x+w+q, y+h+q);
                glVertex2f(x-b, y-q+1);             // top-left
                glVertex2f(x+w+b, y-q+1);           // top-right

                glVertex2f(x-b, y+h+q);             // bottom-left
                glVertex2f(x+w+b, y+h+q);           // bottom-right

                glVertex2f(x+w+q, y);               // top-right
                glVertex2f(x+w+q, y+h);             // bottom-right

                glVertex2f(x-q+1, y);               // top-left
                glVertex2f(x-q+1, y+h);             // bottom-left
            }
            glEnd();
    #else

            glBegin(GL_QUAD_STRIP);

                glVertex2f(x-b, y-b);               // top-left-outside
                glVertex2f(x, y);                   // top-left-inside

                glVertex2f(x+w+b, y-b);             // top-right-outside
                glVertex2f(x+w, y);                 // top-right-inside

                glVertex2f(x+w+b, y+h+b);           // bottom-right-outside
                glVertex2f(x+w, y+h);               // bottom-right-inside

                glVertex2f(x-b, y+h+b);             // bottom-left-outside
                glVertex2f(x, y+h);                 // bottom-left-inside

                glVertex2f(x-b, y-b);               // top-left-outside
                glVertex2f(x, y);                   // top-left-inside

            glEnd();
    #endif

        }
    }

    if (gadSprite)
        gadSprite->SetFrameIndex( FrameForState() );

    if (gadGroup) {
        gadGroup->Render(fillTint);
    }
    else if (gadSprite)
        gadSprite->Render(fillTint);

    if (labelString) {
        SScolorb    labelTint;
        MultiplyColorQuads(col->labelColor, inTint, labelTint);
        labelString->Render(labelTint);
    }

}

//
// TestRectCollision(x, y)
//
bool SS_Gadget::TestRectCollision(float x, float y)
{
    if (hideFlag) return false;

    return (x >= xpos
        && x < xpos + width
        && y >= ypos
        && y < ypos + height);
}

//
// TestLabelCollision(x, y)
//
bool SS_Gadget::TestLabelCollision(float x, float y)
{
    if (hideFlag) return false;

    return labelString->IsPointInside(x, y);
}


#pragma mark -
//--------------------------------------------------------------
// SS_CustomGadget
//

SS_CustomGadget::SS_CustomGadget(SS_GUI *g, gadgetRenderProcPtr r) : SS_Gadget(g)
{
    Init();
    SetRenderProc(r);
}

SS_CustomGadget::SS_CustomGadget(SS_GUI *g, float w, float h, gadgetRenderProcPtr r) : SS_Gadget(g, w, h)
{
    Init();
    SetRenderProc(r);
}

SS_CustomGadget::SS_CustomGadget(SS_GUI *g, char *file, gadgetRenderProcPtr r) : SS_Gadget(g)
{
    Init();
    SetItem(file);
    SetRenderProc(r);
}

SS_CustomGadget::SS_CustomGadget(SS_GUI *g, SS_Sprite *sprite, gadgetRenderProcPtr r) : SS_Gadget(g, sprite)
{
    Init();
    SetRenderProc(r);
}

//
// Init
//
void SS_CustomGadget::Init()
{
    rotation    = 0.0f;
    storage     = NULL;
    renderProc  = NULL;

    DisableFlag(GAD_DRAWBACKGROUND);

    for (int i=10;i--;) {
        misc[i] = 0.0f;
        vals[i] = 0;
    }
}

//
// PushAndPrepareMatrix
// Save the current model matrix and transform it into our local coordinate system
//
void SS_CustomGadget::PushAndPrepareMatrix()
{
    gl_do_texture(0);
    gl_do_blend(1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(xpos, ypos, 0.0f);
    glRotatef(rotation, 0.0f, 0.0f, 1.0f);
}

//
// Render
//
void SS_CustomGadget::Render(const SScolorb &inTint)
{
    SS_Gadget::Render(inTint);

    if (renderProc != NULL)
    {
        PushAndPrepareMatrix();
        (renderProc)(this);
        RestoreMatrix();
    }
}


#pragma mark -
//--------------------------------------------------------------
// SS_Button
//

SS_Button::SS_Button(SS_GUI *g) : SS_Gadget(g)
{
    Init();
}

SS_Button::SS_Button(SS_GUI *g, float w, float h, char *lab) : SS_Gadget(g, w, h)
{
    DEBUGF(1, "[%08X] SS_Button(gui, w, h, l) CONSTRUCTOR\n", this);

    Init();

    if (lab != NULL)
        SetLabel(lab);
}

SS_Button::SS_Button(SS_GUI *g, SS_Sprite *sprite) : SS_Gadget(g, sprite)
{
    DEBUGF(1, "[%08X] SS_Button(gui, sprite) CONSTRUCTOR\n", this);

    Init();
}

SS_Button::SS_Button(SS_GUI *g, char *file1, char *file2, char *file3) : SS_Gadget(g)
{
    DEBUGF(1, "[%08X] SS_Button(gui, \"%s\", \"%s\", \"%s\") CONSTRUCTOR\n", this, file1, file2, file3);

    Init();
    SetItem(file1, file2, file3);
}

//
// Destructor
//
SS_Button::~SS_Button()
{
    DEBUGF(1, "[%08X] SS_Button DESTRUCTOR\n", this);
}

//
// Init
//
void SS_Button::Init()
{
    DEBUGF(1, "[%08X] SS_Button::Init(gui)\n", this);

    SetFlags(GAD_AUTOROLLOVER|GAD_AUTOPRESS|GAD_DRAWBACKGROUND|GAD_DRAWBORDER);
    SetLabelAlignment(SA_CENTER);

//  colorSet normColor = { 2,
//          { 0x33, 0x66, 0xCC, 0xFF },     // border
//          { 0xAF, 0xAF, 0xAF, 0xFF },     // fill
//          { 0x00, 0x00, 0x00, 0xFF } };   // label
//
//  colorSet hoverColor = { 2,
//          { 0xCC, 0xCC, 0xFF, 0xFF },     // border
//          { 0x3F, 0x3F, 0xCF, 0xFF },     // fill
//          { 0xFF, 0xFF, 0xFF, 0xFF } };   // label
//
//  colorSet pressColor = { 2,
//          { 0xFF, 0xFF, 0xFF, 0xFF },     // border
//          { 0x3F, 0xFF, 0x3F, 0xFF },     // fill
//          { 0xFF, 0xFF, 0xFF, 0xFF } };   // label

    colorSet normColor = { 2,
            { 0xFF, 0xFF, 0xFF, 0xFF },     // border
            { 0x00, 0x00, 0x00, 0x80 },     // fill
            { 0xFF, 0xFF, 0xFF, 0xFF } };   // label

    colorSet hoverColor = { 2,
            { 0xFF, 0xFF, 0x00, 0xFF },     // border
            { 0x00, 0x00, 0x40, 0x80 },     // fill
            { 0xFF, 0xFF, 0x00, 0xFF } };   // label

    colorSet pressColor = { 2,
            { 0x00, 0xFF, 0x00, 0xFF },     // border
            { 0x00, 0x40, 0x40, 0xFF },     // fill
            { 0xFF, 0xFF, 0x00, 0xFF } };   // label

    SetButtonColors(normColor, hoverColor, pressColor, normColor);
    SetDisabledAlpha(0x7F);
}

//
// HandleEvent
//
bool SS_Button::HandleEvent(SS_Event *event)
{
    if (event->type == SS_EVENT_UNCLICK)
    {
        bool did = false;

        if (Command != 0x00000000)
        {
            SendCommand(Command);
            did = true;
        }

        if (MessageIsSet())
        {
            Broadcast();
            did = true;
        }

        if (did) return true;
    }

    return SS_Gadget::HandleEvent(event);
}

#pragma mark -
//--------------------------------------------------------------
// SS_Checkbox
//

SS_Checkbox::SS_Checkbox(SS_GUI *g, char *label) : SS_Gadget(g, SS_Checkbox::checkboxSprite)
{
    DEBUGF(1, "[%08X] SS_Checkbox(%08X, %s) CONSTRUCTOR\n", this, g, label);

    Init();

    if (label != NULL)
        SetLabel(label);
}

SS_Checkbox::SS_Checkbox(SS_GUI *g, float w, float h, char *label) : SS_Gadget(g, w, h)
{
    DEBUGF(1, "[%08X] SS_Checkbox(%08X, %.2f, %.2f, %s) CONSTRUCTOR\n", this, g, w, h, label);

    Init();

    if (label != NULL)
        SetLabel(label);

    EnableFlag(GAD_DRAWBORDER|GAD_DRAWBACKGROUND);
}

SS_Checkbox::~SS_Checkbox()
{
}

//
// Init
//
void SS_Checkbox::Init()
{
    DEBUGF(1, "[%08X] SS_Checkbox::Init()\n", this);

    SetFlags(GAD_AUTOROLLOVER|GAD_AUTOPRESS|GAD_LABELCLICK);
}

//
// FrameForState
//
Uint16 SS_Checkbox::FrameForState()
{
    if (IsPressed())
        return State() ? CHECKBOX_OFF_OVER : CHECKBOX_ON_OVER;

    return State() ? (IsHover() ? CHECKBOX_ON_OVER : CHECKBOX_ON) : (IsHover() ? CHECKBOX_OFF_OVER : CHECKBOX_OFF);
}

//
// HandleEvent
//
bool SS_Checkbox::HandleEvent(SS_Event *event)
{
    if (event->type == SS_EVENT_UNCLICK)
    {
        SetState( ~State() );

        if (MessageIsSet())
        {
            message.SetValue(State());
            Broadcast();
        }
    }

    return SS_Gadget::HandleEvent(event);
}

void SS_Checkbox::Render(const SScolorb &inTint)
{
    SS_Gadget::Render(inTint);

    Uint16 ffs = FrameForState();

    if (!gadSprite && !gadGroup && (ffs == CHECKBOX_ON_OVER || ffs == CHECKBOX_ON))
    {
        float xp = xpos + width / 2.0f, yp = ypos + height / 2.0f;
        float mu = MIN(width, height) / 18.0f;
        SScolorb borderTint;
        const colorSet *col = ColorSetForState();
        MultiplyColorQuads(col->borderColor, inTint, borderTint);

        gl_do_texture(0);
        gl_do_blend(1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(xp, yp, 0.0f);
        glRotatef(45.0f, 0.0f, 0.0f, 1.0f);

        glColor4ubv((GLubyte*)&borderTint);

        GLfloat vert[][2] = {
            { 0.0f, 0.0f },             //          0
            { +mu * 1.0f, -mu * 8.0f }, // 0 NNE
            { -mu * 1.0f, -mu * 8.0f }, // 1 NNW    2
            { -mu * 1.0f, -mu * 1.0f }, // 2 NW
            { -mu * 8.0f, -mu * 1.0f }, // 3 WNW    4
            { -mu * 8.0f, +mu * 1.0f }, // 4 WSW
            { -mu * 1.0f, +mu * 1.0f }, // 5 SW     6
            { -mu * 1.0f, +mu * 8.0f }, // 6 SSW
            { +mu * 1.0f, +mu * 8.0f }, // 7 SSE    8
            { +mu * 1.0f, +mu * 1.0f }, // 8 SE
            { +mu * 8.0f, +mu * 1.0f }, // 9 ESE    10
            { +mu * 8.0f, -mu * 1.0f }, // 10 ENE
            { +mu * 1.0f, -mu * 1.0f }, // 11 NE    12
            { +mu * 1.0f, -mu * 8.0f }  // 12 NNE
        };

        xp = yp = 0.0;
        glBegin(GL_TRIANGLE_FAN);
        for (int i=0; i<NUM_ELEMENTS(vert); i++)
            glVertex2fv( &vert[i][0] );
        glEnd();

        glPopMatrix();
    }
}

#pragma mark -
//--------------------------------------------------------------
// SS_RadioButton
//

SS_RadioButton::SS_RadioButton(SS_GUI *g, Uint16 val, char *label) : SS_Gadget(g, SS_RadioButton::radioButtonSprite)
{
    DEBUGF(1, "[%08X] SS_RadioButton(%08X, %d, %s) CONSTRUCTOR\n", this, gui, val, label);

    Init();

    if (label != NULL)
        SetLabel(label);
}

//
// Destructor
//
SS_RadioButton::~SS_RadioButton()
{
}

//
// Init
//
void SS_RadioButton::Init()
{
    DEBUGF(1, "[%08X] SS_RadioButton::Init()\n", this);

    SetFlags(GAD_AUTOROLLOVER|GAD_AUTOPRESS|GAD_LABELCLICK);
}

//
// FrameForState
//
Uint16 SS_RadioButton::FrameForState()
{
    if (IsPressed())
        return State() ? RADIO_ON_OVER : RADIO_OFF_OVER;

    return State() ? (IsHover() ? RADIO_ON_OVER : RADIO_ON) : (IsHover() ? RADIO_OFF_OVER : RADIO_OFF);

}

//
// HandleEvent
//
bool SS_RadioButton::HandleEvent(SS_Event *event)
{
    if (event->type == SS_EVENT_UNCLICK)
        SetState( true );

    return SS_Gadget::HandleEvent(event);
}


#pragma mark -
//--------------------------------------------------------------
// SS_Thumb
//

SS_Thumb::SS_Thumb()
{
    Init(GAD_DRAGGABLE|GAD_DRAWBORDER);
}

SS_Thumb::SS_Thumb(SS_GUI *g, float w, float h) : SS_Gadget(g, w, h, GAD_DRAGGABLE|GAD_DRAWBORDER|GAD_AUTOROLLOVER|GAD_AUTOPRESS)
{
    colorSet normColor = { 2,
            { 0x33, 0x66, 0xCC, 0xFF },     // border
            { 0xAF, 0xAF, 0xAF, 0xFF },     // fill
            { 0x00, 0x00, 0x00, 0xFF } };   // label

    SetNormalColor(normColor);

    colorSet hoverColor = { 2,
            { 0xCC, 0xCC, 0xFF, 0xFF },     // border
            { 0x3F, 0x3F, 0xCF, 0xFF },     // fill
            { 0xFF, 0xFF, 0xFF, 0xFF } };   // label

    SetHoverColor(hoverColor);

    colorSet pressColor = { 2,
            { 0xFF, 0xFF, 0xFF, 0xFF },     // border
            { 0x3F, 0xFF, 0x3F, 0xFF },     // fill
            { 0xFF, 0xFF, 0xFF, 0xFF } };   // label

    SetPressedColor(pressColor);
    SetDisabledColor(normColor);
    SetDisabledAlpha(0x7F);

    xoffs   = 0;
    yoffs   = 0;
    xthumb  = 0;
    ythumb  = 0;
    twidth  = width;
    theight = height;
    minxv   = 0.0f;
    minyv   = 0.0f;
    maxxv   = 0.0f;
    maxyv   = 0.0f;
    trepx   = 0.0f;
    trepy   = 0.0f;
    xvalue  = 0.0f;
    yvalue  = 0.0f;
}

SS_Thumb::SS_Thumb(SS_GUI *g, SS_Sprite *sprite) : SS_Gadget(g, sprite)
{
    Uint32  f = GAD_DRAGGABLE;
    if (sprite->FrameCount() > 1) f |= GAD_AUTOPRESS;
    if (sprite->FrameCount() > 2) f |= GAD_AUTOROLLOVER;
    Init(f);
}

SS_Thumb::SS_Thumb(SS_GUI *g, char *file) : SS_Gadget(g)
{
    Init(GAD_DRAGGABLE);
    SetItem(file);
}

SS_Thumb::SS_Thumb(SS_GUI *g, char *file1, char *file2) : SS_Gadget(g)
{
    Init(GAD_DRAGGABLE|GAD_AUTOPRESS);
    SetItem(file1, file2);
}

SS_Thumb::SS_Thumb(SS_GUI *g, char *file1, char *file2, char *file3) : SS_Gadget(g)
{
    Init(GAD_DRAGGABLE|GAD_AUTOPRESS|GAD_AUTOROLLOVER);
    SetItem(file1, file2, file3);
}

SS_Thumb::~SS_Thumb()
{
}

//
// Init(flags)
//
void SS_Thumb::Init(Uint32 f)
{
    SetFlags(f);
}

//
// HandleEvent
//
bool SS_Thumb::HandleEvent(SS_Event *event)
{
    float x = event->x, y = event->y;

    switch(event->type)
    {
        case SS_EVENT_CLICK:
        {
            if (x >= xthumb && x < xthumb + twidth && y >= ythumb && y < ythumb + theight)
            {
                xoffs = x - xthumb;
                yoffs = y - ythumb;
            }
            else
            {
                xoffs = twidth / 2;
                yoffs = theight / 2;
            }
        }

        case SS_EVENT_DRAG:
        {
            float nx = x - xoffs, ny = y - yoffs;
            if (nx < 0) nx = 0;
            if (nx > width - twidth) nx = width - twidth;
            if (ny < 0) ny = 0;
            if (ny > height - theight) ny = height - theight;
            SetThumbPos(nx, ny);
            break;
        }
    }

    bool did = SS_Gadget::HandleEvent(event);   // to allow rollover behavior?

    if (event->type == SS_EVENT_MOVE)   // always intercept rollover events
        did = true;

    return did;
}

//
// Render
//
//  The base render routine draws whatever elements
//  the gadget has in the order:
//
//  1. Render the background, if any
//  2. Set the frame of the Sprite (if any)
//  3. Render the ItemGroup or Sprite (if any)
//  4. Render the label (if any)
//  5. Render the border
//
void SS_Thumb::Render(const SScolorb &inTint)
{
    if (hideFlag) return;

    colorSet setBack = { 1,
            { 0xFF, 0xFF, 0xFF, 0xFF },     // border
            { 0x00, 0x00, 0x00, 0xFF },     // fill
            { 0xFF, 0xFF, 0xFF, 0xFF } };   // label

    RenderBox(xpos, ypos, width, height, inTint, &setBack);

    const colorSet *col = ColorSetForState();
    RenderBox(xpos + xthumb, ypos + ythumb, twidth, theight, inTint, col);
}



#pragma mark -
//--------------------------------------------------------------
// SS_Slider
//

SS_Slider::SS_Slider(SS_GUI *g, float x1, float x2, float y, float v1, float v2, bool isVert) : SS_Gadget(g, SS_Slider::sliderSprite)
{
    DEBUGF(1, "[%08X] SS_Slider(g, x1, x2, y, v1, v2, isVert) CONSTRUCTOR\n", this);

    //
    // Make default graphics with vectors
    //
    if (SS_Slider::sliderSprite == NULL)
    {
        SS_ItemGroup *vgroup = new SS_ItemGroup();
        SS_VectorSprite *vsprite = new SS_VectorSprite();
        SS_VectorFrame *vframe = new SS_VectorFrame();
        vframe->AppendLine(x1, y, x2, y);
        vsprite->AddFrame(vframe);
        vgroup->AddItem(vsprite);
        vsprite = new SS_VectorSprite();
        vframe = new SS_VectorFrame();
        vframe->SetLineTint(0xA0, 0xA0, 0xFF, 0xFF);
        vframe->SetFillTint(0x60, 0x60, 0xFF, 0xFF);
        vframe->AppendDisc(0.0f, 0.0f, 6.0f, 20);
        vsprite->AddFrame(vframe);
        vgroup->AddItem(vsprite);
        SetItemGroup(vgroup);
    }

    Init(x1, x2, y, v1, v2, isVert);
}

SS_Slider::~SS_Slider()
{
}

//
// Init(x1, x2, y, v1, v2, orient)
//
void SS_Slider::Init(float x1, float x2, float y, float v1, float v2, bool isVert)
{
    DEBUGF(1, "[%08X] SS_Slider::Init(x1, x2, y, v1, v2, orient)\n", this);

    SetFlags(GAD_AUTOROLLOVER|GAD_AUTOPRESS|GAD_DRAGGABLE);

    isVertical = isVert;
    SetXRange(x1, x2);
    SetAxis(y);
    SetVRange(v1, v2);

    SetValue( (v1 > v2) ? (v2 + (v1 - v2) / 2) : (v1 + (v2 - v1) / 2) );

    if (isVert) {
        gadGroup->SetRotation(90);
        gadGroup->SetHandle(0, gadGroup->height);
        SetRegion(height, width);   // rotate the rect
    }
}

//
// SetValue
//
void SS_Slider::SetValue(float v)
{
    if (minVal > maxVal)
        value = (v < maxVal) ? maxVal : ((v > minVal) ? minVal : v);
    else
        value = (v < minVal) ? minVal : ((v > maxVal) ? maxVal : v);

    UpdateThumb();
    Broadcast(v);
}

//
// SetPosition
//
void SS_Slider::SetPosition(float x)
{
    float v;

    if (minVal > maxVal)
        v =  maxVal + ( ((x < minX) ? minX : ((x > maxX) ? maxX : x)) - minX ) * (minVal - maxVal) / (maxX - minX);
    else
        v =  minVal + ( ((x < minX) ? minX : ((x > maxX) ? maxX : x)) - minX ) * (maxVal - minVal) / (maxX - minX);

    SetValue(v);
}

//
// UpdateThumb
//
void SS_Slider::UpdateThumb()
{
//  if (isVertical)
//      gadSprite->Move(thumbY, minX + (value - minVal) * ((maxX - minX) / (maxVal - minVal)));
//  else
    if (minVal > maxVal)
        gadSprite->Move(minX + (value - maxVal) * ((maxX - minX) / (minVal - maxVal)), thumbY);
    else
        gadSprite->Move(minX + (value - minVal) * ((maxX - minX) / (maxVal - minVal)), thumbY);
}

//
// HandleEvent
//
bool SS_Slider::HandleEvent(SS_Event *event)
{
    if (event->type == SS_EVENT_CLICK || event->type == SS_EVENT_DRAG)
        SetPosition(isVertical ? event->y : event->x);

    return SS_Gadget::HandleEvent(event);
}


#pragma mark -
//--------------------------------------------------------------
// SS_Scrollbar
//

SS_Scrollbar::SS_Scrollbar(SS_GUI *g, float startVal, float endVal, float a) : SS_Gadget(g)
{
    DEBUGF(1, "[%08X] SS_Scrollbar(g, topVal, bottomVal, isVert) CONSTRUCTOR\n", this);

//  if (SS_Scrollbar::defaultScrollbar != NULL)
//

    Init(startVal, endVal, a);
}

SS_Scrollbar::~SS_Scrollbar()
{
}

//
// Init(v1, v2, orient)
//
void SS_Scrollbar::Init(float v1, float v2, float a)
{
    DEBUGF(1, "[%08X] SS_Scrollbar::Init(v1, v2, a)\n", this);

    SetFlags(GAD_AUTOROLLOVER|GAD_AUTOPRESS|GAD_DRAGGABLE);

    SetRange(v1, v2);
    SetValue(v1 + (v2 - v1) / 2);
    SetAmount(a);
}

//
// LoadStructure
// Loads several graphics files into a nice little sprite group
// All handles are set to 0,0 which helps with positioning them
// relative to the gadget position (which also has a handle of 0,0)
//
void SS_Scrollbar::LoadStructure(
    char *topGroove,    char *bottomGroove,     char *stretchGroove,
    char *upNormal,     char *downNormal,
    char *upPressed,    char *downPressed,
    char *upHover,      char *downHover )
{
    SS_ItemGroup  *stGroup;
    SS_Sprite       *stSprite;

    stGroup = new SS_ItemGroup();

    // Top Groove Part
    stSprite = new SS_Sprite();
    stSprite->AddFrame(new SS_Frame(topGroove));
    stGroup->AddItem(stSprite);
    stSprite->SetHandle(0, 0);

    topGrooveHeight = stSprite->height;
    ExpandWidth(stSprite->width);

    // Stretchy Groove Part
    stSprite = new SS_Sprite();
    stSprite->AddFrame(new SS_Frame(stretchGroove));
    stGroup->AddItem(stSprite);
    stSprite->SetHandle(0, 0);

    stretchGrooveHeight = stSprite->height;
    ExpandWidth(stSprite->width);

    // Bottom Groove Part
    stSprite = new SS_Sprite();
    stSprite->AddFrame(new SS_Frame(bottomGroove));
    stGroup->AddItem(stSprite);
    stSprite->SetHandle(0, 0);

    bottomGrooveHeight = stSprite->height;
    ExpandWidth(stSprite->width);

    // Up Arrow
    stSprite = new SS_Sprite();
    stSprite->AddFrame(new SS_Frame(upNormal));         // Normal required

    upArrowHeight = stSprite->height;
    ExpandWidth(stSprite->width);

    if (upPressed)
        stSprite->AddFrame(new SS_Frame(upPressed));    // Pressed optional

    if (upHover)
        stSprite->AddFrame(new SS_Frame(upHover));      // Hover optional

    stGroup->AddItem(stSprite);
    stSprite->SetHandle(0, 0);

    // Down Arrow
    stSprite = new SS_Sprite();
    stSprite->AddFrame(new SS_Frame(downNormal));

    downArrowHeight = stSprite->height;
    ExpandWidth(stSprite->width);

    if (downPressed)
        stSprite->AddFrame(new SS_Frame(downPressed));

    if (downHover)
        stSprite->AddFrame(new SS_Frame(downHover));

    stGroup->AddItem(stSprite);
    stSprite->SetHandle(0, 0);

    structureGroup = stGroup;

    if (gadGroup == NULL)
        gadGroup = new SS_ItemGroup();

    gadGroup->AddItem(stGroup);

    if (thumbGroup)
        gadGroup->AddItem(thumbGroup);

    gadGroup = stGroup;
    if (thumbGroup)

    Update();
}

//
// LoadThumb
//
// This takes a lot of arguments, sorry
//
void SS_Scrollbar::LoadThumb(
    char *topNormal,    char *bottomNormal,     char *stretchNormal,
    char *topPressed,   char *bottomPressed,    char *stretchPressed,
    char *topHover,     char *bottomHover,      char *stretchHover,
    char *gripNormal,   char *gripPressed,      char *gripHover )
{
    SS_ItemGroup  *thGroup = new SS_ItemGroup();
    SS_Sprite       *thSprite;

                        // Top Part
                        thSprite = new SS_Sprite();
                        thSprite->AddFrame(new SS_Frame(topNormal));
    if (topPressed)     thSprite->AddFrame(new SS_Frame(topPressed));
    if (topHover)       thSprite->AddFrame(new SS_Frame(topHover));
                        thGroup->AddItem(thSprite);

                        // Stretchy Part
                        thSprite = new SS_Sprite();
                        thSprite->AddFrame(new SS_Frame(stretchNormal));
    if (stretchPressed) thSprite->AddFrame(new SS_Frame(stretchPressed));
    if (stretchHover)   thSprite->AddFrame(new SS_Frame(stretchHover));
                        thGroup->AddItem(thSprite);

                        // Bottom Part
                        thSprite = new SS_Sprite();
                        thSprite->AddFrame(new SS_Frame(bottomNormal));
    if (bottomPressed)  thSprite->AddFrame(new SS_Frame(bottomPressed));
    if (bottomHover)    thSprite->AddFrame(new SS_Frame(bottomHover));
                        thGroup->AddItem(thSprite);

                        // Grip Part
    if (gripNormal) {
                            thSprite = new SS_Sprite();
                            thSprite->AddFrame(new SS_Frame(gripNormal));
        if (gripPressed)    thSprite->AddFrame(new SS_Frame(gripPressed));
        if (gripHover)      thSprite->AddFrame(new SS_Frame(gripHover));
                            thGroup->AddItem(thSprite);
    }

    thumbGroup = thGroup;
    UpdateThumb();
}

//
// SetValue
//
void SS_Scrollbar::SetValue(float v)
{
    if (minVal < maxVal)
        value = (v < minVal) ? minVal : ((v > maxVal) ? maxVal : v);
    else
        value = (v > minVal) ? minVal : ((v < maxVal) ? maxVal : v);

    UpdateThumb();
}

//
// SetPosition
// Set the value according to a mouse drag, for example
// Constrains to the structure rect
//
void SS_Scrollbar::SetPosition(float y)
{
}

//
// SetRange, with error correction
//
void SS_Scrollbar::SetRange(float v1, float v2, float a)
{
    minVal = v1;
    maxVal = v2;

    if (a!=-1) { amount=a; }

    if (amount > ABS(v2 - v1))
        amount = ABS(v2 - v1);

    if (v1 < v2) {
        if (value < v1) value = v1;
        if (value > v2) value = v2;
    } else {
        if (value > v1) value = v1;
        if (value < v2) value = v2;
    }

    UpdateThumb();
}

//
// UpdateThumb
//
void SS_Scrollbar::UpdateThumb()
{
}

//
// HandleEvent
//
bool SS_Scrollbar::HandleEvent(SS_Event *event)
{
//  if (event->type == SS_EVENT_CLICK || event->type == SS_EVENT_DRAG)
//      SetPosition(isVertical ? event->y : event->x);

    return SS_Gadget::HandleEvent(event);
}


#pragma mark -
//--------------------------------------------------------------
// SS_Dragger
//

SS_Dragger::SS_Dragger()
{
    Init(GAD_DRAGGABLE);
}

SS_Dragger::SS_Dragger(SS_GUI *g, float w, float h, Uint32 f) : SS_Gadget(g, w, h, f)
{
    colorSet normColor = { 2,
            { 0xFF, 0xFF, 0xFF, 0xFF },     // border
            { 0x10, 0x20, 0x50, 0xFF },     // fill
            { 0xFF, 0xFF, 0xFF, 0xFF } };   // label

    SetNormalColor(normColor);
}

SS_Dragger::SS_Dragger(SS_GUI *g, SS_Sprite *sprite) : SS_Gadget(g, sprite)
{
    Uint32  f = GAD_DRAGGABLE;

    if (sprite->FrameCount() > 1) f |= GAD_AUTOPRESS;
    if (sprite->FrameCount() > 2) f |= GAD_AUTOROLLOVER;

    Init(f);
}

SS_Dragger::SS_Dragger(SS_GUI *g, char *file) : SS_Gadget(g)
{
    Init(GAD_DRAGGABLE);
    SetItem(file);
}

SS_Dragger::SS_Dragger(SS_GUI *g, char *file1, char *file2) : SS_Gadget(g)
{
    Init(GAD_DRAGGABLE|GAD_AUTOPRESS);
    SetItem(file1, file2);
}

SS_Dragger::SS_Dragger(SS_GUI *g, char *file1, char *file2, char *file3) : SS_Gadget(g)
{
    Init(GAD_DRAGGABLE|GAD_AUTOPRESS|GAD_AUTOROLLOVER);
    SetItem(file1, file2, file3);
}

SS_Dragger::~SS_Dragger()
{
}

//
// Init(flags)
//
void SS_Dragger::Init(Uint32 f)
{
    SetFlags(f);
}

//
// HandleEvent
//
bool SS_Dragger::HandleEvent(SS_Event *event)
{
    if (event->type == SS_EVENT_DRAG) {
        gui->SetOffset(event->xglob - xdiff, event->yglob - ydiff);
    }
    else if (event->type == SS_EVENT_CLICK) {
        world->LayerToFront(gui);
        xdiff = event->xglob - gui->xoffset;
        ydiff = event->yglob - gui->yoffset;
    }

    bool did = SS_Gadget::HandleEvent(event);

    if (event->type == SS_EVENT_MOVE)   // always intercept rollover events
        did = true;

    return did;
}


#pragma mark -
//--------------------------------------------------------------
// SS_TextInput
//

SS_TextInput::SS_TextInput(SS_GUI *g, SS_Sprite *spr) : SS_Gadget(g, spr)
{
    DEBUGF(1, "[%08X] SS_TextInput(gui, sprite)\n", this);

    Init();
}

SS_TextInput::SS_TextInput(SS_GUI *g, float w, float h) : SS_Gadget(g, w, h)
{
    DEBUGF(1, "[%08X] SS_TextInput(gui, w, h)\n", this);

    Init();
    SetMaxStringWidth((int)w + 100);            // pixel width input limit
}

SS_TextInput::~SS_TextInput()
{
    if (editString != NULL)
        delete editString;
}

//
// Init
//
void SS_TextInput::Init()
{
    DEBUGF(1, "[%08X] SS_TextInput::Init()\n", this);

    SetFlags(GAD_DRAWBORDER|GAD_KEYFOCUS);
    editString = new SS_EditString(gui->Font());
    SetMaxClicks(3);
    SetStringOffset(3, 3);
    Move(0, 0);
}

//
// Reset
//
void SS_TextInput::Reset()
{
    LoseFocus();
    SS_Gadget::Reset();
}

//
// Move(x, y)
//
void SS_TextInput::Move(float x, float y)
{
    SS_Gadget::Move(x, y);
    editString->Move(x + xoff, y + yoff + editString->Ascent());
}

//
// Render
//
void SS_TextInput::Render(const SScolorb &inTint)
{
    SS_Gadget::Render(inTint);

    SDL_Rect bounds = { (SInt16)xpos, (SInt16)ypos, (UInt16)width, (UInt16)height };
    editString->Render(inTint, &bounds);
}

//
// HandleEvent
//
bool SS_TextInput::HandleEvent(SS_Event *event)
{
    Uint16      i, s;
    float       x, y;

    switch(event->type)
    {
        case SS_EVENT_ENTER:
        case SS_EVENT_MOVE:
            return true;

        case SS_EVENT_CLICK:
            x = event->x - xoff;
            y = event->y - yoff;
            if (x > 0 && y > 0)
            {
                i = editString->IndexOfPoint(x, y);

                switch (CountClicks())
                {
                    case 3:
                        SelectAll();
                        selectMode = SEL_ALL;
                        break;
                    case 2:
                        SelectWord(i);
                        selectMode = SEL_WORD;
                        break;
                    default:
                        SetSelection(i);
                        selectMode = SEL_CHAR;
                        break;
                }
            }
            break;

        case SS_EVENT_DRAG:
            i = editString->IndexOfPoint(event->x - xoff, event->y - yoff);

            switch (selectMode)
            {
                case SEL_CHAR:
                    s = SelectionStart();
                    SetSelection(s, i - s);
                    break;

                case SEL_WORD:
                    SelectWord(i, true);
                    break;

                case SEL_ALL:
                    break;
            }
            break;

        case SS_EVENT_KEYDOWN:
            switch(event->key)
            {
                case SDLK_ESCAPE:
                case SDLK_RETURN:
                    return false;

                default:
                    editString->HandleKey(event->key, event->mod);
                    return true;
            }
            break;

        case SS_EVENT_KEYUP:
            break;

        case SS_EVENT_GAIN_FOCUS:
            GainFocus();
            SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
            break;

        case SS_EVENT_LOSE_FOCUS:
            LoseFocus();
            SDL_EnableKeyRepeat(0, 0);
            break;

        default:
            break;
    }

    return SS_Gadget::HandleEvent(event);
}


#pragma mark -
//--------------------------------------------------------------
// SS_StaticItem
//
// A simple GUI item consisting of a sprite and a text label
// which are completely non-interactive.
//

SS_StaticItem::SS_StaticItem(SS_GUI *g, char *text, SS_LayerItem *spr) : SS_Gadget(g, spr)
{
    colorSet normColor = { 2,
            { 0xFF, 0xFF, 0xFF, 0xFF },     // border
            { 0x4F, 0x4F, 0x4F, 0xFF },     // fill
            { 0xFF, 0xFF, 0xFF, 0xFF } };   // label

    SetNormalColor(normColor);

    if (text) SetLabel(text);
}

SS_StaticItem::SS_StaticItem(SS_GUI *g, float w, float h, Uint32 f) : SS_Gadget(g, w, h, f)
{
    colorSet normColor = { 2,
            { 0xFF, 0xFF, 0xFF, 0xFF },     // border
            { 0x4F, 0x4F, 0x4F, 0xFF },     // fill
            { 0xFF, 0xFF, 0xFF, 0xFF } };   // label

    SetNormalColor(normColor);
}

SS_StaticItem::~SS_StaticItem()
{
}

