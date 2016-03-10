/*
 * =====================================================================================
 *
 *       Filename: sdldevice.hpp
 *        Created: 03/07/2016 23:57:04
 *  Last Modified: 03/09/2016 23:20:05
 *
 *    Description: copy from flare-engine:
 *				   SDLHardwareRenderDevice.h/cpp
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <SDL2/SDL.h>

class SDLDevice final
{
    public:
        SDLDevice();
       ~SDLDevice();

    public:
       void Present()
       {
           SDL_RenderPresent(m_Renderer);
       }

       void SetWindowTitle(const char *szUTF8Title)
       {
           SDL_SetWindowTitle(m_Window, (szUTF8Title) ? szUTF8Title : "");
       }

       void SetGamma(double fGamma)
       {
           Uint16 pRawRamp[256];
           SDL_CalculateGammaRamp((float)(std::min(std::max(fGamma, 0.0), 1.0)), pRawRamp);
           SDL_SetWindowGammaRamp(m_Window, pRawRamp, pRawRamp, pRawRamp);
       }

       void ClearScreen()
       {
           SetColor(0, 0, 0, 0XFF);
           SDL_RenderClear(m_Renderer);
       }

       void DrawLine(int nX, int nY, int nW, int nH)
       {
           SDL_RenderDrawLine(m_Renderer, nX, nY, nX + nW, nY + nH);
       }

       void SetColor(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nA)
       {
           SDL_SetRenderDrawColor(m_Renderer, nR, nG, nB, nA);
       }

       void DrawPixel(int nX, int nY)
       {
           SDL_RenderDrawPoint(m_Renderer, nX, nY);
       }

       void DrawRectangle(int nX, int nY, int nW, int nH)
       {
           DrawLine(nX     , nY     , nX + nW, nY     );
           DrawLine(nX     , nY     , nX     , nY + nH);
           DrawLine(nX + nW, nY     , nX + nW, nY + nH);
           DrawLine(nX     , nY + nH, nX + nW, nY + nH);
       }




    public:

       SDLDevice();
       int createContext(bool allow_fallback = true);

       virtual int render(Renderable& r, Rect& dest);
       virtual int render(Sprite* r);
       virtual int renderToImage(Image* src_image, Rect& src, Image* dest_image, Rect& dest);

       int renderText(FontStyle *font_style, const std::string& text, const Color& color, Rect& dest);
       Image *renderTextToImage(FontStyle* font_style, const std::string& text, const Color& color, bool blended = true);
       void drawPixel(int x, int y, const Color& color);
       void drawRectangle(const Point& p0, const Point& p1, const Color& color);
       void blankScreen();
       void commitFrame();
       void destroyContext();
       void windowResize();
       Image *createImage(int width, int height);
       void setGamma(float g);
       void updateTitleBar();
       void freeImage(Image *image);

       Image* loadImage(const std::string& filename,
               const std::string& errormessage = "Couldn't load image",
               bool IfNotFoundExit = false);
    private:
       void drawLine(int x0, int y0, int x1, int y1, const Color& color);

       SDL_Window *m_Window;
       SDL_Renderer *renderer;
       SDL_Texture *texture;
       SDL_Surface* titlebar_icon;
       char* title;
};


#ifndef RENDERDEVICE_H
#define RENDERDEVICE_H

#include <vector>
#include <map>
#include "Utils.h"

class Image;
class RenderDevice;
class FontStyle;

/** A Sprite representation
 *
 * A Sprite is instantiated from a Image instance using
 * Image::createSprite() which will increase the reference counter of
 * the image. Sprite::~Sprite() will release the reference to the
 * source image instance.
 *
 * A Sprite represents an area in a Image, it can be the full image or
 * just parts of the image such as an image atlas / spritemap.
 *
 * Sprite constructor is private to prevent creation of Sprites
 * outside of Image instance.
 *
 * @class Sprite
 * @author Henrik Andersson
 * @date 2014-01-30
 *
 */
class Sprite {

    public:
        ~Sprite();

        Rect local_frame;

        Image * getGraphics();
        void setOffset(const Point& _offset);
        void setOffset(const int x, const int y);
        Point getOffset();
        void setClip(const Rect& clip);
        void setClip(const int x, const int y, const int w, const int h);
        void setClipX(const int x);
        void setClipY(const int y);
        void setClipW(const int w);
        void setClipH(const int h);
        Rect getClip();
        void setDest(const Rect& _dest);
        void setDest(const Point& _dest);
        void setDest(int x, int y);
        void setDestX(int x);
        void setDestY(int y);
        FPoint getDest();
        int getGraphicsWidth();
        int getGraphicsHeight();
    private:
        Sprite(Image *);
        friend class Image;

    protected:
        /** reference to source image */
        Image *image;
        Rect src; // location on the sprite in pixel coordinates.
        Point offset;      // offset from map_pos to topleft corner of sprite
        FPoint dest;
};

