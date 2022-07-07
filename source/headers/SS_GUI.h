/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_GUI.h
 *
 *  $Id: SS_GUI.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_GUI_H__
#define __SS_GUI_H__

#include "SDL.h"
#include "SS_Types.h"
#include "SS_Layer.h"
#include "SS_LayerItem.h"
#include "SS_SFont.h"


//
// Gadget Event Handler prototype
//
typedef bool (*gadgetEventProcPtr)(SS_Gadget*, SS_Event*);
typedef void (*gadgetRenderProcPtr)(SS_CustomGadget*);


#pragma mark -
//--------------------------------------------------------------
// SS_GUI
//
class SS_GUI : public SS_Layer
{
    private:
        SS_SFont            *guiFont;
        static SS_GUI       *activeGUI;
        bool                dormant;

    public:

        SS_GadgetList       gadgetList;
        SS_GadgetList       rolloverList;

        SS_Gadget           *clickedGadget;                     // the clicked gadget
        SS_Gadget           *keyFocusGadget;                    // the gadget receiving keyboard events

                            SS_GUI();
                            SS_GUI(SS_SFont *font);
        virtual             ~SS_GUI();

        virtual inline layerType Type() { return SS_LAYER_GUI; }

        void                SetEnabled(bool e);
        void                Activate();
        static void         ActivateNone();
        inline void         SetDormant(bool b) { dormant = b; }

        virtual void        SetWorld(SS_World *w);
        inline SS_World*    World() { return world; }

        // Set all the default gadgets
        static void         SetGadgetSuite(SS_LayerItem *check, SS_ItemGroup *slider=NULL, SS_Scrollbar *scroller=NULL);

        inline void         SetFont(char *filename, float desc) { guiFont = new SS_SFont(filename, desc); }
        inline void         SetFont(SS_SFont *font) { guiFont = font; }
        inline SS_SFont*    Font() { return guiFont; }

        void                AddGadget(SS_Gadget *gadget);

        SS_Gadget*          GadgetAtPointer();
        SS_Gadget*          GadgetAtXY(float x, float y);
        inline SS_Gadget*   KeyFocus() { return keyFocusGadget; }
        void                SetKeyFocus(SS_Gadget *g);

        virtual bool        HandleEvent(SDL_Event *e);
        virtual void        HandleCommand(long command) { }
        void                ExitAllRollovers();

        virtual void        Process() {}
        void                Animate();
        void                Render();

    private:
        void                Init();
};

#pragma mark -
//--------------------------------------------------------------
// SS_Gadget
//
// TODO:
//  - Add support for color highlighting - click & rollover
//  - Add support for rectangle highlighting - focused gadget
//

enum {
    GAD_NOFLAGS         = 0,
    GAD_AUTOROLLOVER    = (1L << 0),        // Use built-in rollover behavior
    GAD_AUTOPRESS       = (1L << 1),        // Use built-in mouseclick behavior
    GAD_DRAWBACKGROUND  = (1L << 2),        // Draw the background if nothing else exists
    GAD_DRAWBORDER      = (1L << 3),        // Gadget has a visible border
    GAD_ACTIVATECLICK   = (1L << 4),        // Gadget takes activation clicks
    GAD_LABELCLICK      = (1L << 5),        // Gadget takes label clicks
    GAD_USERECT         = (1L << 6),        // Gadget uses its Rect for collision testing
    GAD_DRAGGABLE       = (1L << 7),        // Allow drag events in the gadget
    GAD_KEYFOCUS        = (1L << 8),        // Gadget takes keyboard events
    GAD_LIVEUPDATE      = (1L << 9),        // Gadget calls onChange during drags
    GAD_STRETCHY        = (1L << 10)        // Gadget has stretchiness
};

class SS_Gadget : public SS_Broadcaster, public SS_Listener
{
    friend class SS_GUI;

    protected:
        bool                    enabled;            // is the gadget enabled?
        bool                    hideFlag;           // is the gadget hidden?
        colorSet                setNormal;          // Color when no rollover or press
        colorSet                setHover;           // Color when hovering/highlighted
        colorSet                setPressed;         // Color when pressed
        colorSet                setDisabled;        // Color when disabled

