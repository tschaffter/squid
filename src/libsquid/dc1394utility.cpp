/**
 * Copyright (c) 2010-2012 Thomas Schaffter (thomas.schaff...@gmail.com)
 *
 * We release this software open source under a Creative Commons Attribution-
 * NonCommercial 3.0 Unported License. Please cite the papers listed on
 * http://tschaffter.ch/projects/squid/ when using sQuid in your publication.
 *
 * For commercial use, please contact Thomas Schaffter.
 *
 * A brief description of the license is available at:
 * http://creativecommons.org/licenses/by-nc/3.0/
 *
 * The full license is available at:
 * http://creativecommons.org/licenses/by-nc/3.0/legalcode
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "dc1394utility.h"
#include <tiffio.h>
#include <glog/logging.h>

using namespace squid;

const char* resolutionLabels[DC1394_VIDEO_MODE_NUM] = {

  "DC1394_VIDEO_MODE_160x120_YUV444",
  "DC1394_VIDEO_MODE_320x240_YUV422",
  "DC1394_VIDEO_MODE_640x480_YUV411",
  "DC1394_VIDEO_MODE_640x480_YUV422",
  "DC1394_VIDEO_MODE_640x480_RGB8",
  "DC1394_VIDEO_MODE_640x480_MONO8",
  "DC1394_VIDEO_MODE_640x480_MONO16",
  "DC1394_VIDEO_MODE_800x600_YUV422",
  "DC1394_VIDEO_MODE_800x600_RGB8",
  "DC1394_VIDEO_MODE_800x600_MONO8",
  "DC1394_VIDEO_MODE_1024x768_YUV422",
  "DC1394_VIDEO_MODE_1024x768_RGB8",
  "DC1394_VIDEO_MODE_1024x768_MONO8",
  "DC1394_VIDEO_MODE_800x600_MONO16",
  "DC1394_VIDEO_MODE_1024x768_MONO16",
  "DC1394_VIDEO_MODE_1280x960_YUV422",
  "DC1394_VIDEO_MODE_1280x960_RGB8",
  "DC1394_VIDEO_MODE_1280x960_MONO8",
  "DC1394_VIDEO_MODE_1600x1200_YUV422",
  "DC1394_VIDEO_MODE_1600x1200_RGB8",
  "DC1394_VIDEO_MODE_1600x1200_MONO8",
  "DC1394_VIDEO_MODE_1280x960_MONO16",
  "DC1394_VIDEO_MODE_1600x1200_MONO16",
  "DC1394_VIDEO_MODE_EXIF",
  "DC1394_VIDEO_MODE_FORMAT7_0",
  "DC1394_VIDEO_MODE_FORMAT7_1",
  "DC1394_VIDEO_MODE_FORMAT7_2",
  "DC1394_VIDEO_MODE_FORMAT7_3",
  "DC1394_VIDEO_MODE_FORMAT7_4",
  "DC1394_VIDEO_MODE_FORMAT7_5",
  "DC1394_VIDEO_MODE_FORMAT7_6",
  "DC1394_VIDEO_MODE_FORMAT7_7"
};

const char* fpsLabels[DC1394_FRAMERATE_NUM] = {

    "DC1394_FRAMERATE_1_875",
    "DC1394_FRAMERATE_3_75",
    "DC1394_FRAMERATE_7_5",
    "DC1394_FRAMERATE_15",
    "DC1394_FRAMERATE_30",
    "DC1394_FRAMERATE_60",
    "DC1394_FRAMERATE_120",
    "DC1394_FRAMERATE_240"
};

const char* codingLabels[DC1394_COLOR_CODING_NUM] = {

    "DC1394_COLOR_CODING_MONO8",
    "DC1394_COLOR_CODING_YUV411",
    "DC1394_COLOR_CODING_YUV422",
    "DC1394_COLOR_CODING_YUV444",
    "DC1394_COLOR_CODING_RGB8",
    "DC1394_COLOR_CODING_MONO16",
    "DC1394_COLOR_CODING_RGB16",
    "DC1394_COLOR_CODING_MONO16S",
    "DC1394_COLOR_CODING_RGB16S",
    "DC1394_COLOR_CODING_RAW8",
    "DC1394_COLOR_CODING_RAW16"
};

const float fpsValues[DC1394_FRAMERATE_NUM] = {

    1.875,
    3.75,
    7.5,
    15.0,
    30.0,
    60.0,
    120.0,
    240.0
};

// ======================================================================
// VIDEO MODE UTILITY METHODS

std::string squid::dc1394ToSringResolution(const dc1394video_mode_t resolution) throw(MyException*) {

    int i = (int)resolution;
    if (i < DC1394_VIDEO_MODE_MIN && i > DC1394_VIDEO_MODE_MAX)
        throw new MyException("Invalid dc1394 resolution.");

    return std::string(resolutionLabels[i-DC1394_VIDEO_MODE_MIN]);
}

// ----------------------------------------------------------------------

dc1394video_mode_t squid::stringToDc1394Resolution(const std::string strResolution) throw(MyException*) {

    for (unsigned int i = 0; i < DC1394_VIDEO_MODE_NUM; i++) {
        if (strcmp(strResolution.c_str(), resolutionLabels[i]) == 0)
            return (dc1394video_mode_t)(i + (unsigned int) DC1394_VIDEO_MODE_MIN);
    }
    throw new MyException("Invalid string resolution.");
}

// ----------------------------------------------------------------------

dc1394video_modes_t squid::getSupportedResolutions(dc1394camera_t* camera) throw(MyException*) {

    dc1394video_modes_t supportedResolutions;
    dc1394error_t err;
    if ((err = dc1394_video_get_supported_modes(camera, &supportedResolutions)) != DC1394_SUCCESS)
        throw new MyException("Failed to get supported video modes.");

    return supportedResolutions;
}

// ----------------------------------------------------------------------

std::vector<std::string> squid::getSupportedResolutionsLabels(dc1394camera_t* camera) throw(MyException*) {

    std::vector<std::string> list;
    dc1394video_modes_t supportedResolutions = getSupportedResolutions(camera);
    for (unsigned int i = 0; i < supportedResolutions.num; i++)
        list.push_back(dc1394ToSringResolution(supportedResolutions.modes[i]));

    return list;
}

// ----------------------------------------------------------------------

dc1394video_mode_t squid::getHighestSupportedResolution(dc1394camera_t* camera, dc1394video_modes_t* supportedResolutions, dc1394color_coding_t coding) throw(MyException*) {

    dc1394color_coding_t detectedCoding;
    dc1394video_mode_t resolution;
    int i;
    // select highest resolution mode among the non-scalable video mode
    for (i = supportedResolutions->num - 1; i >= 0; i--) {
        if (!dc1394_is_video_mode_scalable(supportedResolutions->modes[i])) {
            dc1394_get_color_coding_from_video_mode(camera, supportedResolutions->modes[i], &detectedCoding);
            if (detectedCoding == coding) {
                resolution = supportedResolutions->modes[i];
                break;
            }
        }
    }
    if (i < 0)
        throw new MyException("Failed to find at least one supported video mode with coding " + dc1394ToSringCoding(coding) + ".");

    return resolution;
}

// ----------------------------------------------------------------------

bool squid::isResolutionSupported(dc1394video_modes_t* supportedResolutions, dc1394video_mode_t resolution) {

    for (unsigned int i = 0; i < supportedResolutions->num; i++) {
        if (supportedResolutions->modes[i] == resolution)
            return true;
    }
    return false;
}

// ======================================================================
// FRAMERATE UTILITY METHODS

std::string squid::dc1394ToStringFps(const dc1394framerate_t fps) throw(MyException*) {

    int i = (int)fps;
    if (i < DC1394_FRAMERATE_MIN && i > DC1394_FRAMERATE_MAX)
        throw new MyException("Invalid dc1394 FPS.");

    return std::string(fpsLabels[i-DC1394_FRAMERATE_MIN]);
}

// ----------------------------------------------------------------------

dc1394framerate_t squid::stringToDc1394Fps(const std::string strFps) throw(MyException*) {

    for (unsigned int i = 0; i < DC1394_FRAMERATE_NUM; i++) {
        if (strcmp(strFps.c_str(), fpsLabels[i]) == 0)
            return (dc1394framerate_t)(i + (unsigned int) DC1394_FRAMERATE_MIN);
    }
    throw new MyException("Invalid string FPS.");
}

// ----------------------------------------------------------------------

unsigned int squid::dc1394FpsToPeriodInUs(dc1394framerate_t fps) {

    return 1000000/fpsValues[fps - DC1394_FRAMERATE_MIN];
}

// ----------------------------------------------------------------------

dc1394framerates_t squid::getSupportedFps(dc1394camera_t* camera, dc1394video_mode_t resolution) throw(MyException*) {

    dc1394framerates_t supportedFps;
    dc1394error_t err;
    if ((err = dc1394_video_get_supported_framerates(camera, resolution, &supportedFps)) != DC1394_SUCCESS)
        throw new MyException("Failed to get supported framerates.");

    return supportedFps;
}

// ----------------------------------------------------------------------

std::vector<std::string> squid::getSupportedFpsLabels(dc1394camera_t* camera, dc1394video_mode_t resolution) throw(MyException*) {

    std::vector<std::string> list;
    dc1394framerates_t supportedFps = getSupportedFps(camera, resolution);
    for (unsigned int i = 0; i < supportedFps.num; i++)
        list.push_back(dc1394ToStringFps(supportedFps.framerates[i]));

    return list;
}

// ----------------------------------------------------------------------

dc1394framerate_t squid::getHighestSupportedFps(dc1394framerates_t* supportedFps) throw(MyException*) {

    if (supportedFps->num == 0)
        throw new MyException("Failed to find at least one supported framerate.");

    return supportedFps->framerates[supportedFps->num-1];
}

// ----------------------------------------------------------------------

bool squid::isFpsSupported(dc1394framerates_t* supportedFps, dc1394framerate_t fps) {

    for (unsigned int i = 0; i < supportedFps->num; i++) {
        if (supportedFps->framerates[i] == fps)
            return true;
    }
    return false;
}

// ======================================================================
// CODING UTILITY METHODS

std::string squid::dc1394ToSringCoding(const dc1394color_coding_t coding) throw(MyException*) {

    int i = (int)coding;
    if (i < DC1394_COLOR_CODING_MIN && i > DC1394_COLOR_CODING_MAX)
        throw new MyException("Invalid dc1394 coding.");

    return std::string(codingLabels[i - DC1394_COLOR_CODING_MIN]);
}

// ----------------------------------------------------------------------

dc1394color_coding_t squid::stringToDc1394Coding(const std::string strCoding) throw(MyException*) {

    for (unsigned int i = 0; i < DC1394_COLOR_CODING_NUM; i++) {
        if (strcmp(strCoding.c_str(), codingLabels[i]) == 0)
            return (dc1394color_coding_t)(i + (unsigned int)DC1394_COLOR_CODING_MIN);
    }
    throw new MyException("Invalid string coding.");
}

// ======================================================================
// FRAME UTILITY METHODS

void squid::grayscale8bitsToPgm(const char* filename, unsigned char* image, const unsigned int width, const unsigned int height) throw(MyException*) {

    FILE* imageFile_ = NULL;
    imageFile_ = fopen(filename, "wb");

    if (imageFile_ == NULL)
        throw new MyException("Unable to create " + std::string(filename));

    fprintf(imageFile_, "P5\n%u %u 255\n", width, height);
    if ((unsigned int)height * width != fwrite(image, 1, height * width, imageFile_))
        throw new MyException("fwrite error");

    if (fclose(imageFile_) != 0)
        throw new MyException("Unable to close the stream.");
}

// ----------------------------------------------------------------------

void squid::grayscale8bitsToTiff(const char* filename, unsigned char* image, const unsigned int width, const unsigned int height) throw(MyException*) {

    float xres, yres;
    xres = yres = 100;
    unsigned short res_unit = RESUNIT_CENTIMETER;
    unsigned short spp = 1;
    unsigned short bppSave = 8; // save in 8bits
    unsigned short compression = COMPRESSION_NONE;
    unsigned short photo = PHOTOMETRIC_MINISBLACK;
    unsigned short orientation = ORIENTATION_TOPLEFT;
    unsigned short fillorder = FILLORDER_MSB2LSB;
    unsigned short planarconfig = PLANARCONFIG_CONTIG;

    // open TIFF
    TIFF* out = TIFFOpen(filename, "w");
    if (out == NULL)
        throw new MyException("Cannot write TIFF to " + std::string(filename));

     uint16 array[256 * 256];
     for (int j = 0; j < 256; j++) {
         for(int i = 0; i < 256; i++)
                array[j * 256 + i] = i * j;
     }

    // write the in order to define the image
    if( (TIFFSetField(out, TIFFTAG_IMAGEWIDTH,  width / spp) == 0)  ||
    (TIFFSetField(out, TIFFTAG_IMAGELENGTH,     height) == 0)       ||
    (TIFFSetField(out, TIFFTAG_BITSPERSAMPLE,   bppSave) == 0)      ||
    (TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, spp) == 0)          ||
    (TIFFSetField(out, TIFFTAG_COMPRESSION,     compression) == 0)  ||
    (TIFFSetField(out, TIFFTAG_PLANARCONFIG,    planarconfig) == 0) ||
    (TIFFSetField(out, TIFFTAG_PHOTOMETRIC,     photo) == 0)        ||
    (TIFFSetField(out, TIFFTAG_ORIENTATION,     orientation) == 0)  ||
    (TIFFSetField(out, TIFFTAG_FILLORDER,       fillorder) == 0)    ||
    (TIFFSetField(out, TIFFTAG_XRESOLUTION,     xres) == 0)         ||
    (TIFFSetField(out, TIFFTAG_YRESOLUTION,     yres) == 0)         ||
    (TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT,  res_unit) ==0 ) ) {
        TIFFClose(out);
        throw new MyException("Unable to write all Tags to TIFF file " + std::string(filename));
    }

//    // 16 bit format data
//    unsigned char* pTmpImgBuf = new unsigned char[width * height];
//    memcpy((void*)pTmpImgBuf, (const void*)pImageBuf, (width * sizeof(unsigned char)) * height );
//
//    // shift data, if necassary
//    int SHL = bppSave-bppReal;
//    if(SHL > 0)
//    {
//        for( unsigned int c=0; c<(width*height); c++ )
//        {
//            pTmpImgBuf[c] = (pTmpImgBuf[c] << SHL);
//        }
//    }
//
//    // write image data to TIFF
//    for(unsigned int yC = 0; yC < height; yC++)
//    {
//        if(TIFFWriteScanline(out, &(pTmpImgBuf[yC * width]), yC) == -1)
//        {
//            LOG(ERROR) << "Unable to write scanline #" << yC << ": " << szFilename << std::endl;
//            delete [] pTmpImgBuf;
//            TIFFClose(out);
//            return false;
//        }
//    }
//
//    delete [] pTmpImgBuf;

    // Write the information to the file
    if (TIFFWriteEncodedStrip(out, 0, image, width * height) == 0) {
        TIFFClose(out);
        throw new MyException("Unable to encode TIFF image " + std::string(filename));
    }
    TIFFClose(out);
}
