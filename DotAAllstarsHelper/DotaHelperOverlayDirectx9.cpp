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


typedef HRESULT (WINAPI *
D3DXFilterTexture_p )(
	LPDIRECT3DBASETEXTURE9    pBaseTexture,
	CONST PALETTEENTRY*       pPalette,
	UINT                      SrcLevel,
	DWORD                     Filter );

D3DXFilterTexture_p D3DXFilterTexture_org;

void DrawImage( ID3DXSprite* pSprite, IDirect3DTexture9* texture, float width, float height, float x, float y )
{
	D3DXMATRIX matAll;
	float scalex = *GetWindowXoffset / DesktopScreen_Width;
	float scaley = *GetWindowYoffset / DesktopScreen_Height;
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

void DrawOverlayDx9( )
{
	IDirect3DDevice9 * d = deviceglobal;
	if ( !d || OverlayDrawed )
	{
		return;
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif

	ID3DXSprite* pSprite;
	D3D9CreateSprite_org( d, &pSprite );

	pSprite->Begin( D3DXSPRITE_ALPHABLEND );
	d->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	d->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	d->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );



	for ( auto & img : ListOfRawImages )
	{
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


		IDirect3DTexture9 * ppTexture = ( IDirect3DTexture9 * )img.textureaddr;
		if ( !ppTexture )
		{
			d->CreateTexture( ( UINT )img.width, ( UINT )img.height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &ppTexture, NULL );
			//D3DXCreateTextureFromFileA( d, "qergqerg.png", &ppTexture );
			D3DLOCKED_RECT rect;
			ppTexture->LockRect( 0, &rect, 0, 0 );
			unsigned char* dest = static_cast< unsigned char* >( rect.pBits );
			memcpy( dest, img.img.buf, ( size_t )( img.width * img.height * 4 ) );
			ppTexture->UnlockRect( 0 );
			D3DXFilterTexture_org( ppTexture, NULL, D3DX_DEFAULT, D3DX_DEFAULT );
			img.textureaddr = ppTexture;
		}
		/*	if ( img.size_x > 0.0f && img.size_y > 0.0f )
			{

			}
			else
			{*/
		DrawImage( pSprite, ppTexture, ( float )img.width, ( float )img.height, *GetWindowXoffset * img.overlay_x, *GetWindowYoffset * img.overlay_y );
		//}
		//ppTexture->Release( );

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
	IDirect3DDevice9 * d = *( IDirect3DDevice9** )( GlobalWc3Data + 1412 );
	deviceglobal = d;
	OverlayDrawed = FALSE;
	HRESULT retval = d->EndScene( );
	return retval;
}

void Uninitd3d9Hook( )
{
	MH_DisableHook( EndScene_dx9_org );
	//if ( Reset_dx9_org )
	//	MH_DisableHook( Reset_dx9_org );
	for ( auto & img : ListOfRawImages )
	{
		if ( img.textureaddr )
		{
			IDirect3DTexture9 * ppTexture = ( IDirect3DTexture9 * )img.textureaddr;
			ppTexture->Release( );
			ppTexture = NULL;
			img.textureaddr = NULL;
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