    private:
        SS_SFont                *gadFont;           // the font associated with this gadget
        stringAlign             labelAlignment;     // label alignment type (top, right, bottom, left)

        Uint32                  state;              // whatever int you wint
        Uint32                  flags;              // attributes and standard behaviors

        bool                    pointerIn;          // is the pointer inside?

        SS_GadgetNode           *rolloverNode;      // node in the rollover list
        SS_GadgetNode           *guiNode;           // node in the gui list

        Uint32                  lastClickTime;      // allow gadgets to track multi-clicks
        Uint16                  clickCount;         // count up multi-clicks
        Uint16                  maxClicks;          // 2:double-clickable  3:triple-clickable  etc
        float                   clickx, clicky;     // local point where the click began

        // Low-level events
        gadgetEventProcPtr      mouseEnterProc, mouseMoveProc, mouseExitProc;
        gadgetEventProcPtr      mouseDownProc, mouseDragProc, mouseUpProc;
        gadgetEventProcPtr      keyDownProc, keyUpProc;
        gadgetEventProcPtr      idleProc;

        // High-level events
        gadgetEventProcPtr      gainFocusProc, loseFocusProc;
        gadgetEventProcPtr      onChangeProc;

    protected:
        SS_World                *world;
        SS_GUI                  *gui;               // the gui is a layer
        SS_ItemGroup            *gadGroup;          // the spritegroup associated with this gadget (if any)
        SS_LayerItem            *bkgdSprite;        // the background element from the group
        SS_LayerItem            *gadSprite;         // the foreground element from the group
        SS_String               *labelString;       // the string associated with this gadget - will be group-addable later
        long                    Command;            // a default command-code

    public:
        float                   xpos, ypos;         // the position of this gadget
        float                   width, height;

    public:

                                SS_Gadget();        // default and inherited constructor
                                SS_Gadget(SS_GUI *g);
                                SS_Gadget(SS_GUI *g, float w, float h, Uint32 f=GAD_NOFLAGS);
                                SS_Gadget(SS_GUI *g, SS_LayerItem *item);
                                SS_Gadget(SS_GUI *g, SS_ItemGroup *grp);
                                SS_Gadget(SS_GUI *g, char *file1, char *file2=NULL, char *file3=NULL, char *file4=NULL);

        virtual                 ~SS_Gadget();

        virtual inline itemType   Type() const { return SS_ITEM_GADGET; }

        virtual void            Reset();
        void                    RemoveSelf();

        // Accessors
//      inline SS_World*        World()         { return world; }
        inline float            Width() const           { return width; }
        inline float            Height() const          { return height; }
        inline Uint32           Flags() const           { return flags; }
        inline Uint32           Flags(Uint32 m) const   { return flags & m; }
        inline Uint16           State() const           { return state; }
        inline SS_String*       String() const          { return labelString; }
        inline char*            Label() const           { if (labelString) { return labelString->Text(); } else { return NULL; } }
        inline SS_SFont*        Font() const            { return gadFont; }
        inline SS_LayerItem*    Sprite() const          { return gadSprite; }
        inline SS_ItemGroup*    SpriteGroup() const     { return (SS_ItemGroup*)this; }

        // Setters
        virtual void            SetWorld(SS_World *w);
        virtual void            SetGUI(SS_GUI *g);

        virtual void            SetRegion(float w, float h) { width = w; height = h; }

        inline void             SetFlags(Uint32 f)                      { flags = f; }
        inline void             EnableFlag(Uint32 f)                    { flags |= f; }
        inline void             DisableFlag(Uint32 f)                   { flags &= ~f; }
        inline void             Enable()                                { enabled = true; }
        inline void             Disable()                               { enabled = false; }
        inline void             SetHidden(bool h)                       { hideFlag = h; }
        inline void             Hide()                                  { SetHidden(true); }
        inline void             Show()                                  { SetHidden(false); }
        inline void             SetWidth(float w)                       { width = w; }
        inline void             SetHeight(float h)                      { height = h; }
        inline void             SetLabelAlignment(stringAlign la)       { labelAlignment = la; }

