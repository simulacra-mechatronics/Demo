#ifndef BITMAPSKIN_H
#define BITMAPSKIN_H

#include <windows.h>

class BitmapSkin
{
    public:
        /** Default constructor */
        BitmapSkin(WORD wResBitmapUp);
        void loadFromResource();

        /** Default destructor */
        ~BitmapSkin();
    private:
        HBITMAP hSkinBitmap;
        unsigned int width;
        unsigned int height;

        void reportError();
};

#endif // BITMAPSKIN_H
