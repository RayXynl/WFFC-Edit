#pragma once

#include <d3d11.h> 
#include <SimpleMath.h>  
#include "InputCommands.h"
#include "StepTimer.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Camera
{
public:
	Camera();
	~Camera();

	void Tick(InputCommands* Input);
	void Update(DX::StepTimer const& timer);
	void Rotate();

	void FocusOnObject(const DirectX::SimpleMath::Vector3 objectPos, float distance);

	DirectX::SimpleMath::Matrix GetViewMatrix() { return m_view; };
	DirectX::SimpleMath::Vector3 GetPosition() { return m_camPosition; };
private:
	//functionality
	float								m_movespeed;

	//camera
	DirectX::SimpleMath::Vector3		m_camPosition;
	DirectX::SimpleMath::Vector3		m_camOrientation;
	DirectX::SimpleMath::Vector3		m_camLookAt;
	DirectX::SimpleMath::Vector3		m_camLookDirection;
	DirectX::SimpleMath::Vector3		m_camRight;
	DirectX::SimpleMath::Vector3		m_camUp;


	DirectX::SimpleMath::Vector3		m_orbitFocal;

	float								m_camRotRate;
	float 								m_orbitDistance;	
	bool								m_doOnce;
	bool								m_orbitMode;	

	DirectX::SimpleMath::Matrix         m_view;


	POINT								m_prevMousePos;
	POINT								m_currMousePos;

	InputCommands						m_InputCommands;
};

