#include "JPEGWriter.h"

#include <stdio.h>

extern "C"
{
#include <jpeglib.h>
#include <jerror.h>
}

bool WriteJPEG(
    const std::string& filename,
    int width,
    int height,
    int quality,
    const unsigned char* rgbData)
{
    FILE* file = fopen(filename.c_str(), "wb");

    if (!file)
        return false;

    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);

    jpeg_stdio_dest(&cinfo, file);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo, quality, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    while (cinfo.next_scanline < cinfo.image_height)
    {
        JSAMPROW row =
            (JSAMPROW)&rgbData[cinfo.next_scanline * width * 3];

        jpeg_write_scanlines(
            &cinfo,
            &row,
            1);
    }

    jpeg_finish_compress(&cinfo);

    jpeg_destroy_compress(&cinfo);

    fclose(file);

    return true;
}