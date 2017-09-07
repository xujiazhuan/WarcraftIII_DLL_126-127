#include "Main.h"
#include <d3d9.h>
#include <d3dx9.h>
//#pragma comment(lib,"d3dx9.lib")

// Thanks ENAleksey(http://xgm.guru/user/ENAleksey) for help 

IDirect3DDevice9 * deviceglobal = NULL;

/*************************************************************************
* D3DXMatrixTransformation2D
*/
D3DXMATRIX* WINAPI D3DXMatrixTransformation2D(
	D3DXMATRIX *pout, CONST D3DXVECTOR2 *pscalingcenter,
	FLOAT scalingrotation, CONST D3DXVECTOR2 *pscaling,
	CONST D3DXVECTOR2 *protationcenter, FLOAT rotation,
	CONST D3DXVECTOR2 *ptranslation )
{
	D3DXQUATERNION rot, sca_rot;
	D3DXVECTOR3 rot_center, sca, sca_center, trans;

	if ( pscalingcenter )
	{
		sca_center.x = pscalingcenter->x;
		sca_center.y = pscalingcenter->y;
		sca_center.z = 0.0f;
	}
	else
	{
		sca_center.x = 0.0f;
		sca_center.y = 0.0f;
		sca_center.z = 0.0f;
	}

	if ( pscaling )
	{
		sca.x = pscaling->x;
		sca.y = pscaling->y;
		sca.z = 0.0f;
	}
	else
	{
		sca.x = 0.0f;
		sca.y = 0.0f;
		sca.z = 0.0f;
	}

	if ( protationcenter )
	{
		rot_center.x = protationcenter->x;
		rot_center.y = protationcenter->y;
		rot_center.z = 0.0f;
	}
	else
	{
		rot_center.x = 0.0f;
		rot_center.y = 0.0f;
		rot_center.z = 0.0f;
	}

	if ( ptranslation )
	{
		trans.x = ptranslation->x;
		trans.y = ptranslation->y;
		trans.z = 0.0f;
	}
	else
	{
		trans.x = 0.0f;
		trans.y = 0.0f;
		trans.z = 0.0f;
	}

	rot.w = cos( rotation / 2.0f );
	rot.x = 0.0f;
	rot.y = 0.0f;
	rot.z = sin( rotation / 2.0f );

	sca_rot.w = cos( scalingrotation / 2.0f );
	sca_rot.x = 0.0f;
	sca_rot.y = 0.0f;
	sca_rot.z = sin( scalingrotation / 2.0f );

	D3DXMatrixTransformation( pout, &sca_center, &sca_rot, &sca, &rot_center, &rot, &trans );

	return pout;
}


typedef HRESULT( WINAPI *
	D3DXCreateSprite_p )(
		LPDIRECT3DDEVICE9   pDevice,
		LPD3DXSPRITE*       ppSprite );

D3DXCreateSprite_p D3D9CreateSprite_org;


typedef HRESULT( WINAPI *
	D3DXFilterTexture_p )(
		LPDIRECT3DBASETEXTURE9    pBaseTexture,
		CONST PALETTEENTRY*       pPalette,
		UINT                      SrcLevel,
		DWORD                     Filter );

D3DXFilterTexture_p D3DXFilterTexture_org;

void DrawImage( ID3DXSprite* pSprite, IDirect3DTexture9* texture, float width, float height, float x, float y )
{
	D3DXMATRIX matAll;
	float scalex = *GetWindowXoffset / DefaultSceenWidth;
	float scaley = *GetWindowYoffset / DefaultSceenHeight;


	//scalex *= DesktopScreen_Width / DefaultSceenWidth;
	//scaley *= DesktopScreen_Height / DefaultSceenHeight;

	float posx = x;
	float posy = y + scaley * height;

	if ( x <= 0.0f )
		x = 0.0f;

	if ( x >= *GetWindowYoffset )
		x = *GetWindowYoffset;

	D3DXVECTOR2 position = D3DXVECTOR2( posx, posy );
	D3DXVECTOR2 scaling( scalex, -scaley );
	D3DXVECTOR2 spriteCentre = D3DXVECTOR2( width *scalex / 2, height * scaley / 2 );

	D3DXMatrixTransformation2D( &matAll, NULL, 0.0f, &scaling, &spriteCentre, 0.0f, &position );

	pSprite->SetTransform( &matAll );
	pSprite->Draw( texture, NULL, NULL, NULL, 0xffffffff );

}