        inline void             SetNormalColor(colorSet &col)           { setNormal = col; }
        inline void             SetNormalBorderColor(SScolorb &col)     { setNormal.borderColor = col; }
        inline void             SetNormalFillColor(SScolorb &col)       { setNormal.fillColor = col; }
        inline void             SetNormalLabelColor(SScolorb &col)      { setNormal.labelColor = col; }

        inline void             SetHoverColor(colorSet &col)            { setHover = col; }
        inline void             SetHoverBorderColor(SScolorb &col)      { setHover.borderColor = col; }
        inline void             SetHoverFillColor(SScolorb &col)        { setHover.fillColor = col; }
        inline void             SetHoverLabelColor(SScolorb &col)       { setHover.labelColor = col; }

        inline void             SetPressedColor(colorSet &col)          { setPressed = col; }
        inline void             SetPressedBorderColor(SScolorb &col)    { setPressed.borderColor = col; }
        inline void             SetPressedFillColor(SScolorb &col)      { setPressed.fillColor = col; }
        inline void             SetPressedLabelColor(SScolorb &col)     { setPressed.labelColor = col; }

        inline void             SetDisabledColor(colorSet &col)         { setDisabled = col; }
        inline void             SetDisabledBorderColor(SScolorb &col)   { setDisabled.borderColor = col; }
        inline void             SetDisabledFillColor(SScolorb &col)     { setDisabled.fillColor = col; }
        inline void             SetDisabledLabelColor(SScolorb &col)    { setDisabled.labelColor = col; }

        inline void             SetButtonColors(colorSet &ncol, colorSet &hcol, colorSet &pcol, colorSet &dcol)
                                { setNormal = ncol; setHover = hcol; setPressed = pcol; setDisabled = dcol; }

        inline void             SetDisabledAlpha(GLubyte a) { setDisabled.borderColor.a = a; setDisabled.fillColor.a = a; setDisabled.labelColor.a = a; }

        inline void             SetState(Uint16 s) { state = s; }

        static SS_Sprite*       MakeSprite(char *file1, char *file2=NULL, char *file3=NULL, char *file4=NULL);
        static SS_ItemGroup*    MakeSpriteGroup(char *file1, char *file2 = NULL, char *file3=NULL, char *file4=NULL);

        void                    SetItem(SS_LayerItem *item);
        inline void             SetItem(char *file1, char *file2 = NULL, char *file3 = NULL, char *file4 = NULL);

        void                    SetItemGroup(SS_ItemGroup *grp);
        inline void             SetItemGroup(char *file1, char *file2 = NULL, char *file3 = NULL, char *file4 = NULL);

        virtual inline void     SetFont(SS_SFont *font) { gadFont = font; }

        void                    SetLabel(char *text);
        inline void             SetLabel(SS_String *str);
        void                    RealignLabel();

        virtual bool            TestRectCollision(float x, float y);
        bool                    TestLabelCollision(float x, float y);
        virtual bool            ContainsPoint(float x, float y);

        // Event Handling
        void                    SetHandler(ssEventCode type, gadgetEventProcPtr proc);
        virtual bool            HandleEvent(SS_Event *event);
        inline void             SetCommand(long command)    { Command = command; }
        virtual inline void     SendCommand(long command)   { if (gui) gui->HandleCommand(command); }
        inline void             DoIdle()                    { if (idleProc) (*idleProc)(this, NULL); }
        inline void             DoGainFocus()               { if (gainFocusProc) (*gainFocusProc)(this, NULL); }
        inline void             DoLoseFocus()               { if (loseFocusProc) (*loseFocusProc)(this, NULL); }
        inline void             DoChange()                  { if (onChangeProc) (*onChangeProc)(this, NULL); }

        virtual void            Update()                    {}      // Implement this to receive updates on your gadget

        // Event Support
        void                    MarkEntered();
        void                    MarkExited();
        bool                    IsEntered() const           { return rolloverNode != NULL; }

