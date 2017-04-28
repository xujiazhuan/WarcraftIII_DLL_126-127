# WarcraftIII_DLL_126-127
## Improvements for Warcraft III 126a and 127a, used in new DoTA (d1stats.ru)

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
>
>![Widescreen off](/Images/WideScreen_NO.jpg?raw=true "Widescreen support disabled 16:9")
>
>*(Widescreen support enabled:)*
>
>![Widescreen on](/Images/WideScreen_YES.jpg?raw=true "Widescreen support enabled 16:9")
>
>
>
>	"Ally skill view"
>![Ally skill viewer](/Images/AllySkillView.jpg?raw=true "You can see ally skills")
>
>
>	"Ally manabar view"
>![Ally manabar viewer](/Images/ManaBar.jpg?raw=true "You can see ally manabars")


# RawImage update:
```cpp
// Создает RawImage (RGBA) с указанным цветом
// Create RawImage with color
int __stdcall CreateRawImage( int width, int height, RGBAPix defaultcolor )
// Загружает RawImage из filename (tga,blp)
// Load RawImage from game file
int __stdcall LoadRawImage( const char * filename )
// Рисует RawImage2 на RawImage
// Draw RawImage2 at RawImage
int __stdcall RawImage_DrawImg( int RawImage, int RawImage2, int drawx, int drawy, int blendmode )
// Заполняет выбранный пиксель указанным цветом
// Fill pixel with color
int __stdcall RawImage_DrawPixel( int RawImage, int x, int y, RGBAPix color )//RGBAPix = unsigned int
// Рисует прямоугольник с указанным цветом и размером
// Fill rectangle with color
int __stdcall RawImage_DrawRect( int RawImage, int drawx, int drawy, int widthsize, int heightsize, RGBAPix color )
// Рисует линию с указанным цветом и размером
// Draw line with color
int __stdcall RawImage_DrawLine( int RawImage, int x1, int y1, int x2, int y2, int size, RGBAPix color )
// Рисует круг с указанным радиусом, толщиной, цветом
// Draw circle with color
int __stdcall RawImage_DrawCircle( int RawImage, int x, int y, int radius, int size, RGBAPix color )
// Заполняет круг указанным цветом
// Fill circle with color
int __stdcall RawImage_FillCircle( int RawImage, int x, int y, int radius, RGBAPix color )
// Оставляет только круг с указанным радиусом
// Erase circle
int __stdcall RawImage_EraseCircle( int RawImage, int x, int y, int radius, BOOL inverse )
// Делает пиксели с цветом color - прозрачными, power от 0 до 255
// Not working
int __stdcall RawImage_EraseColor( int RawImage, RGBAPix color, int power )
// Загружает шрифт из файла
// Load font from game file
int __stdcall RawImage_LoadFontFromResource( const char * filepath )
// Устанавливает настройки шрифта для RawImage_DrawText
// Set font settings for DrawText
int __stdcall RawImage_SetFontSettings( const char * fontname, int fontsize, unsigned int flags )
// Пишет текст в указанных координатах с указанными цветом и настройками шрифта RawImage_SetFontSettings
// Draw text at RawImage
int __stdcall RawImage_DrawText( int RawImage, const char * text, int x, int y, RGBAPix color )
// Сохраняет RawImage в blp и делает доступным для использования в игре
// Save RawImage to game file
int __stdcall SaveRawImageToGameFile( int RawImage, const char * filename, BOOL IsTga, BOOL enabled )
// Сохраняет RawImage на диск в TGA по выбранному пути
// Save RawIamge to file
int __stdcall DumpRawImageToFile( int RawImage, const char * filename )
// Получает RawImage из списка RawImages по имени файла.
// Search RawImage by filename
int __stdcall GetRawImageByFile( const char * filename )
// Получает ширину RawImage
// Get width
int __stdcall RawImage_GetWidth( int RawImage )
// Получает высоту RawImage
// Get height
int __stdcall RawImage_GetHeight( int RawImage )
// Изменяет размер RawImage
// Resize RawImage
int __stdcall RawImage_Resize( int RawImage, int newwidth, int newheight )
// Рисует RawImage по заданным координатам (от 0.0 до 1.0) в игре. Можно установить размер (от 0.0 до 1.0).
// Draw RawImage as overlay ingame. xsize / ysize not working in this version (use 0.0)
int __stdcall RawImage_DrawOverlay( int RawImage, BOOL enabled, float xpos, float ypos, float xsize, float ysize )
```
"Example result:"
![RawImage Draw API](/Images/DrawRawImageApi.png?raw=true "You can see result of using experimental Draw API")