/** An image representation
 * An image can only be instantiated, and is owned, by a RenderDevice
 * For a SDL render device this means SDL_Surface or a SDL_Texture, and
 * by OpenGL render device this is a texture.
 *
 * Image uses a refrence counter to control when to free the resource, when the
 * last reference is released, the Image is freed using RenderDevice::freeImage().
 *
 * The caller who instantiates an Image is responsible for release the reference
 * to the image when not used anymore.
 *
 * Image is a source for a Sprite and is therefor responsible for instantiating
 * Sprites using Image::createSprite().
 *
 * Creating a Sprite of a Image increases the reference counter, destructor of a
 * Sprite will release the reference to the image.
 *
 * @class Image
 * @author Henrik Andersson
 * @date 2014-01-30
 */
class Image {
    public:
        void ref();
        void unref();
        uint32_t getRefCount() const;

        virtual int getWidth() const;
        virtual int getHeight() const;

        virtual void fillWithColor(const Color& color) = 0;
        virtual void drawPixel(int x, int y, const Color& color) = 0;
        virtual Image* resize(int width, int height) = 0;

        class Sprite *createSprite(bool clipToSize = true);

    private:
        Image(RenderDevice *device);
        virtual ~Image();
        friend class SDLSoftwareImage;
        friend class SDLHardwareImage;

    private:
        RenderDevice *device;
        uint32_t ref_counter;
};

struct Renderable {
    public:
        Image *image; // image to be used
        Rect src; // location on the sprite in pixel coordinates.

        FPoint map_pos;     // The map location on the floor between someone's feet
        Point offset;      // offset from map_pos to topleft corner of sprite
        uint64_t prio;     // 64-32 bit for map position, 31-16 for intertile position, 15-0 user dependent, such as Avatar.
        Renderable()
            : image(NULL)
              , src(Rect())
              , map_pos()
              , offset()
              , prio(0) {
              }
};



/** Provide abstract interface for FLARE engine rendering devices.
 *
 * Provide an abstract interface for renderning a Renderable to the screen.
 * Each rendering device implementation must fully implement the interface.
 * The idea is that the render() method replicates the behaviour of
 * SDL_BlitSurface() with different rendering backends, but bases on the
 * information carried in a Renderable struct.
 *
 * As this is for the FLARE engine, implementations use the the engine's global
 * settings context.
 *
 * @class RenderDevice
 * @author Kurt Rinnert
 * @author Henrik Andersson
 * @date 2013-07-06
 *
 */
class RenderDevice {

    public:
        RenderDevice();
        virtual ~RenderDevice();

        /** Context operations */
        virtual int createContext(bool allow_fallback = true) = 0;
        virtual void destroyContext() = 0;
        virtual void setGamma(float g) = 0;
        virtual void updateTitleBar() = 0;

        /** factory functions for Image */
        virtual Image *loadImage(const std::string& filename,
                const std::string& errormessage = "Couldn't load image",
                bool IfNotFoundExit = false) = 0;
        virtual Image *createImage(int width, int height) = 0;
        virtual void freeImage(Image *image) = 0;

        /** Screen operations */
        virtual int render(Sprite* r) = 0;
        virtual int render(Renderable& r, Rect& dest) = 0;
        virtual int renderToImage(Image* src_image, Rect& src, Image* dest_image, Rect& dest) = 0;
        virtual int renderText(FontStyle *font_style, const std::string& text, const Color& color, Rect& dest) = 0;
        virtual Image* renderTextToImage(FontStyle* font_style, const std::string& text, const Color& color, bool blended = true) = 0;
        virtual void blankScreen() = 0;
        virtual void commitFrame() = 0;
        virtual void drawPixel(int x, int y, const Color& color) = 0;
        virtual void drawRectangle(const Point& p0, const Point& p1, const Color& color) = 0;
        virtual void windowResize() = 0;

        bool reloadGraphics();

    protected:
        /* Compute clipping and global position from local frame. */
        bool localToGlobal(Sprite *r);

        /* Image cache operations */
        Image *cacheLookup(const std::string &filename);
        void cacheStore(const std::string &filename, Image *);
        void cacheRemove(Image *image);
        void cacheRemoveAll();

        bool fullscreen;
        bool hwsurface;
        bool vsync;
        bool texture_filter;
        Point min_screen;

        bool is_initialized;
        bool reload_graphics;

        Rect m_clip;
        Rect m_dest;

    private:
        typedef std::map<std::string, Image *> IMAGE_CACHE_CONTAINER;
        typedef IMAGE_CACHE_CONTAINER::iterator IMAGE_CACHE_CONTAINER_ITER;

        IMAGE_CACHE_CONTAINER cache;

        virtual void drawLine(int x0, int y0, int x1, int y1, const Color& color) = 0;
};