#ifndef __IMAGE_H__
#define __IMAGE_H__
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

class RGBColor {
  public:
   uint8_t r;
   uint8_t g;
   uint8_t b;

  public:
   RGBColor() = default;
   RGBColor(uint8_t red, uint8_t green, uint8_t blue)
       : r(red), g(green), b(blue) {}
   RGBColor operator*(float scale);
   RGBColor operator+(int value);
};

class RGBColorImage {
   // Image origin is the left bottom corner
  public:
   RGBColorImage(size_t width, size_t height);
   RGBColor getPixel(size_t x, size_t y);
   void setPixel(size_t x, size_t y, RGBColor color);
   uint8_t* getData();  // for glfw
   size_t getWidth() const;
   size_t getHeight() const;

  private:
   size_t width, height;
   std::vector<RGBColor> data;
};

#endif