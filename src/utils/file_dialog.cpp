#include "file_dialog.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

std::string OpenZipFileDialog()
{
    char filename[MAX_PATH] = {0};

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = nullptr;
    ofn.lpstrFile   = filename;
    ofn.nMaxFile    = MAX_PATH;

    // Filter string is double-null-terminated
    ofn.lpstrFilter = "Zip Files (*.zip)\0*.zip\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn))
        return std::string(filename);

    return "";
}
#else
std::string OpenZipFileDialog()  { return ""; }
#endif


std::string OpenObjFileDialog()
{
    char filename[MAX_PATH] = {0};

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = nullptr;
    ofn.lpstrFile   = filename;
    ofn.nMaxFile    = MAX_PATH;

    ofn.lpstrFilter = "Wavefront OBJ (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn))
        return std::string(filename);

    return "";
}


std::string SaveJsonFileDialog()
{
#ifdef _WIN32
    char fileName[MAX_PATH] = { 0 };

    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "Scene JSON (*.json)\0*.json\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "json";

    if (GetSaveFileNameA(&ofn))
    {
        return std::string(fileName);
    }

    return "";
#else
    // Fallback for Linux / macOS (no native dialog yet)
    return "assets/exported_scene.json";
#endif
}