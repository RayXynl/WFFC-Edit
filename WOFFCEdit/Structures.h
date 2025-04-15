#pragma once
#include <DirectXMath.h> 
#include <d3d11.h> 
#include <SimpleMath.h>  
#include "DisplayObject.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

struct DObjectState
{
	DisplayObject m_displayRef;
	SceneObject m_sceneRef;
	int m_ID;
	bool m_isNewObject = false;

	DObjectState(const DisplayObject& displayRef, SceneObject sceneRef, int ID, bool newObject)
	{
		m_displayRef = displayRef;
		m_sceneRef = sceneRef;
		m_ID = ID;
		m_isNewObject = newObject;

	}

	// Constructor WITHOUT SceneObject
	DObjectState(const DisplayObject& displayRef, int ID, bool newObject)
	{
		m_displayRef = displayRef;

		m_ID = ID;
		m_isNewObject = newObject;
	}
	/*DisplayObject* m_objPtr;   //// change name
	DisplayObject m_objectData;
	SceneObject m_sceneObjref;
	int m_ID;
	float m_posX, m_posY, m_posZ;
	float m_rotX, m_rotY, m_rotZ;
	float m_scaleX, m_scaleY, m_scaleZ;
	bool m_isNewObject;

	DObjectState(DisplayObject* objptr, DisplayObject objData, int ID, XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, bool newFlag)
	{
		m_objPtr = objptr;
		m_objectData = objData;
		m_ID = ID;
		m_posX = pos.x, m_posY = pos.y, m_posZ = pos.z;
		m_rotX = rot.x, m_rotY = rot.y, m_rotZ = rot.z;
		m_scaleX = scale.x, m_scaleY = scale.y, m_scaleZ = scale.z;
		m_isNewObject = newFlag;
	}
	DObjectState(DisplayObject* objptr, DisplayObject objData, SceneObject sceneObjref, int ID, XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, bool newFlag)
	{
		m_objPtr = objptr;
		m_objectData = objData;
		m_sceneObjref = sceneObjref;
		m_ID = ID;
		m_posX = pos.x, m_posY = pos.y, m_posZ = pos.z;
		m_rotX = rot.x, m_rotY = rot.y, m_rotZ = rot.z;
		m_scaleX = scale.x, m_scaleY = scale.y, m_scaleZ = scale.z;
		m_isNewObject = newFlag;
	}*/

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