        virtual inline bool     IsPressed() const           { return Flags(GAD_AUTOPRESS) && IsClicked() && (Flags(GAD_DRAGGABLE) || pointerIn); }
        virtual inline bool     IsHover() const             { return Flags(GAD_AUTOROLLOVER) && !IsPressed() && IsEntered(); }
        virtual Uint16          FrameForState() const       { return (IsPressed() ? 1 : IsHover() ? 2 : 0); }

        inline bool             IsClicked() const           { return (gui && this == gui->clickedGadget); }
        Uint16                  CountClicks();
        inline void             ResetClicks()               { clickCount = 1; }
        inline void             SetMaxClicks(Uint16 m)      { maxClicks = m; }

        virtual void            Move(float x, float y);
        virtual void            Animate();

        const colorSet*         ColorSetForState() const;

        void                    Render() { Render(SS_WHITE_B); }
        virtual void            Render(const SScolorb &inTint);

    private:
        void                    Init();
};

#pragma mark -
//--------------------------------------------------------------
// SS_Button
//
class SS_Button : public SS_Gadget
{
    public:
                        SS_Button(SS_GUI *g);
                        SS_Button(SS_GUI *g, float w, float h, char *lab=NULL);
                        SS_Button(SS_GUI *g, SS_Sprite *sprite);
                        SS_Button(SS_GUI *g, char *file1, char *file2=NULL, char *file3=NULL);
                        ~SS_Button();

        virtual bool    HandleEvent(SS_Event *event);

    private:
        void            Init();
};


#pragma mark -
//--------------------------------------------------------------
// SS_Checkbox
//

enum {
    CHECKBOX_OFF = 0,
    CHECKBOX_ON,
    CHECKBOX_OFF_OVER,
    CHECKBOX_ON_OVER
};

class SS_Checkbox : public SS_Gadget
{
    public:
        static SS_LayerItem *checkboxSprite;

                            SS_Checkbox(SS_GUI *g, char *label=NULL);
                            SS_Checkbox(SS_GUI *g, float w, float h, char *label=NULL);
                            ~SS_Checkbox();

        void                LoadImages(char *off1, char *on1, char *off2=NULL, char *on2=NULL);
        static void         LoadDefaultImages(char *off1, char *on1, char *off2=NULL, char *on2=NULL);

        virtual Uint16      FrameForState();

        bool                HandleEvent(SS_Event *event);
        void                Render(const SScolorb &inTint);

    private:
        void                Init();
};


#pragma mark -
//--------------------------------------------------------------
// SS_RadioButton
//

enum {
    RADIO_OFF = 0,
    RADIO_ON,
    RADIO_OFF_OVER,
    RADIO_ON_OVER
};

class SS_RadioButton : public SS_Gadget
{
    private:
        static Uint16       currentGroupNum;
        static SS_LayerItem *radioButtonSprite;

        Uint16              groupNum;

    public:
                            SS_RadioButton(SS_GUI *g, Uint16 val, char *label=NULL);
                            ~SS_RadioButton();

        static inline void  BeginGroup(Uint16 g) { SS_RadioButton::currentGroupNum = g; }

        virtual Uint16      FrameForState();

        bool                HandleEvent(SS_Event *event);

    private:
        void                Init();
};


#pragma mark -
//--------------------------------------------------------------
// SS_Dial
//
class SS_Dial : public SS_Gadget
{
    public:

        float           minRad, maxRad;     // extent of the draggable area
        float           loVal, hiVal;       // values of the extremities
        float           tickSize;           // value constraint (i.e., 10, 20, 30...)
        float           value;              // the value of the slider

                        SS_Dial(SS_GUI *g, float loval, float hival, float loangle, float hiangle);
                        ~SS_Dial();

        inline void     SetLowValue(float lo) { loVal = lo; }
        inline void     SetHighValue(float hi) { hiVal = hi; }
        inline void     SetValueRange(float lo, float hi) { loVal = lo; hiVal = hi; }

