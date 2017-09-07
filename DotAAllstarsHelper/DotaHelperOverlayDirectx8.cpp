#include "Main.h"
#include <d3d8.h>
#include <d3dx8.h>
#pragma comment(lib,"d3dx8.lib")

// Thanks ENAleksey(http://xgm.guru/user/ENAleksey) for help 

IDirect3DDevice8* deviceglobal = NULL;

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

void DrawImage( ID3DXSprite* pSprite, IDirect3DTexture8* texture, float width, float height, float x, float y )
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

	pSprite->DrawTransform( texture, NULL, &matAll, 0xffffffff );
}

D3DMATERIAL8 oldmat;
void SetNewLightDx8( int id )
{
	if ( deviceglobal != NULL )
	{
		D3DMATERIAL8 mymat;

		mymat.Ambient.r = 1.0;
		mymat.Ambient.g = 1.0;
		mymat.Ambient.b = 1.0;
		mymat.Diffuse.r = 1.0;
		mymat.Diffuse.g = 1.0;
		mymat.Diffuse.b = 1.0;

		mymat.Specular.r = 1.0;
		mymat.Specular.g = 1.0;
		mymat.Specular.b = 1.0;

		deviceglobal->SetMaterial( &mymat );

	}
}

void SetOldLightDx8( int id )
{
	if ( deviceglobal != NULL )
	{

	}
}


typedef HRESULT( __stdcall *  DrawIndexedPrimitiveDx8p )( D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount );
DrawIndexedPrimitiveDx8p DrawIndexedPrimitiveDx8_org;


HRESULT DrawIndexedPrimitiveDx8( D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount )
{
	HRESULT retval;

	retval = DrawIndexedPrimitiveDx8_org( PrimitiveType, MinIndex, NumVertices, StartIndex, PrimitiveCount );

	return retval;
}


void DrawOverlayDx8( )
{
	HRESULT rv;
	IDirect3DDevice8 * d = deviceglobal;
	if ( !d || OverlayDrawed )
	{
		return;
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif


	ID3DXSprite* pSprite;
	rv = D3DXCreateSprite( d, &pSprite );
	if ( !pSprite || rv != S_OK )
		return;
	pSprite->Begin( );
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
				IDirect3DTexture8 * ppTexture = ( IDirect3DTexture8 * )img.textureaddr;
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
				IDirect3DTexture8 * ppTexture = ( IDirect3DTexture8 * )img.textureaddr;
				ppTexture->Release( );
				ppTexture = NULL;
				img.textureaddr = NULL;
			}
		}
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif

		IDirect3DTexture8 * ppTexture = NULL;
		if ( img.textureaddr )
		{
#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
			ppTexture = ( IDirect3DTexture8 * )img.textureaddr;
		}
		else
		{
#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
			rv = d->CreateTexture( ( UINT )img.width, ( UINT )img.height, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &ppTexture );
			if ( ppTexture && rv == S_OK)
			{
#ifdef DOTA_HELPER_LOG
				AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
				D3DLOCKED_RECT rect;

				rv = ppTexture->LockRect( 0, &rect, 0, 0 );
				if ( rv == S_OK )
				{
					unsigned char* dest = static_cast< unsigned char* >( rect.pBits );
					memcpy( dest, img.img.buf, ( size_t )( img.width * img.height * 4 ) );
					ppTexture->UnlockRect( 0 );
					D3DXFilterTexture( ppTexture, NULL, D3DX_DEFAULT, D3DX_DEFAULT );
					img.textureaddr = ppTexture;
#ifdef DOTA_HELPER_LOG
					AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
				}
			}

		}
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		/*if ( img.size_x > 0.0f && img.size_y > 0.0f )
		{
			DrawImage( d, pSprite, ppTexture, img.size_x * *GetWindowXoffset, img.size_y * *GetWindowYoffset, *GetWindowXoffset * img.overlay_x, *GetWindowYoffset * img.overlay_y );
		}
		else
		{*/
		/*

		Time1 = за сколько времени нужно


		*/

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
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		//}
	}
	pSprite->End( );
	pSprite->Release( );
	pSprite = NULL;

}


typedef HRESULT( __fastcall * EndScene_p )( int GlobalWc3Data );
EndScene_p EndScene_org;
EndScene_p EndScene_ptr;



HRESULT __fastcall EndScene_my( int GlobalWc3Data )
{
	IDirect3DDevice8 * d = *( IDirect3DDevice8** )( GlobalWc3Data + 1412 );
	if ( d != NULL )
	{
		deviceglobal = d;

	}
	OverlayDrawed = FALSE;
	HRESULT retval = deviceglobal->EndScene( );
	return retval;
}


void Uninitd3d8Hook( BOOL cleartextures )
{
	if ( EndScene_org )
	{
		MH_DisableHook( EndScene_org );
		if ( cleartextures )
		{
			for ( auto & img : ListOfRawImages )
			{
				if ( img.textureaddr )
				{
					IDirect3DTexture8 * ppTexture = ( IDirect3DTexture8 * )img.textureaddr;
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

void Initd3d8Hook( )
{
	EndScene_org = ( EndScene_p )( GameDll + 0x52F270 );
	MH_CreateHook( EndScene_org, &EndScene_my, reinterpret_cast< void** >( &EndScene_ptr ) );
	MH_EnableHook( EndScene_org );
}

