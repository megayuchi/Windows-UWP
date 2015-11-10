#pragma once

#include <math.h>
#include <windows.h>


struct MATRIX4;


struct VECTOR3
{
	float	x;
	float	y;
	float	z;

	inline	VECTOR3		operator +(const VECTOR3 &v);
	inline	VECTOR3		operator -(const VECTOR3 &v);
	inline	VECTOR3		operator *(const float	&f);
	inline	VECTOR3		operator /(const float	&f);
	inline	VECTOR3		operator *(const VECTOR3 &v);
	inline	BOOL		operator==( const VECTOR3& v);
	inline	BOOL		operator!=( const VECTOR3& v);
	inline	BOOL		NearZero( float fE);


	inline void			Set( float in_x, float in_y, float in_z );
	
/*
	VECTOR3		operator +(const VECTOR3 &v3);
	VECTOR3		operator -(const VECTOR3 &v3);
	VECTOR3		operator *(const VECTOR3 &v3);
	VECTOR3		operator /(const VECTOR3 &v3);

	VECTOR3		operator +(const float a);
	VECTOR3		operator -(const float a);
	VECTOR3		operator *(const float a);
	VECTOR3		operator /(const float a);

	void			operator +=(const VECTOR3 &v3);
	void			operator -=(const VECTOR3 &v3);
	void			operator *=(const VECTOR3 &v3);
	void			operator *=(const float a);
	void			operator /=(const float a);
	void			operator /=(const VECTOR3 &v3);
*/
};

inline void VECTOR3::Set( float in_x, float in_y, float in_z )
{
	x = in_x;
	y = in_y;
	z = in_z;
}
inline VECTOR3		VECTOR3::operator +(const VECTOR3 &v3)
{
	VECTOR3	result;
	result.x	=	this->x + v3.x;
	result.y	=	this->y + v3.y;
	result.z	=	this->z + v3.z;
	return	result;
}

inline VECTOR3		VECTOR3::operator -(const VECTOR3 &v3)
{
	VECTOR3		result;
	result.x	=	this->x -	v3.x;
	result.y	=	this->y	-	v3.y;
	result.z	=	this->z	-	v3.z;
	return	result;
}

inline VECTOR3		VECTOR3::operator *(const float	&f)
{
	VECTOR3		r;
	r.x	=	this->x	*	f;
	r.y	=	this->y *	f;
	r.z	=	this->z	*	f;
	return	r;
}

inline VECTOR3		VECTOR3::operator /(const float	&f)
{
	VECTOR3		r;
	r.x	=	this->x / f;
	r.y	=	this->y / f;
	r.z	=	this->z / f;
	return	r;
}

inline VECTOR3		VECTOR3::operator *(const VECTOR3 &v)
{
	VECTOR3		r;
	r.x	=	this->x * v.x;
	r.y =	this->y * v.y;
	r.z	=	this->z * v.z;
	return		r;
}

inline	BOOL		VECTOR3::operator==( const VECTOR3& v)
{
	BOOL	bResult;
	if( this->x == v.x && this->y == v.y && this->z == v.z)
		bResult	=	TRUE;
	else	
		bResult	=	FALSE;

	return	bResult;
}
inline	BOOL		VECTOR3::operator!=( const VECTOR3& v)
{
	BOOL	bResult;
	if( this->x != v.x || this->y != v.y || this->z != v.z)
		bResult	=	TRUE;
	else	
		bResult	=	FALSE;

	return	bResult;
}


inline BOOL VECTOR3::NearZero( float fE)
{
 if( this->x > -fE && this->x < fE && this->y > -fE && this->y < fE && this->z > -fE && this->z < fE)
  return TRUE;
 else
  return FALSE;
}


struct MATRIX4
{
	float	_11;
	float	_12;
	float	_13;
	float	_14;

	float	_21;
	float	_22;
	float	_23;
	float	_24;

	float	_31;
	float	_32;
	float	_33;
	float	_34;

	float	_41;
	float	_42;
	float	_43;
	float	_44;

};