        inline void     SetLowAngle(float lo) { minRad = RAD(lo); }
        inline void     SetHighAngle(float hi) { maxRad = RAD(hi); }
        inline void     SetAngleRange(float lo, float hi) { minRad = RAD(lo); maxRad = RAD(hi); }

        inline void     SetLowRad(float lo) { minRad = lo; }
        inline void     SetHighRad(float hi) { maxRad = hi; }
        inline void     SetRadRange(float lo, float hi) { minRad = lo; maxRad = hi; }

        void            SetValue(float v);
        void            SetAngle(float x);

        void            UpdateThumb();
        bool            HandleEvent(SS_Event *event);

    private:
        inline void     Init(float loval, float hival, float loangle, float hiangle);
};


#pragma mark -
//--------------------------------------------------------------
// SS_Thumb
//
//
class SS_Thumb : public SS_Gadget
{
    public:
        float           xoffs, yoffs;           // offset from top-left for thumb dragging
        float           xthumb, ythumb;         // the physical position of the thumb
        float           twidth, theight;        // the physical size of the thumb
        float           minxv, minyv;
        float           maxxv, maxyv;
        float           trepx, trepy;           // the amount represented by the thumb
        float           xvalue, yvalue;         // the value the thumb represents

                        SS_Thumb();
                        SS_Thumb(SS_GUI *g, float w, float h);
                        SS_Thumb(SS_GUI *g, SS_Sprite *sprite);
                        SS_Thumb(SS_GUI *g, char *file1);
                        SS_Thumb(SS_GUI *g, char *file1, char *file2);
                        SS_Thumb(SS_GUI *g, char *file1, char *file2, char *file3);
                        ~SS_Thumb();

        bool            HandleEvent(SS_Event *event);
        void            Render(const SScolorb &inTint);
        void            SetThumbPos(float x, float y)                           { xthumb = x; ythumb = y; UpdateValues(); }
        void            SetWholeSpan(float x1, float y1, float x2, float y2)    { minxv = x1; minyv = y1; maxxv = x2; maxyv = y2; UpdateSpan(); }
        void            SetThumbSpan(float rx, float ry)                        { trepx = rx; trepy = ry; UpdateSpan(); }
        void            SetThumbSize(float w, float h)                          { twidth = w; theight = h; trepx = (maxxv - minxv) * (twidth / width); trepy = (maxyv - minyv) * (theight / height); }
        void            SetValue(float xv, float yv)                            { xvalue = xv; yvalue = yv; UpdateThumbPos(); }

        void            UpdateValues()
        {
            if (twidth < width && maxxv >= minxv)
                xvalue = minxv + ( (maxxv - minxv) * (xthumb / (width - twidth)) );
            else
                xvalue = minxv;

            if (theight < height && maxyv >= minyv)
                yvalue = minyv + ( (maxyv - minyv) * (ythumb / (height - theight)) );
            else
                yvalue = minyv;
        }

        void            UpdateSpan()
        {
            if (maxxv > minxv)
                twidth = width * (trepx / (maxxv - minxv));
            else
                twidth = width;

            if (maxyv > minyv)
                theight = height * (trepy / (maxyv - minyv));
            else
                theight = height;

            UpdateThumbPos();
        }

        void            UpdateThumbPos()
        {
            if (maxxv > minxv)
                xthumb = (width - twidth) * (xvalue / (maxxv - minxv));
            else
                xthumb = 0.0f;

            if (maxyv > minyv)
                ythumb = (height - theight) * (yvalue / (maxyv - minyv));
            else
                ythumb = 0.0f;
        }

    private:
        void            Init(Uint32 f);
};


#pragma mark -
//--------------------------------------------------------------
// SS_Slider
//
class SS_Slider : public SS_Gadget
{
    public:
        static SS_ItemGroup *sliderSprite;

        bool            isVertical;         // vertical flag
        float           thumbY;             // thumb's y location
        float           minX, maxX;         // extent of the draggable area
        float           minVal, maxVal;     // values of the extremities
        float           tickSize;           // value constraint (i.e., 10, 20, 30...)
        float           value;              // the value of the slider

