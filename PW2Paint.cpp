#include "System.h"

#include "Windows/Paint.h"

#include "Utils/FileUtils.h"

#include "Utils/NDS/NCLRFormat.h"
#include "Utils/NDS/NCGRFormat.h"
int main(int argc, char* argv[])
{
    int t = 0;
    if (t == 1)
    {
        FileStream test;
        LoadFileStream(test, "Test/pkstaticPal");
        Palette pal;
        LoadNCLR(test, pal);
        ReleaseFileStream(test);
        SaveNCLR(test, pal);
        SaveFileStream(test, "Test/pkstaticPalSave");
        return 0;
    }
    else if (t == 2)
    {
        FileStream test;
        LoadFileStream(test, "Test/pksprite");
        Image img;
        LoadNCGR(test, img, IMAGE_FLAG_DECOMP);
        ReleaseFileStream(test);
        SaveNCGR(test, img, IMAGE_FLAG_DECOMP);
        SaveFileStream(test, "Test/pkspriteSave");
        return 0;
    }

    string imgPath, palPath, imgSavePath, palSavePath;
    u32 flags = 0;

    for (int arg = 1; arg < argc; ++arg)
    {
        if (strcmp(argv[arg], "-i") == 0)
        {
            ++arg;
            imgPath = argv[arg];
        }
        else if (strcmp(argv[arg], "-p") == 0)
        {
            ++arg;
            palPath = argv[arg];
        }
        else if (strcmp(argv[arg], "-si") == 0)
        {
            ++arg;
            imgSavePath = argv[arg];
        }
        else if (strcmp(argv[arg], "-sp") == 0)
        {
            ++arg;
            palSavePath = argv[arg];
        }
        else if (strcmp(argv[arg], "-f") == 0)
        {
            ++arg;
            flags = stoi(argv[arg]);
        }
    }

    CreateFolder("Logs");

    ReturnState state = OK;

    Paint* paint = new Paint(imgPath, palPath, imgSavePath, palSavePath, flags);
    state = paint->Init(100, 100, "PW2Paint", DEFAULT_FONT, (float)DEFAULT_FONT_SIZE);
    if (state != OK)
    {
        Log(CRITICAL, "Failed loading Paint!");
        return 0;
    }
    Log(INFO, "Paint Loop");
    while (state == OK)
    {
        state = paint->Update();
        if (state == EXIT)
        {
            // TODO add save before quit
        }
    }
    delete paint;

    Log(INFO, "Paint closed");
    return 0;
}
