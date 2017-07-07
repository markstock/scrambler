//
// scrambler
//
// Scramble a png image by displacing pixels based on HSV coordinates
//
// (c)2017 Mark J Stock
//

#include "lodepng.h"
#include <iostream>
#include <cmath>


//
// entry and exit
//
int main(int argc, char *argv[])
{
  const char* in_file = argc > 1 ? argv[1] : "test.png";
  const char* out_file = "out.png";

  // load in the test file
  std::vector<unsigned char> in_image; //the raw pixels
  unsigned int in_width, in_height;
  unsigned int error = lodepng::decode(in_image, in_width, in_height, in_file);
  //if there's an error, display it
  if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

  // decompose it to hsv
  std::vector<unsigned char> in_hsv;
  in_hsv.resize(in_width * in_height * 4);
  for (unsigned int i = 0; i < in_width*in_height; i++) {
    // assign r,g,b to temporaries
    unsigned char r = in_image[4*i+0];
    unsigned char g = in_image[4*i+1];
    unsigned char b = in_image[4*i+2];
    // copy over the alpha channel
    in_hsv[4*i+3] = in_image[4*i+3];
    // and make some references to the hsv array?

    // now convert
    // using code from https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
    unsigned char rgbMin, rgbMax;

    rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
    rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);

    in_hsv[4*i+2] = rgbMax;
    if (in_hsv[4*i+2] == 0)
    {
        in_hsv[4*i+0] = 0;
        in_hsv[4*i+1] = 0;
        continue;
    }

    in_hsv[4*i+1] = 255 * long(rgbMax - rgbMin) / in_hsv[4*i+2];
    if (in_hsv[4*i+1] == 0)
    {
        in_hsv[4*i+0] = 0;
        continue;
    }

    if (rgbMax == r)
        in_hsv[4*i+0] = 0 + 43 * (g - b) / (rgbMax - rgbMin);
    else if (rgbMax == g)
        in_hsv[4*i+0] = 85 + 43 * (b - r) / (rgbMax - rgbMin);
    else
        in_hsv[4*i+0] = 171 + 43 * (r - g) / (rgbMax - rgbMin);
  }

  // define the scrambling parameters
  unsigned int hue_bins = 8;
  float hue_displace = 0.05 * (float)std::max(in_width, in_height);

  // idea: bin all of the hues (easy!) and divide it into contiguous regions
  //   that way, for inputs with similar hue, we can still bin

  // create new image data arrays
  unsigned int band = (int)hue_displace + 2;
  unsigned int out_width = in_width + 2*band;
  unsigned int out_height = in_height + 2*band;
  std::vector<unsigned char> out_image;
  out_image.resize(out_width * out_height * 4);
  // fill with solid white
  for (unsigned int i = 0; i < out_width*out_height; i++) {
    out_image[4*i+0] = 255;
    out_image[4*i+1] = 255;
    out_image[4*i+2] = 255;
    out_image[4*i+3] = 255;
    //out_image[4*i+3] = 0;
  }
  // march through all input pixels and dislocate
  for (unsigned int y = 0; y < in_height; y++)
  for (unsigned int x = 0; x < in_width; x++)
  {
    size_t src_addr = 4*in_width*y + 4*x;
    // this is *sc1*
    // 0.0245436926 is 2pi/256
    //float angle = (float)in_hsv[src_addr+0] * 0.0245436926;
    // now use bins
    float angle = (hue_bins*in_hsv[src_addr+0]/255)/(float)hue_bins * 6.28318531;
    // idea: consider scaling the displacement by the value or saturation
    unsigned int destx = x + band + hue_displace * cos(angle);
    unsigned int desty = y + band + hue_displace * sin(angle);
    size_t dest_addr = 4*out_width*desty + 4*destx;
    // need to be able to splat this smoothly, but need blending to do that
    // need to understand how to do blending to make this work
    out_image[dest_addr + 0] = in_image[src_addr + 0];
    out_image[dest_addr + 1] = in_image[src_addr + 1];
    out_image[dest_addr + 2] = in_image[src_addr + 2];
    out_image[dest_addr + 3] = in_image[src_addr + 3];
  }

  // idea: make separate images of all of the quantized pixels, then create a shadow blur
  //   for each one, so that when we stack the new ones together, we can imitate depth
  //   by slightly darkening the buffer before overlaying the pixels from the next bin!
  // to get even more complicated, do the shadows in OpenGL and really have depth, then 
  //   one could even shuffle or jitter the layers in time


  // output to a new png
  error = lodepng::encode(out_file, out_image, out_width, out_height);
  //if there's an error, display it
  if (error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
}

