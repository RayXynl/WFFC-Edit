#pragma once
#include <DirectXMath.h> 
#include <d3d11.h> 
#include <SimpleMath.h>  
#include "DisplayObject.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

struct DObjectState
{
	DisplayObject	m_displayRef;
	SceneObject		m_sceneRef;
	int				m_ID;
	bool			m_isNewObject = false;

	DObjectState(const DisplayObject& displayRef, SceneObject sceneRef, int ID, bool newObject)
	{
		m_displayRef = displayRef;
		m_sceneRef = sceneRef;
		m_ID = ID;
		m_isNewObject = newObject;

	}

	DObjectState(const DisplayObject& displayRef, int ID, bool newObject)
	{
		m_displayRef = displayRef;

		m_ID = ID;
		m_isNewObject = newObject;
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