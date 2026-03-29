#include "Windows/Paint.h"

#include "Utils/NDS/NCGRFormat.h"
#include "Utils/NDS/NCLRFormat.h"

Paint::Paint(string imgPath, string palPath, string imgSavePath, string palSavePath, u32 flags) : texture((ImTextureID)0), update(true), currPalIdx(0), currColor(0), zoom(1.0f), imgSavePath(imgSavePath), palSavePath(palSavePath), saveFlags(flags)
{
    FileStream imgStream;
    LoadFileStream(imgStream, imgPath);
    LoadNCGR(imgStream, image, flags);

    FileStream palStream;
    LoadFileStream(palStream, palPath);
    LoadNCLR(palStream, palette);
}

Paint::~Paint() {}

ReturnState Paint::RenderGUI()
{
    static bool commandInput = false;
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
        commandInput = true;
    if (ImGui::IsKeyReleased(ImGuiKey_LeftCtrl))
        commandInput = false;

    static bool pressInput = false;
    if (ImGui::IsKeyDown(ImGuiKey_MouseLeft))
        pressInput = true;
    if (ImGui::IsKeyReleased(ImGuiKey_MouseLeft))
        pressInput = false;

    if (commandInput)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_S))
            SaveAll();

        if (ImGui::IsKeyPressed(ImGuiKey_Z))
            BackwardAction();
    }

    ImGui::Begin("Paint");
    if (update)
    {
        UpdateTexture();
        update = false;
    }

    if (ImGui::Button("Save"))
    {
        SaveAll();
    }

    ColorPalette();

    const char* toolNames[] = { "Brush", "Picker", "Bucket", };
    static int toolSelected = (int)TOOL_BRUSH;

    if (ImGui::BeginCombo("Tool", toolNames[toolSelected], 0))
    {
        for (int n = 0; n < TOOL_MAX; n++)
        {
            const bool selected = (toolSelected == n);
            if (ImGui::Selectable(toolNames[n], selected))
                toolSelected = (Tool)n;

            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImVec2 pos;
    bool imgHover = Canvas(pos);

    ImGui::Text("Pos: (%d, %d)", (int)pos.x, (int)pos.y);
    u32 index = ((u32)pos.y * image.width) + (u32)pos.x;

    static int pressState = 0;
    if (imgHover && pressInput)
    {

        bool found = false;
        if (image.data[index] == (u8)currColor)
            found = true;

        if (toolSelected == TOOL_BRUSH)
        {
            for (u32 i = 0; i < paintPoints.size() && !found; ++i)
            {
                if (paintPoints.at(i).index == index)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                paintPoints.emplace_back((PaintPoint){ image.data[index], index });
                image.data[index] = currColor;
                update = true;
            }
        }

        pressState = 1;
    }
    else if (pressState == 1)
    {
        pressState = 2;
    }

    if (pressState == 2)
    {
        switch (toolSelected)
        {
            case TOOL_BRUSH:
                AddPaintAction();
                break;
            case TOOL_PICKER:
                if (currColor != image.data[index])
                {
                    AddSelectAction(SELECT_COLOR, &currColor);
                    currColor = image.data[index];
                }
                break;
            case TOOL_BUCKET:
                break;
        }
        pressState = 0;
    }

    ImGui::End();
    return OK;
}

bool Paint::Canvas(ImVec2& pos)
{
    float zoomPrev = zoom;
    if (ImGui::SliderFloat("Zoom", &zoom, 1.0f, 10.0f) && zoomPrev != zoom)
        AddSelectAction(ZOOM, &zoomPrev);

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();

    pos.x = (io.MousePos.x - canvasPos.x) / zoom;
    pos.y = (io.MousePos.y - canvasPos.y) / zoom;

    if (pos.x < 0.0f) { pos.x = 0.0f; }
    else if (pos.x > image.width) { pos.x = image.width; }
    if (pos.y < 0.0f) { pos.y = 0.0f; }
    else if (pos.y > image.height) { pos.y = image.height; }

    ImGui::Image((ImTextureID)texture, ImVec2(image.width * zoom, image.height * zoom));
    return ImGui::IsItemHovered();
}

void Paint::ColorPalette()
{
    ImGui::BeginGroup();
    {
        for (u32 palIdx = 0; palIdx < palette.count; ++palIdx)
        {
            if (palIdx != currPalIdx)
                ImGui::BeginDisabled();
            ImGui::BeginGroup();
            {

                for (u32 colIdx = 0; colIdx < palette.sizes[palIdx]; ++colIdx)
                {
                    if (colIdx != 0)
                        ImGui::SameLine();
                    u32 rgba = palette.Color(palIdx, colIdx);
                    u8* values = (u8*)&rgba;
                    ImVec4 color = {(float)values[0] / 255.0f, (float)values[1] / 255.0f, (float)values[2] / 255.0f, 1.0f};
                    string buttonLabel = ("##PaletteColor" + to_string(palIdx) + to_string(colIdx));

                    bool selectedColor = false;
                    if (palIdx == currPalIdx && colIdx == currColor)
                        selectedColor = true;

                    if (selectedColor)
                    {
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
                        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(255,255,255,255));
                    }

                    if (ImGui::ColorButton(buttonLabel.c_str(), color, 0, ImVec2(20, 20)))
                    {
                        AddSelectAction(SELECT_COLOR, &currColor);
                        currColor = colIdx;
                    }

                    if (selectedColor)
                    {
                        ImGui::PopStyleColor();
                        ImGui::PopStyleVar();
                    }
                }
            }
            ImGui::EndGroup();
            if (palIdx != currPalIdx)
                ImGui::EndDisabled();

            ImGui::SameLine();
            int currentPal = currPalIdx;
            if (ImGui::RadioButton(string("##PaletteIdx" + to_string(palIdx)).c_str(), &currPalIdx, palIdx))
                AddSelectAction(SELECT_PALETTE, &currentPal);
        }
    }
    ImGui::EndGroup();
    ImGui::SameLine();

    u32 rgba = palette.Color(currPalIdx, currColor);
    u8* values = (u8*)&rgba;
    ImVec4 color = {(float)values[0] / 255.0f, (float)values[1] / 255.0f, (float)values[2] / 255.0f, 1.0f};
    if (ImGui::ColorEdit3("##MainPaletteColor", &color.x, ImGuiColorEditFlags_NoInputs))
    {
        u32 rgbaPrev = rgba;

        values[0] = color.x * 255;
        values[1] = color.y * 255;
        values[2] = color.z * 255;
        values[3] = 255;

        if (rgbaPrev != rgba)
        {
            AddSelectAction(CHANGE_COLOR, &rgbaPrev);
            palette.Set(currPalIdx, currColor, rgba);
        }
    }
}

