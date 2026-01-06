#pragma once
#include <string>

/**
 * @brief Opens a native file dialog to select a ZIP file.
 *
 * Displays the operating system's file chooser dialog filtered to
 * show ZIP archives (*.zip). The dialog does not change the current
 * working directory of the application.
 *
 * @return Absolute path to the selected ZIP file,
 *         or an empty string if the dialog was canceled.
 */
std::string OpenZipFileDialog(); // returns "" if canceled


/**
 * @brief Opens a native file dialog to select a Wavefront OBJ model file.
 *
 * Displays the operating system's file chooser dialog filtered to
 * show Wavefront OBJ files (*.obj). This is typically used to load
 * a new model into the currently active scene.
 *
 * @return Absolute path to the selected OBJ file,
 *         or an empty string if the dialog was canceled.
 */
std::string OpenObjFileDialog();

/**
 * @brief Opens a native "Save File" dialog for exporting a scene JSON file.
 *
 * @return Full file path chosen by the user, or empty string if canceled.
 */
std::string SaveJsonFileDialog();

/**
 * @brief Opens a native "Save File" dialog for exporting a rendered image.
 *
 * This dialog allows the user to choose a target file path for saving the
 * currently rendered output image (e.g. raytraced or preview image).
 *
 * Supported file formats:
 * - PNG (*.png)
 * - JPEG (*.jpg, *.jpeg)
 *
 * On Windows, a native file save dialog is used.
 * On non-Windows platforms, a fallback path is returned.
 *
 * @return Absolute or relative file path chosen by the user,
 *         or an empty string if the dialog was canceled.
 */
std::string SaveImageFileDialog();