                        SS_Slider(SS_GUI *g, float x1, float x2, float y, float v1, float v2, bool isVert=false);
                        ~SS_Slider();

        static void     LoadImages(char *file1, char *file2, char *file3=NULL, char *file4=NULL);
        static void     LoadDefaultImages(char *file1, char *file2, char *file3=NULL, char *file4=NULL);

        inline void     SetVRange(float v1, float v2) { minVal = v1; maxVal = v2; }
        inline void     SetXRange(float x1, float x2) { minX = x1; maxX = x2; }
        inline void     SetAxis(float y) { thumbY = y; }
        void            SetValue(float v);
        void            SetPosition(float x);

        void            UpdateThumb();
        bool            HandleEvent(SS_Event *event);

    private:
        inline void     Init(float x1, float x2, float y, float v1, float v2, bool isVert=false);
};


#pragma mark -
//--------------------------------------------------------------
// SS_Scrollbar
//
// TODO: Reimplement the arrows and groove as buttons, and
// make the thumb behave in a slider-like way.
//
// Gadget Groups:
// It would make sense to have a standard way of implementing
// groups of gadgets that work together. The arrows, for example,
// should generate custom events back to the scrollbar, but there
// is no concept of a parent gadget, so buttons can't signal the
// scrollbar that something happened. A nice interface for this
// might be:
//
// button->SetDelegate(idCode, eventCode, scrollbar);
// and the handler for the button says:
// delegate->HandleEmbeddedEvent(idCode, &theEvent);
//
// The above scheme is simplest with no change to basic event
// handling. Other approaches require sub-views leading to
// spritegroup-like complexities.
//
// Consider a text-field with two scrollbars, however:
// The super-gadget has no parts of its own. It simply
// commands a bunch of sub-gadgets. When the view size
// changes it goes manually to each gadget in turn to move
// and resize it.
//
// A view scheme means that every gadget is a view. The pure
// view is essentially the Gadget class as it now exists.
// Any gadget can contain any number of child gadgets, and
// those gadgets can be assigned certain properties, such as
// moving or stretching in response to resize of the parent
// view.
//
// So every gadget gets flags for sticky sides, and knows what
// to do in response to resize of the parent view.
//
// The only reason the view is needed is to allow events to
// pass to children. This is extremely useful.
//
// I will continue to implement the scrollbar with a view
// towards manual creation, resizing, etc., then generalize
// all the code later in a view-like manner. SS_Gadget will then
// be called SS_View.
//
class SS_Scrollbar : public SS_Gadget
{
    public:
        static SS_Scrollbar *defaultScrollbar;

        float           minVal, maxVal;         // values of the extremities
        float           thumbRail;              // thumb's X location in a vertical scroller
        float           value;                  // the value that the thumb represents
        float           amount;                 // how much the thumb represents, in the same units

        SScolorb        thumbHoverTint;
        SScolorb        thumbPressedTint;
        SScolorb        arrowHoverColor;
        SScolorb        arrowPressedColor;

        Uint16          topGrooveHeight, stretchGrooveHeight, bottomGrooveHeight;
        Uint16          upArrowHeight, downArrowHeight;

        SS_ItemGroup    *thumbGroup, *structureGroup;
        SS_Sprite       *thumbTop, *thumbBottom, *thumbStretch, *thumbGrip;
        SS_Sprite       *grooveTop, *grooveBottom, *grooveStretch;
        SS_Sprite       *upArrow, *downArrow;

                        SS_Scrollbar(SS_GUI *g, float v1, float v2, float a=1);
                        ~SS_Scrollbar();

        inline void     SetRange(float v1, float v2, float a=-1);
        void            SetValue(float v);
        void            SetPosition(float y);
        void            SetAmount(float a) { amount = (a < minVal) ? minVal : ( (a > maxVal) ? maxVal : a ); UpdateThumb(); }
        float           Amount() { return amount; }
        void            SetThumbRail(float x) { thumbRail = x; UpdateThumb(); }

        void            Update() {}         // Update uses the width and height to
        void            UpdateThumb();
        bool            HandleEvent(SS_Event *event);