void Paint::SaveAll()
{
    FileStream imgStream;
    LoadEmptyFileStream(imgStream);
    SaveNCGR(imgStream, image, saveFlags);
    SaveFileStream(imgStream, imgSavePath);
    ReleaseFileStream(imgStream);

    FileStream palStream;
    LoadEmptyFileStream(palStream);
    SaveNCLR(palStream, palette);
    SaveFileStream(palStream, palSavePath);
    ReleaseFileStream(palStream);
}

bool Paint::UpdateTexture()
{
    if (texture != 0)
    {
        glDeleteTextures(1, &texture);
        texture = 0;
    }

    u32 x = 0;
    u32 y = 0;
    u32 width = image.width;
    u32 height = image.height;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    u32* pixels = new u32[width * height];
    for (u32 pY = y; pY < (y + height) && pY < image.height; ++pY)
    {
        u32 pos = pY * image.width;
        u32 pPos = (pY - y) * width;
        for (u32 pX = x; pX < (x + width) && pX < image.width; ++pX)
        {
            u32 colorIdx = image.data[pos + pX];
            pixels[pPos + (pX - x)] = palette.Color(currPalIdx, colorIdx);
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] pixels;
    return true;
}

void Paint::AddSelectAction(ActionType type, void* selection)
{
    Action action;
    action.type = type;

    action.data = new u32;
    memcpy(action.data, selection, sizeof(u32));

    actions.push_back(action);

    update = true;
}

void Paint::AddPaintAction()
{
    if (paintPoints.empty())
        return;

    Action action;
    action.type = PAINT;

    action.data = new vector<PaintPoint>;
    *((vector<PaintPoint>*)action.data) = paintPoints;
    paintPoints.clear();

    actions.push_back(action);
}

void Paint::BackwardAction()
{
    if (actions.empty())
        return;

    Action& action = actions.at(actions.size() - 1);
    switch (action.type)
    {
        case SELECT_PALETTE:
            currPalIdx = *((int*)action.data);
            delete (u32*)action.data;
            break;
        case SELECT_COLOR: 
            currColor = *((int*)action.data);
            delete (u32*)action.data;
            break;
        case CHANGE_COLOR: 
            {
                u32 rgba = *((u32*)action.data);
                palette.Set(currPalIdx, currColor, rgba);
            }
            delete (u32*)action.data;
            break;
        case ZOOM:
            zoom = *((float*)action.data);
            delete (u32*)action.data;
            break;
        case PAINT: 
            {
                vector<PaintPoint>* points = (vector<PaintPoint>*)action.data;
                for (u32 i = 0; i < points->size(); ++i)
                {
                    image.data[points->at(i).index] = points->at(i).prev;
                }
            }
            delete (vector<PaintPoint>*)action.data;
            break;
    }

    actions.erase(actions.begin() + actions.size() - 1);
    update = true;
}
