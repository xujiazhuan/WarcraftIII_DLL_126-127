# WarcraftIII_DLL_126-xxx
## Improvements for Warcraft III 126+, used in new DoTA (d1stats.ru)

# Features:

>
>	"Attack Speed" and "Magic amplification"
>![More unit info](/Images/AttackSpeedAndMagicAmplification.jpg?raw=true "You can see real Attack Speed and Magic amplification")
>
>
>	"Move Speed" and "Magic Protection" 
>![More unit info](/Images/MagicProtectionAndMoveSpeed.jpg?raw=true "You can see real Move Speed and Magic protection")
>	
>
>	"Widescreen support"
>
>*(Widescreen support disabled:)*
>![Widescreen off](/Images/WideScreen_NO.jpg?raw=true "Widescreen support disabled 16:9")
>*(Widescreen support enabled:)*
>![Widescreen on](/Images/WideScreen_YES.jpg?raw=true "Widescreen support enabled 16:9")
>
>
>	"Ally skill view"
>![Ally skill viewer](/Images/AllySkillView.jpg?raw=true "You can see ally skills")
>
>
>	"Ally manabar view"
>![Ally manabar viewer](/Images/ManaBar.jpg?raw=true "You can see ally manabars")


# RawImage update:

// Create RawImage (RGBA) with selected color.
int __stdcall CreateRawImage( int width, int height, RGBAPix defaultcolor )
// Load RawImage from ingame filename (tga,blp).
int __stdcall LoadRawImage( const char * filename, BOOL IsTga )
// Draw RawImage2 on RawImage.
int __stdcall RawImage_DrawImg( int RawImage, int RawImage2, int drawx, int drawy )
// Set color for point.
int __stdcall RawImage_DrawPixel( int RawImage, int x, int y, RGBAPix color )
// Draw rectangle with specified thickness and color.
int __stdcall RawImage_DrawRect( int RawImage, int drawx, int drawy, int widthsize, int heightsize, RGBAPix color )
// Draw line with specified thickness and color.
int __stdcall RawImage_DrawLine( int RawImage, int x1, int y1, int x2, int y2, int size, RGBAPix color )
// Draws circle with specified radius and thickness.
int __stdcall RawImage_DrawCircle( int RawImage, int x, int y, int radius, int size, RGBAPix color )
// Save RawImage to blp or tga and make visibled for game.
int __stdcall SaveRawImageToGameFile( int RawImage, const char * filename, BOOL IsTga )
// Save dump RawImage(tga) with specified filename
int __stdcall DumpRawImageToFile( int RawImage, const char * filename )
