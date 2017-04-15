#include "Main.h"
#include "blpaletter.h"
#include <windows.h>
#include <string>
#include <tlhelp32.h>
#include <iostream>
#include <gl\gl.h>
#include <gl\glu.h>

#pragma comment(lib,"opengl32.lib")

typedef BOOL( __stdcall * wglSwapLayerBuffers_p )( HDC, UINT );

wglSwapLayerBuffers_p wglSwapLayerBuffers_org;
wglSwapLayerBuffers_p wglSwapLayerBuffers_ptr;

//GLint maxviewport[ 2 ];

void DrawAllRawImages( )
{
	glEnable( GL_BLEND );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	float ScreenX = *GetWindowXoffset;
	float ScreenY = *GetWindowYoffset;

	glViewport( 0, 0, ( GLsizei )ScreenX, ( GLsizei )ScreenY );
	glOrtho( 0.0, ScreenX, ScreenY, 0, -1.0, 1.0 );
	glMatrixMode( GL_MODELVIEW );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	float zoomx = ScreenX / DesktopScreen_Width;
	float zoomy = ScreenY / DesktopScreen_Height;

	for ( auto & img : ListOfRawImages )
	{
		if ( !img.used_for_overlay )
			continue;

		glLoadIdentity( );
	
	//	glTranslatef( ScreenX *img.overlay_x, ScreenY *img.overlay_y, 0.0f );

		//float AspectRatio = DesktopScreen_Height / DesktopScreen_Width;
		//float AspectRatio2 = DesktopScreen_Width / DesktopScreen_Height;
	
		glPixelZoom( zoomx, -zoomy );
		

		Buffer tmpBuf = Buffer( );
		tmpBuf.Clone( img.img );
		glRasterPos3f( ScreenX *img.overlay_x, ScreenY *img.overlay_y,0.0f );
		flip_vertically( reinterpret_cast< BYTE* >( &tmpBuf[ 0 ] ), img.width, img.height, 4 );
		glDrawPixels( img.width, img.height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, tmpBuf.buf );
		tmpBuf.Clear( );

	}

	glDisable( GL_BLEND );
}

HGLRC DotaGlobalOverlay_OPENGL = NULL;
HDC GlobalDc = NULL;

BOOL __stdcall wglSwapLayerBuffers_my( HDC dc, UINT b )
{
	if ( !dc || !*InGame ) {
		return	wglSwapLayerBuffers_ptr( dc, b );
	}

	GlobalDc = dc;

	if ( !DotaGlobalOverlay_OPENGL )
		DotaGlobalOverlay_OPENGL = wglCreateContext( dc );

	/*HGLRC oldcontext = wglGetCurrentContext( );

	wglMakeCurrent( dc, DotaGlobalOverlay_OPENGL );

	DrawAllRawImages( );

	wglMakeCurrent( dc, oldcontext );*/

	return wglSwapLayerBuffers_ptr( dc, b );
}

void DrawOverlayGl( )
{
	if ( !GlobalDc || !*InGame ) {
		return;
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__ );
#endif

	if ( !DotaGlobalOverlay_OPENGL )
		DotaGlobalOverlay_OPENGL = wglCreateContext( GlobalDc );


	HGLRC oldcontext = wglGetCurrentContext( );

	wglMakeCurrent( GlobalDc, DotaGlobalOverlay_OPENGL );

	DrawAllRawImages( );

	wglMakeCurrent( GlobalDc, oldcontext );


}


void UninitOpenglHook( )
{
	MH_DisableHook( wglSwapLayerBuffers_org );
}

//
//typedef int( __fastcall  * DrawWc3UI_p )( int );
//DrawWc3UI_p DrawWc3UI_org;
//DrawWc3UI_p DrawWc3UI_ptr;
//
//int __fastcall  DrawWc3UI_my( int a1 )
//{
//	if ( !GlobalDC || !*InGame ) {
//		return DrawWc3UI_ptr( a1 );
//	}
//
//
//
//	if ( !DotaGlobalOverlay_OPENGL )
//		DotaGlobalOverlay_OPENGL = wglCreateContext( GlobalDC );
//
//	HGLRC oldcontext = wglGetCurrentContext( );
//
//	wglMakeCurrent( GlobalDC, DotaGlobalOverlay_OPENGL );
//
//	DrawAllRawImages( );
//
//	wglMakeCurrent( GlobalDC, oldcontext );
//
//
//	return DrawWc3UI_ptr( a1 );
//}DrawWc3UI_org = ( DrawWc3UI_p )( GameDll + 0x6290C0 );
//MH_CreateHook( DrawWc3UI_org, &DrawWc3UI_my, reinterpret_cast< void** >( &DrawWc3UI_ptr ) );
//MH_EnableHook( DrawWc3UI_org );

void InitOpenglHook( )
{
	HMODULE hOpenGL = GetModuleHandleA( "opengl32.dll" );
	if ( hOpenGL )
	{
		wglSwapLayerBuffers_org = ( wglSwapLayerBuffers_p )GetProcAddress( hOpenGL, "wglSwapLayerBuffers" );
		MH_CreateHook( wglSwapLayerBuffers_org, &wglSwapLayerBuffers_my, reinterpret_cast< void** >( &wglSwapLayerBuffers_ptr ) );
		MH_EnableHook( wglSwapLayerBuffers_org );
	}
	//	glGetIntegerv( GL_MAX_VIEWPORT_DIMS, maxviewport );

}

