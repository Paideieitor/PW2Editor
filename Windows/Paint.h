#ifndef _PAINT_H
#define _PAINT_H

#include "Windows/Window.h"

enum ActionType
{
    SELECT_PALETTE,
    SELECT_COLOR,
    CHANGE_COLOR,
    ZOOM,
    PAINT,
};

struct Action
{
    ActionType type;
    void* data;
};

struct PaintPoint
{
    u8 prev;
    u32 index;
};

enum Tool
{
    TOOL_BRUSH,
    TOOL_PICKER,
    TOOL_BUCKET,
    //TOOL_SELECT,
    TOOL_MAX,
};

class Paint : public Window
{
public:
    Paint(string imgPath, string palPath, string imgSavePath, string palSavePath, u32 flags);
    virtual ~Paint();

protected:
    virtual ReturnState RenderGUI() override;

    void ColorPalette();
    bool Canvas(ImVec2& canvasPos, ImVec2& pos);

    void BucketSpread(int x, int y, u8 prevCol, u8 col);

private:
    Image image;
    Palette palette;
    GLuint texture;

    bool update;

    int currPalIdx;
    int currColor;
    float zoom;
    ImVec4 selection;
    vector<PaintPoint> paintPoints;

    vector<Action> actions;

    u32 saveFlags;
    string imgSavePath, palSavePath;

    void SaveAll();

    bool UpdateTexture();

    void AddSelectAction(ActionType type, void* selection);
    void AddPaintAction();
    void BackwardAction();
};

#endif // _PAINT_H