//bool dumped = false;
//void DumpRenderState( )
//{
//	if ( !dumped )
//	{
//		dumped = true;
//		FILE * f;
//		fopen_s( &f, "dumpex.txt", "w" );
//		for ( int i = 0; i < 175; i++ )
//		{
//			DWORD val;
//			deviceglobal->GetRenderState( ( D3DRENDERSTATETYPE )i, &val );
//			fprintf_s( f, "%i-%X\n", i, val );
//		}
//		fclose( f );
//	}
//
D3DMATERIAL9 oldmat;
D3DLIGHT9 oldlight2;
D3DLIGHT9 oldlight[ 256 ];

void SetNewLightDx9( int id )
{
	if ( deviceglobal != NULL )
	{
		for ( int i = 0; i < 256; i++ )
		{
			BOOL enabled = FALSE;
			deviceglobal->GetLightEnable( i, &enabled );
			if ( enabled )
			{
				D3DLIGHT9 mylight = oldlight[ i ];
				deviceglobal->GetLight( i, &oldlight[ i ] );
				mylight.Diffuse.r *= 5.2f;
				mylight.Diffuse.g *= 5.2f;
				mylight.Diffuse.b *= 5.2f;

				mylight.Ambient.r *= 5.2f;
				mylight.Ambient.g *= 5.2f;
				mylight.Ambient.b *= 5.2f;
				deviceglobal->SetLight( i, &mylight );
			}
		}
		/*D3DLIGHT9 mymat;


		mymat.Ambient.r *= 1.2;
		mymat.Ambient.g *= 1.2;
		mymat.Ambient.b *= 1.2;
		mymat.Diffuse.r *= 1.2;
		mymat.Diffuse.g *= 1.2;
		mymat.Diffuse.b *= 1.2;
		mymat.Specular.r *= 1.2;
		mymat.Specular.g *= 1.2;
		mymat.Specular.b *= 1.2;

		deviceglobal->SetLight( 0, &mymat );
	*/

	//if ( IsKeyPressed( '2' ) )
	//	deviceglobal->SetTexture( 0, 0 );
	/*deviceglobal->GetLight( 0, &oldlight );



	D3DLIGHT9 mylight = oldlight;
	mylight.Ambient.r = 0.0;
	mylight.Ambient.g = 0.0;
	mylight.Ambient.b = 1.0;
	mylight.Diffuse.r = 0.0;
	mylight.Diffuse.g = 0.0;
	mylight.Diffuse.b = 1.0;
	mylight.Specular.r = 0.0;
	mylight.Specular.g = 0.0;
	mylight.Specular.b = 1.0;

	deviceglobal->SetLight( 0, &mylight );*/
	}
}

void SetOldLightDx9( int id )
{
	if ( deviceglobal != NULL )
	{
		for ( int i = 0; i < 256; i++ )
		{
			BOOL enabled = FALSE;
			deviceglobal->GetLightEnable( i, &enabled );
			if ( enabled )
			{
				deviceglobal->SetLight( i, &oldlight[ i ] );
			}
		}

	}
}

typedef HRESULT( __stdcall *  DrawIndexedPrimitiveDx9p )( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount );
DrawIndexedPrimitiveDx9p DrawIndexedPrimitiveDx9_org = NULL;


HRESULT __stdcall DrawIndexedPrimitiveDx9( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount )
{
	HRESULT retval;
	if ( IsKeyPressed( '1' ) )
		SetNewLightDx9( 0 );
	retval = DrawIndexedPrimitiveDx9_org( pDevice, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount );

	return retval;
}




