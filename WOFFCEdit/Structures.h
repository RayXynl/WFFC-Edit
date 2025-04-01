#pragma once
#include <DirectXMath.h> 
#include <d3d11.h> 
#include <SimpleMath.h>  
#include "DisplayObject.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

struct DObjectState
{
	DisplayObject* m_objPtr;  
	float m_posX, m_posY, m_posZ;
	float m_rotX, m_rotY, m_rotZ;
	float m_scaleX, m_scaleY, m_scaleZ;
	
	DObjectState(DisplayObject* objptr, XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale)
	{
		m_objPtr = objptr;
		m_posX = pos.x, m_posY = pos.y, m_posZ = pos.z;
		m_rotX = rot.x, m_rotY = rot.y, m_rotZ = rot.z;
		m_scaleX = scale.x, m_scaleY = scale.y, m_scaleZ = scale.z;
	}

	bool operator==(const DObjectState& comparison) const {

		return (m_objPtr == comparison.m_objPtr &&
			m_posX == comparison.m_posX		&& m_posY == comparison.m_posY		&& m_posZ == comparison.m_posZ &&
			m_rotX == comparison.m_rotX		&& m_rotY == comparison.m_rotY		&& m_rotZ == comparison.m_rotZ &&
			m_scaleX == comparison.m_scaleX && m_scaleY == comparison.m_scaleY	&& m_scaleZ == comparison.m_scaleZ);
	}
	bool operator!=(const DObjectState& comparison) const {

		return (m_objPtr != comparison.m_objPtr &&
			m_posX != comparison.m_posX && m_posY != comparison.m_posY && m_posZ != comparison.m_posZ &&
			m_rotX != comparison.m_rotX && m_rotY != comparison.m_rotY && m_rotZ != comparison.m_rotZ &&
			m_scaleX != comparison.m_scaleX && m_scaleY != comparison.m_scaleY && m_scaleZ != comparison.m_scaleZ);
	}
};

enum TransformType
{
	PositionX	= 0,
	PositionY	= 1,
	PositionZ	= 2,
	RotationX	= 3,
	RotationY	= 4,
	RotationZ	= 5,
	ScaleX		= 6,
	ScaleY		= 7,
	ScaleZ		= 8
};