        void            LoadByName(char *prefix);

        void            SetParts(SS_Button *upArrow, SS_Button *downArrow, SS_Slider *scrollSlider);

        void            LoadStructure(
                            char *topGroove,        char *bottomGroove,     char *stretchGroove,
                            char *upNormal,         char *downNormal,
                            char *upPressed=NULL,   char *downPressed=NULL,
                            char *upHover=NULL,     char *downHover=NULL );

        void            LoadThumb(
                            char *topNormal,        char *bottomNormal,         char *stretchNormal,
                            char *topPressed=NULL,  char *bottomPressed=NULL,   char *stretchPressed=NULL,
                            char *topHover=NULL,    char *bottomHover=NULL,     char *stretchHover=NULL,
                            char *gripNormal=NULL,  char *gripPressed=NULL,     char *gripHover=NULL );

        void            MakeDefault();
        inline void     ExpandWidth(Uint16 w) { if (w > Width()) SetWidth(w); }

        void            SetRegion(float w, float h) { SS_Gadget::SetRegion(w, h); Update(); }
        inline void     Resize(float w, float h) { SetRegion(w, h); }

    private:
        inline void     Init(float v1, float v2, float a=1);
};


#pragma mark -
//--------------------------------------------------------------
// SS_Dragger
//
//
class SS_Dragger : public SS_Gadget
{
    public:
        float           xdiff, ydiff;

                        SS_Dragger();
                        SS_Dragger(SS_GUI *g, SS_Sprite *sprite);
                        SS_Dragger(SS_GUI *g, char *file1);
                        SS_Dragger(SS_GUI *g, char *file1, char *file2);
                        SS_Dragger(SS_GUI *g, char *file1, char *file2, char *file3);
                        SS_Dragger(SS_GUI *g, float w, float h, Uint32 f=GAD_DRAWBACKGROUND|GAD_DRAWBORDER);
                        ~SS_Dragger();

        bool            HandleEvent(SS_Event *event);

    private:
        void            Init(Uint32 f);
};


#pragma mark -
//--------------------------------------------------------------
// SS_TextInput
//
enum {
    SEL_CHAR = 0,
    SEL_WORD,
    SEL_ALL
};

class SS_TextInput : public SS_Gadget
{
    private:
        Uint16          selectMode;
        SS_EditString   *editString;
        float           xoff, yoff;

    public:
                            SS_TextInput(SS_GUI *g, SS_Sprite *spr);
                            SS_TextInput(SS_GUI *g, float w, float h);
                            ~SS_TextInput();

        virtual void        Reset();

        inline void         SetStringOffset(float x, float y) { xoff = x; yoff = y; }

        inline void         SetMaxStringLength(Uint16 max) { editString->SetMaxStringLength(max); }
        inline void         SetMaxStringWidth(Uint16 max) { editString->SetMaxStringWidth(max); }

        virtual void        Move(float x, float y);
        inline void         SetText(char *t) { editString->SetText(t); }
        inline void         SetFont(SS_SFont *f) { editString->SetFont(f); }

        inline void         GainFocus() { editString->GainFocus(); }
        inline void         LoseFocus() { editString->LoseFocus(); }

        inline Uint16       Length() { return editString->Length(); }
        inline char*        Text() { return editString->Text(); }

        inline void         SelectAll() { SetSelection(0, Length()); }
        inline void         SelectWord(Uint16 i, bool ext=false) { editString->SelectWord(i, ext); }
        inline void         SetSelection(Sint16 s=0, Sint16 l=0) { editString->SetSelection(s, l); }
        inline Uint16       SelectionStart() { return editString->SelectionStart(); }
        inline Sint16       SelectionLength() { return editString->SelectionLength(); }
        inline Uint16       SelectionEnd() { return editString->SelectionEnd(); }

        inline void         SetCursorTint(GLubyte r, GLubyte g, GLubyte b, GLubyte a) { editString->SetCursorTint(r, g, b, a); }
        inline void         SetCursorTint(GLubyte r, GLubyte g, GLubyte b) { editString->SetCursorTint(r, g, b); }
        inline void         SetCursorAlpha(GLubyte a) { editString->SetCursorAlpha(a); }