void DrawOverlayDx9( )
{
	HRESULT rv;
	IDirect3DDevice9 * d = deviceglobal;
	if ( !d || OverlayDrawed )
	{
		return;
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif

	ID3DXSprite* pSprite;
	rv = D3D9CreateSprite_org( d, &pSprite );
	if ( !pSprite || rv != S_OK )
		return;
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif

	pSprite->Begin( D3DXSPRITE_ALPHABLEND );
	d->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	d->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	d->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );


#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	for ( auto & img : ListOfRawImages )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		if ( !img.used_for_overlay )
		{
			if ( img.textureaddr )
			{
				IDirect3DTexture9 * ppTexture = ( IDirect3DTexture9 * )img.textureaddr;
				ppTexture->Release( );
				ppTexture = NULL;
				img.needResetTexture = FALSE;
				img.textureaddr = NULL;
			}
			continue;
		}
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		if ( img.needResetTexture )
		{
			img.needResetTexture = FALSE;
			if ( img.textureaddr )
			{
				IDirect3DTexture9 * ppTexture = ( IDirect3DTexture9 * )img.textureaddr;
				ppTexture->Release( );
				ppTexture = NULL;
				img.textureaddr = NULL;
			}
		}
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif

		IDirect3DTexture9 * ppTexture = NULL;
		if ( img.textureaddr )
		{
#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
			ppTexture = ( IDirect3DTexture9 * )img.textureaddr;
		}
		else 
		{
#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
			rv = d->CreateTexture( ( UINT )img.width, ( UINT )img.height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &ppTexture, NULL );
			if ( ppTexture && rv == S_OK )
			{
#ifdef DOTA_HELPER_LOG
				AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
				//D3DXCreateTextureFromFileA( d, "qergqerg.png", &ppTexture );
				D3DLOCKED_RECT rect;
				rv = ppTexture->LockRect( 0, &rect, 0, 0 );
				if ( rv == S_OK )
				{
					unsigned char* dest = static_cast< unsigned char* >( rect.pBits );
					memcpy( dest, img.img.buf, ( size_t )( img.width * img.height * 4 ) );
					ppTexture->UnlockRect( 0 );
					D3DXFilterTexture_org( ppTexture, NULL, D3DX_DEFAULT, D3DX_DEFAULT );
					img.textureaddr = ppTexture;
				}
			}
		}
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		/*	if ( img.size_x > 0.0f && img.size_y > 0.0f )
			{

			}
			else
			{*/
		if ( img.MoveTime1 )
		{
			float lenx = abs( img.overlay_x - img.overlay_x2 );
			float leny = abs( img.overlay_y - img.overlay_y2 );

			if ( lenx > 0.004 )
			{
				if ( img.overlay_x > img.overlay_x2 )
					img.overlay_x -= 0.002;
				else if ( img.overlay_x < img.overlay_x2 )
					img.overlay_x += 0.002;
			}

			if ( leny > 0.004 )
			{
				if ( img.overlay_y > img.overlay_y2 )
					img.overlay_y -= 0.002;
				else if ( img.overlay_y < img.overlay_y2 )
					img.overlay_y += 0.002;
			}

			DWORD newTickImg = GetTickCount( ) - img.StartTimer;
			if ( newTickImg > img.MoveTime1 )
				img.MoveTime1 = 0;
			else
				img.MoveTime1 -= newTickImg;

			img.StartTimer = GetTickCount( );
		}
		else if ( img.SleepTime )
		{
			DWORD newTickImg = GetTickCount( ) - img.StartTimer;
			if ( newTickImg > img.SleepTime )
				img.SleepTime = 0;
			else
				img.SleepTime -= newTickImg;

			img.StartTimer = GetTickCount( );
		}
		else if ( img.MoveTime2 )
		{

			float lenx = abs( img.overlay_x - img.overlay_x0 );
			float leny = abs( img.overlay_y - img.overlay_y0 );

			if ( lenx > 0.004 )
			{
				if ( img.overlay_x > img.overlay_x0 )
					img.overlay_x -= 0.002;
				else if ( img.overlay_x < img.overlay_x0 )
					img.overlay_x += 0.002;
			}

			if ( leny > 0.004 )
			{
				if ( img.overlay_y > img.overlay_y0 )
					img.overlay_y -= 0.002;
				else if ( img.overlay_y < img.overlay_y0 )
					img.overlay_y += 0.002;
			}



			DWORD newTickImg = GetTickCount( ) - img.StartTimer;
			if ( newTickImg > img.MoveTime2 )
				img.MoveTime2 = 0;
			else
				img.MoveTime2 -= newTickImg;

			img.StartTimer = GetTickCount( );
		}
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		if ( ppTexture && rv == S_OK )
		{
			DrawImage( pSprite, ppTexture, ( float )img.width, ( float )img.height, *GetWindowXoffset * img.overlay_x, *GetWindowYoffset * img.overlay_y );
		}
		//}
		//ppTexture->Release( );
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	}
	pSprite->End( );
	pSprite->Release( );
	pSprite = NULL;
}