        inline void         SetCursorStyle(cursorStyle c) { editString->SetCursorStyle(c); }
        inline cursorStyle  CursorStyle() { return editString->CursorStyle(); }

        inline void         SetSelectionTint(GLubyte r, GLubyte g, GLubyte b, GLubyte a) { editString->SetSelectionTint(r, g, b, a); }
        inline void         SetSelectionTint(GLubyte r, GLubyte g, GLubyte b) { editString->SetSelectionTint(r, g, b); }
        inline void         SetSelectionAlpha(GLubyte a) { editString->SetSelectionAlpha(a); }

        virtual void        Render(const SScolorb &inTint);

        bool                HandleEvent(SS_Event *event);

    private:
        void                Init();
};


#pragma mark -
//--------------------------------------------------------------
// SS_StaticItem
//
// A simple sprite and/or label
//
class SS_StaticItem : public SS_Gadget
{
    public:
                        SS_StaticItem(SS_GUI *g, char *text=NULL, SS_LayerItem *spr=NULL);
                        SS_StaticItem(SS_GUI *g, float w, float h, Uint32 f=GAD_DRAWBACKGROUND|GAD_DRAWBORDER);
                        ~SS_StaticItem();

//      void            Init();

//      void            SetSpriteAndText(SS_Sprite *spr, char *text) { SetItem(spr); SetLabel(text); }
//      void            SetText(char *text, stringAlign align=(stringAlign)0);
};


#pragma mark -
//--------------------------------------------------------------
// SS_MenuItem
//
// A selectable item
//
class SS_MenuItem : public SS_Gadget
{
    private:
        bool            isChecked;
        SS_MenuItem     *nextItem, *prevItem;

    public:
                        SS_MenuItem(SS_GUI *g, char *font=NULL);
                        ~SS_MenuItem();

    private:
        void            Init();
};


#pragma mark -
//--------------------------------------------------------------
// SS_Menu
//
// A list of selectable strings
//
class SS_Menu : public SS_Gadget
{
    private:
        SS_MenuItem *firstItem, *lastItem;

    public:
                    SS_Menu(SS_GUI *g, char *font=NULL);
                    ~SS_Menu();

        void        AppendItem(SS_Sprite *spr, char *text);

    private:
        void        Init();
};


#pragma mark -
//--------------------------------------------------------------
// SS_Menubar
//
// A list of selectable strings
//
class SS_Menubar : public SS_Gadget
{
    private:
        SS_MenuItem *firstItem, *lastItem;

    public:
                    SS_Menubar(SS_GUI *g, char *font=NULL);
                    ~SS_Menubar();

        void        AppendMenu(SS_Sprite *spr, char *text=NULL);

    private:
        void        Init();
};


#pragma mark -
//--------------------------------------------------------------
// SS_CustomGadget
//
// A gadget with its own render procedure
//
class SS_CustomGadget : public SS_Gadget
{
    private:
        gadgetRenderProcPtr renderProc;
        float               rotation;
        char                *storage;

    public:
        float           misc[10];
        int             vals[10];

                        SS_CustomGadget(SS_GUI *g, gadgetRenderProcPtr r=NULL);
                        SS_CustomGadget(SS_GUI *g, float w, float h, gadgetRenderProcPtr r=NULL);
                        SS_CustomGadget(SS_GUI *g, char *file, gadgetRenderProcPtr r=NULL);
                        SS_CustomGadget(SS_GUI *g, SS_Sprite *sprite, gadgetRenderProcPtr r=NULL);
                        ~SS_CustomGadget() { if (storage != NULL) delete storage; }

        inline void     SetRenderProc(gadgetRenderProcPtr r) { renderProc = r; }
        char*           AllocStorage(int size) { char *s = new char[size]; storage = s; return s; }

        void            PushAndPrepareMatrix();
        inline void     RestoreMatrix() { glPopMatrix(); }
        virtual void    Render(const SScolorb &inTint);

    private:
        void            Init();
};

#endif