typedef HRESULT( __fastcall * EndScene_dx9_p )( int GlobalWc3Data );
EndScene_dx9_p EndScene_dx9_org;
EndScene_dx9_p EndScene_dx9_ptr;

HRESULT __fastcall EndScene_dx9_my( int GlobalWc3Data )
{
	LPDIRECT3DDEVICE9 d = *( IDirect3DDevice9** )( GlobalWc3Data + 1412 );

	if ( d != NULL )
	{
		deviceglobal = d;
		/*if ( GetVTableFunction( ( PDWORD* )d, 82 ) != ( PBYTE )DrawIndexedPrimitiveDx9 )
		{
			DrawIndexedPrimitiveDx9_org = ( DrawIndexedPrimitiveDx9p )HookVTableFunction( ( PDWORD* )d, ( PBYTE )DrawIndexedPrimitiveDx9, 82 );
		}*/
	}
	OverlayDrawed = FALSE;
	HRESULT retval = d->EndScene( );
	return retval;
}

void Uninitd3d9Hook( BOOL cleartextures )
{
	if ( EndScene_dx9_org )
	{
		MH_DisableHook( EndScene_dx9_org );
		if ( cleartextures )
		{
			for ( auto & img : ListOfRawImages )
			{
				if ( img.textureaddr )
				{
					IDirect3DTexture9 * ppTexture = ( IDirect3DTexture9 * )img.textureaddr;
					ppTexture->Release( );
					ppTexture = NULL;
					img.textureaddr = NULL;
					img.ingame = FALSE;
					img.used_for_overlay = FALSE;
				}
			}
		}
	}
}

void Initd3d9Hook( )
{
	char loadanyd3dx[ 20 ];
	HMODULE d3d9_43 = NULL;
	for ( int i = 43; i > 20; i-- )
	{
		memset( loadanyd3dx, 0, 20 );
		sprintf_s( loadanyd3dx, "d3dx9_%i.dll", i );
		d3d9_43 = LoadLibraryA( loadanyd3dx );
		if ( d3d9_43 != NULL )
			break;
	}
	if ( !d3d9_43 )
	{
		return;
	}
	D3D9CreateSprite_org = ( D3DXCreateSprite_p )GetProcAddress( d3d9_43, "D3DXCreateSprite" );
	D3DXFilterTexture_org = ( D3DXFilterTexture_p )GetProcAddress( d3d9_43, "D3DXFilterTexture" );
	if ( !D3D9CreateSprite_org || !D3DXFilterTexture_org )
	{
		return;
	}
	EndScene_dx9_org = ( EndScene_dx9_p )( GameDll + 0x0ECFF0 );
	MH_CreateHook( EndScene_dx9_org, &EndScene_dx9_my, reinterpret_cast< void** >( &EndScene_dx9_ptr ) );
	MH_EnableHook( EndScene_dx9_org );
}
