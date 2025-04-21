//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "DisplayObject.h"
#include <string>
#include <algorithm>
#include <iostream>
#include <string>
#include <stack>
#include <Windows.h>
using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game()

{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
	//m_displayList->clear();
	
	//initial Settings
	//modes
	m_grid = false;


	HRESULT rs;
	rs = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"highlight.png", nullptr, &m_highlightTexture);
}

Game::~Game()
{

#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height, std::vector<DisplayObject>* displayList, std::stack<DObjectState>* redoStack, std::stack<DObjectState>* undoStack)
{
	m_displayList = displayList;
	m_undoStack = undoStack;
	m_redoStack = redoStack;
    m_gamePad = std::make_unique<GamePad>();

    m_keyboard = std::make_unique<Keyboard>();

    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	GetClientRect(window, &m_ScreenDimensions);

#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    m_effect2->Play();
#endif
}

void Game::SetGridState(bool state)
{
	m_grid = state;
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick(InputCommands *Input)
{
	//copy over the input commands so we have a local version to use elsewhere.
	m_InputCommands = *Input;
    m_timer.Tick([&]()
    {
        Update(m_timer);
		m_Camera.Tick(&m_InputCommands);
		m_Camera.Update(m_timer);
    });

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
	
    m_batchEffect->SetView(m_Camera.GetViewMatrix());
    m_batchEffect->SetWorld(Matrix::Identity);
	m_displayChunk.m_terrainEffect->SetView(m_Camera.GetViewMatrix());
	m_displayChunk.m_terrainEffect->SetWorld(Matrix::Identity);

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }
#endif

   
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

	if (m_grid)
	{
		// Draw procedurally generated dynamic grid
		const XMVECTORF32 xaxis = { 512.f, 0.f, 0.f };
		const XMVECTORF32 yaxis = { 0.f, 0.f, 512.f };
		DrawGrid(xaxis, yaxis, g_XMZero, 512, 512, Colors::Gray);
	}
	//CAMERA POSITION ON HUD
	m_sprites->Begin();
	WCHAR   Buffer[256];

	//RENDER OBJECTS FROM SCENEGRAPH
	int numRenderObjects = m_displayList->size();
	for (int i = 0; i < numRenderObjects; i++)
	{
		m_deviceResources->PIXBeginEvent(L"Draw model");
		const XMVECTORF32 scale = { m_displayList->at(i).m_scale.x, m_displayList->at(i).m_scale.y, m_displayList->at(i).m_scale.z };
		const XMVECTORF32 translate = { m_displayList->at(i).m_position.x, m_displayList->at(i).m_position.y, m_displayList->at(i).m_position.z };

		//convert degrees into radians for rotation matrix
		XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(m_displayList->at(i).m_orientation.y *3.1415 / 180,
															m_displayList->at(i).m_orientation.x *3.1415 / 180,
															m_displayList->at(i).m_orientation.z *3.1415 / 180);

		XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

		m_displayList->at(i).m_model->Draw(context, *m_states, local, m_Camera.GetViewMatrix(), m_projection, false);

		bool selected = std::find(selectedID.begin(), selectedID.end(), m_displayList->at(i).m_ID) != selectedID.end();
		//////// NEW Feature comment on THIS ||||||||||||||||||||||||||||||||
		if (selected)
		{
			XMVECTORF32 highlightScale = {
				m_displayList->at(i).m_scale.x * 1.06f,
				m_displayList->at(i).m_scale.y * 1.06f,
				m_displayList->at(i).m_scale.z * 1.06f
			};

			
			XMMATRIX highlightLocal = m_world * XMMatrixTransformation(
				g_XMZero, Quaternion::Identity, highlightScale, g_XMZero, rotate, translate
			);


			m_displayList->at(i).m_model->UpdateEffects([&](IEffect* effect)
				{
					auto basicEffect = dynamic_cast<BasicEffect*>(effect);
					if (basicEffect)
					{
					
						basicEffect->SetDiffuseColor(Colors::White);
						basicEffect->SetEmissiveColor(Colors::White);
					}
				});

		
			m_displayList->at(i).m_model->Draw(context, *m_states, highlightLocal, m_Camera.GetViewMatrix(), m_projection, false);

		}
		else
		{
		
			m_displayList->at(i).m_model->UpdateEffects([&](IEffect* effect)
				{
					auto basicEffect = dynamic_cast<BasicEffect*>(effect);
					if (basicEffect)
					{
						basicEffect->SetDiffuseColor(Colors::White);
						basicEffect->SetEmissiveColor(Colors::Black);
					}
				});
		}
		m_deviceResources->PIXEndEvent();
	}
    m_deviceResources->PIXEndEvent();

	//RENDER TERRAIN
	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(),0);
	context->RSSetState(m_states->CullNone());
//	context->RSSetState(m_states->Wireframe());		//uncomment for wireframe

	//Render the batch,  This is handled in the Display chunk becuase it has the potential to get complex
	m_displayChunk.RenderBatch(m_deviceResources);

	std::wstring var = L"Cam X: " + std::to_wstring(m_Camera.GetPosition().x) + L" Cam Z: " + std::to_wstring(m_Camera.GetPosition().z);
	m_font->DrawString(m_sprites.get(), var.c_str(), XMFLOAT2(100, 10), Colors::Yellow);
	m_sprites->End();


    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetBackBufferRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

void XM_CALLCONV Game::DrawGrid(FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color)
{
    m_deviceResources->PIXBeginEvent(L"Draw grid");

    auto context = m_deviceResources->GetD3DDeviceContext();
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    context->RSSetState(m_states->CullCounterClockwise());

    m_batchEffect->Apply(context);

    context->IASetInputLayout(m_batchInputLayout.Get());

    m_batch->Begin();

    xdivs = std::max<size_t>(1, xdivs);
    ydivs = std::max<size_t>(1, ydivs);

    for (size_t i = 0; i <= xdivs; ++i)
    {
        float fPercent = float(i) / float(xdivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, yAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, yAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= ydivs; i++)
    {
        float fPercent = float(i) / float(ydivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    m_batch->End();

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

void Game::BuildDisplayList(std::vector<SceneObject> * SceneGraph)
{
	auto device = m_deviceResources->GetD3DDevice();
	auto devicecontext = m_deviceResources->GetD3DDeviceContext();

	if (!m_displayList->empty())		//is the vector empty
	{
		m_displayList->clear();		//if not, empty it
	}
	std::sort(SceneGraph->begin(), SceneGraph->end(),
		[](const SceneObject& a, const SceneObject& b) {
			return a.ID < b.ID;
		});
	//for every item in the scenegraph
	int numObjects = SceneGraph->size();
	for (int i = 0; i < numObjects; i++)
	{
		
		//create a temp display object that we will populate then append to the display list.
		DisplayObject newDisplayObject;
		
		//load model
		std::wstring modelwstr = StringToWCHART(SceneGraph->at(i).model_path);							//convect string to Wchar
		newDisplayObject.m_model = Model::CreateFromCMO(device, modelwstr.c_str(), *m_fxFactory, true);	//get DXSDK to load model "False" for LH coordinate system (maya)

		//Load Texture
		std::wstring texturewstr = StringToWCHART(SceneGraph->at(i).tex_diffuse_path);								//convect string to Wchar
		HRESULT rs;
		rs = CreateDDSTextureFromFile(device, texturewstr.c_str(), nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource

		//if texture fails.  load error default
		if (rs)
		{
			CreateDDSTextureFromFile(device, L"database/data/Error.dds", nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource
		}


		//apply new texture to models effect
		newDisplayObject.m_model->UpdateEffects([&](IEffect* effect) //This uses a Lambda function,  if you dont understand it: Look it up.
		{	
			auto lights = dynamic_cast<BasicEffect*>(effect);
			if (lights)
			{
				lights->SetTexture(newDisplayObject.m_texture_diffuse);			
			}
		});

		newDisplayObject.m_ID = SceneGraph->at(i).ID;

		//set position
		newDisplayObject.m_position.x = SceneGraph->at(i).posX;
		newDisplayObject.m_position.y = SceneGraph->at(i).posY;
		newDisplayObject.m_position.z = SceneGraph->at(i).posZ;
		
		//setorientation
		newDisplayObject.m_orientation.x = SceneGraph->at(i).rotX;
		newDisplayObject.m_orientation.y = SceneGraph->at(i).rotY;
		newDisplayObject.m_orientation.z = SceneGraph->at(i).rotZ;

		//set scale
		newDisplayObject.m_scale.x = SceneGraph->at(i).scaX;
		newDisplayObject.m_scale.y = SceneGraph->at(i).scaY;
		newDisplayObject.m_scale.z = SceneGraph->at(i).scaZ;

		//set wireframe / render flags
		newDisplayObject.m_render		= SceneGraph->at(i).editor_render;
		newDisplayObject.m_wireframe	= SceneGraph->at(i).editor_wireframe;

		newDisplayObject.m_light_type		= SceneGraph->at(i).light_type;
		newDisplayObject.m_light_diffuse_r	= SceneGraph->at(i).light_diffuse_r;
		newDisplayObject.m_light_diffuse_g	= SceneGraph->at(i).light_diffuse_g;
		newDisplayObject.m_light_diffuse_b	= SceneGraph->at(i).light_diffuse_b;
		newDisplayObject.m_light_specular_r = SceneGraph->at(i).light_specular_r;
		newDisplayObject.m_light_specular_g = SceneGraph->at(i).light_specular_g;
		newDisplayObject.m_light_specular_b = SceneGraph->at(i).light_specular_b;
		newDisplayObject.m_light_spot_cutoff = SceneGraph->at(i).light_spot_cutoff;
		newDisplayObject.m_light_constant	= SceneGraph->at(i).light_constant;
		newDisplayObject.m_light_linear		= SceneGraph->at(i).light_linear;
		newDisplayObject.m_light_quadratic	= SceneGraph->at(i).light_quadratic;
		
		m_displayList->push_back(newDisplayObject);
		
	}
		
		
		
}

void Game::BuildDisplayObject(SceneObject* SceneObject)
{
	auto device = m_deviceResources->GetD3DDevice();
	auto devicecontext = m_deviceResources->GetD3DDeviceContext();

	//create a temp display object that we will populate then append to the display list.
	DisplayObject newDisplayObject;

	//load model
	std::wstring modelwstr = StringToWCHART(SceneObject->model_path);							//convect string to Wchar
	newDisplayObject.m_model = Model::CreateFromCMO(device, modelwstr.c_str(), *m_fxFactory, true);	//get DXSDK to load model "False" for LH coordinate system (maya)

	//Load Texture
	std::wstring texturewstr = StringToWCHART(SceneObject->tex_diffuse_path);								//convect string to Wchar
	HRESULT rs;
	rs = CreateDDSTextureFromFile(device, texturewstr.c_str(), nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource

	//if texture fails.  load error default
	if (rs)
	{
		CreateDDSTextureFromFile(device, L"database/data/Error.dds", nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource
	}

	//apply new texture to models effect
	newDisplayObject.m_model->UpdateEffects([&](IEffect* effect) //This uses a Lambda function,  if you dont understand it: Look it up.
		{
			auto lights = dynamic_cast<BasicEffect*>(effect);
			if (lights)
			{
				lights->SetTexture(newDisplayObject.m_texture_diffuse);
			}
		});

	newDisplayObject.m_ID = SceneObject->ID;

	//set position
	newDisplayObject.m_position.x = SceneObject->posX;
	newDisplayObject.m_position.y = SceneObject->posY;
	newDisplayObject.m_position.z = SceneObject->posZ;

	//setorientation
	newDisplayObject.m_orientation.x = SceneObject->rotX;
	newDisplayObject.m_orientation.y = SceneObject->rotY;
	newDisplayObject.m_orientation.z = SceneObject->rotZ;

	//set scale
	newDisplayObject.m_scale.x		= SceneObject->scaX;
	newDisplayObject.m_scale.y		= SceneObject->scaY;
	newDisplayObject.m_scale.z		= SceneObject->scaZ;

	//set wireframe / render flags
	newDisplayObject.m_render		= SceneObject->editor_render;
	newDisplayObject.m_wireframe	= SceneObject->editor_wireframe;

	newDisplayObject.m_light_type			= SceneObject->light_type;
	newDisplayObject.m_light_diffuse_r		= SceneObject->light_diffuse_r;
	newDisplayObject.m_light_diffuse_g		= SceneObject->light_diffuse_g;
	newDisplayObject.m_light_diffuse_b		= SceneObject->light_diffuse_b;
	newDisplayObject.m_light_specular_r		= SceneObject->light_specular_r;
	newDisplayObject.m_light_specular_g		= SceneObject->light_specular_g;
	newDisplayObject.m_light_specular_b		= SceneObject->light_specular_b;
	newDisplayObject.m_light_spot_cutoff	= SceneObject->light_spot_cutoff;
	newDisplayObject.m_light_constant		= SceneObject->light_constant;
	newDisplayObject.m_light_linear			= SceneObject->light_linear;
	newDisplayObject.m_light_quadratic		= SceneObject->light_quadratic;

	m_displayList->push_back(newDisplayObject);
}

void Game::BuildDisplayChunk(ChunkObject * SceneChunk)
{
	//populate our local DISPLAYCHUNK with all the chunk info we need from the object stored in toolmain
	//which, to be honest, is almost all of it. Its mostly rendering related info so...
	m_displayChunk.PopulateChunkData(SceneChunk);		//migrate chunk data
	m_displayChunk.LoadHeightMap(m_deviceResources);
	m_displayChunk.m_terrainEffect->SetProjection(m_projection);
	m_displayChunk.InitialiseBatch();
}

void Game::SaveDisplayChunk(ChunkObject * SceneChunk)
{
	m_displayChunk.SaveHeightMap();			//save heightmap to file.
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif


#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);

    m_fxFactory = std::make_unique<EffectFactory>(device);
	m_fxFactory->SetDirectory(L"database/data/"); //fx Factory will look in the database directory
	m_fxFactory->SetSharing(false);	//we must set this to false otherwise it will share effects based on the initial tex loaded (When the model loads) rather than what we will change them to.

    m_sprites = std::make_unique<SpriteBatch>(context);

    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

    m_batchEffect = std::make_unique<BasicEffect>(device);
    m_batchEffect->SetVertexColorEnabled(true);

    {
        void const* shaderByteCode;
        size_t byteCodeLength;

        m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        DX::ThrowIfFailed(
            device->CreateInputLayout(VertexPositionColor::InputElements,
                VertexPositionColor::InputElementCount,
                shaderByteCode, byteCodeLength,
                m_batchInputLayout.ReleaseAndGetAddressOf())
        );
    }

    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");

//    m_shape = GeometricPrimitive::CreateTeapot(context, 4.f, 8);

    // SDKMESH has to use clockwise winding with right-handed coordinates, so textures are flipped in U
    m_model = Model::CreateFromSDKMESH(device, L"tiny.sdkmesh", *m_fxFactory);
	

    // Load textures
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"seafloor.dds", nullptr, m_texture1.ReleaseAndGetAddressOf())
    );

    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"windowslogo.dds", nullptr, m_texture2.ReleaseAndGetAddressOf())
    );

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        1000.0f
    );

    m_batchEffect->SetProjection(m_projection);
	
}

void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_batch.reset();
    m_batchEffect.reset();
    m_font.reset();
    m_shape.reset();
    m_model.reset();
    m_texture1.Reset();
    m_texture2.Reset();
    m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion

std::wstring StringToWCHART(std::string s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

std::vector<int> Game::MousePicking(bool multiSelect)
{
	int selected = -1;
	float pickedDistance = 0;

	//setup near and far planes of frustum with mouse X and mouse y passed down from Toolmain. 
		//they may look the same but note, the difference in Z
	const XMVECTOR nearSource = XMVectorSet(m_InputCommands.mouse_X, m_InputCommands.mouse_Y, 0.0f, 1.0f);
	const XMVECTOR farSource = XMVectorSet(m_InputCommands.mouse_X, m_InputCommands.mouse_Y, 1.0f, 1.0f);

	//Loop through entire display list of objects and pick with each in turn. 
	for (int i = 0; i < m_displayList->size(); i++)
	{
		//Get the scale factor and translation of the object
		const XMVECTORF32 scale = { m_displayList->at(i).m_scale.x,		m_displayList->at(i).m_scale.y,		m_displayList->at(i).m_scale.z };
		const XMVECTORF32 translate = { m_displayList->at(i).m_position.x,		m_displayList->at(i).m_position.y,	m_displayList->at(i).m_position.z };

		//convert euler angles into a quaternion for the rotation of the object
		XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(m_displayList->at(i).m_orientation.y * 3.1415 / 180, m_displayList->at(i).m_orientation.x * 3.1415 / 180,
			m_displayList->at(i).m_orientation.z * 3.1415 / 180);

		//create set the matrix of the selected object in the world based on the translation, scale and rotation.
		XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

		//Unproject the points on the near and far plane, with respect to the matrix we just created.
		XMVECTOR nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_Camera.GetViewMatrix(), local);

		XMVECTOR farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_Camera.GetViewMatrix(), local);

		//turn the transformed points into our picking vector. 
		XMVECTOR pickingVector = farPoint - nearPoint;
		pickingVector = XMVector3Normalize(pickingVector);

		float closestDist = FLT_MAX;
	
		for (int y = 0; y < m_displayList->at(i).m_model.get()->meshes.size(); y++)
		{
			if (m_displayList->at(i).m_model.get()->meshes[y]->boundingBox.Intersects(nearPoint, pickingVector, pickedDistance))
			{
				if (pickedDistance < closestDist)
				{
					closestDist = pickedDistance;
					selected = m_displayList->at(i).m_ID;
				}
			}
		}
	}

	if (selected != -1)									// if selected object is valid
	{
		if (!selectedID.empty())						// check if selectedID vector is empty 
		{
			if (selectedID[0] == -1)					// if first element is invalid object
				selectedID.clear();						// clear vector 
		}
		if (multiSelect)								// if multiple objects are being selected
		{
			auto it = std::find(selectedID.begin(), selectedID.end(), selected);	// check if current selected object has already been selected
			if (it == selectedID.end())												// if object has not been selected
				selectedID.push_back(selected);										// add to selected vector
			else if (it != selectedID.end())										// if object has alreayd been selected
				selectedID.erase(it);												// remove from selected vector	
			else
				selectedID.clear();													// clear vector
		}
		else
		{
			selectedID.clear();														// clear vector
			selectedID.push_back(selected);											// push selected object ID
		}
	}
	else
	{
		selectedID.clear();															// clear selected ID vector
		selectedID.push_back(-1);													// push -1 as no object selection has been made
	}

	return selectedID;																// return selected object IDs vector
}

void Game::MoveObjects(std::vector<int>& selectedIDs)
{
	if (selectedIDs.empty())				// If selectedID vector is empty
		return;								// return from function

	if (selectedIDs[0] <= -1)				// if first element in selectdID vector is invalid selection
		return;								// return from function	
		
	if (m_displayList->empty())				// if display list is empty
		return;								// return from function
	
	GetSelectedObject(selectedIDs);			// Get selected objects

	for (int i = 0; i < m_selectedObjects.size(); i++)					// Loop through selected objects
	{
		DisplayObject* object = m_selectedObjects[i];					// get selected object

		if (m_InputCommands.forward)	object->m_position.z += 0.1f;	// move object forward
		if (m_InputCommands.back)		object->m_position.z -= 0.1f;	// move object back
		if (m_InputCommands.left)		object->m_position.x += 0.1f;	// move object left
		if (m_InputCommands.right)		object->m_position.x -= 0.1f;	// move object right
		if (m_InputCommands.up)			object->m_position.y += 0.1f;	// move object up
		if (m_InputCommands.down)		object->m_position.y -= 0.1f;	// move object down
	}
}

void Game::RotateObjects(std::vector<int>& selectedIDs)
{
	if (selectedIDs.empty())				// If selectedID vector is empty
		return;								// return from function

	if (selectedIDs[0] <= -1)				// if first element in selectdID vector is invalid selection
		return;								// return from function	

	if (m_displayList->empty())				// if display list is empty
		return;								// return from function

	GetSelectedObject(selectedIDs);			// Get selected objects

	for (int i = 0; i < m_selectedObjects.size(); i++) // Loop through selected objects
	{
		DisplayObject* object = m_selectedObjects[i];		// get current object

		if (m_InputCommands.forward)	object->m_orientation.z -= 0.5f;	// rotate object around z axis
		if (m_InputCommands.back)		object->m_orientation.z += 0.5f;	// rotate object around z axis
		if (m_InputCommands.left)		object->m_orientation.x -= 0.5f;	// rotate object around x axis
		if (m_InputCommands.right)		object->m_orientation.x += 0.5f;	// rotate object around x axis
		if (m_InputCommands.up)			object->m_orientation.y -= 0.5f;	// rotate object around y axis
		if (m_InputCommands.down)		object->m_orientation.y += 0.5f;	// rotate object around y axis
	}
}

void Game::ScaleObjects(std::vector<int>& selectedIDs)
{
	if (selectedIDs.empty())				// If selectedID vector is empty
		return;								// return from function

	if (selectedIDs[0] <= -1)				// if first element in selectdID vector is invalid selection
		return;								// return from function	

	if (m_displayList->empty())				// if display list is empty
		return;								// return from function


	GetSelectedObject(selectedIDs);			// Get selected objects

	for (int i = 0; i < m_selectedObjects.size(); i++)		// Loop through selected objects
	{
		DisplayObject* object = m_selectedObjects[i];		// Get current object

		if (object->m_scale.x < 0.0f)	object->m_scale.x *= -1.f; // prevent negative scale
		if (object->m_scale.y < 0.0f)	object->m_scale.y *= -1.f; // prevent negative scale
		if (object->m_scale.z < 0.0f)	object->m_scale.z *= -1.f; // prevent negative scale

		if (m_InputCommands.forward)	object->m_scale.z += 0.1f;	// scale object in z plane
		if (m_InputCommands.back)		object->m_scale.z -= 0.1f;	// scale object in z plane
		if (m_InputCommands.left)		object->m_scale.x -= 0.1f;	// scale object in x plane
		if (m_InputCommands.right)		object->m_scale.x += 0.1f;	// scale object in x plane
		if (m_InputCommands.up)			object->m_scale.y += 0.1f;	// scale object in y plane
		if (m_InputCommands.down)		object->m_scale.y -= 0.1f;	// scale object in y plane
	}
}

void Game::Undo(std::stack<DObjectState>* undoStack, std::stack<DObjectState>* redoStack, std::vector<SceneObject>& sceneGraph)
{
	if (!undoStack->empty())						// if undo stack is not mempty
	{
		for (int i = 0; i < 2; i++)
		{
			DObjectState& topState = undoStack->top();		// Get last state to undo 

			if (topState.m_isNewObject)						// Check if object is new
			{
				auto sceneObj = std::find_if(sceneGraph.begin(), sceneGraph.end(),		// find object in scene graph
					[&](const SceneObject& obj) { return obj.ID == topState.m_ID; });

				if (sceneObj != sceneGraph.end())			// if object is in scene graph
				{
					if (i == 0)
						sceneGraph.erase(sceneObj);			// remove object from scene graph
					i++;
				}
				else                                        // else create new object and to scene graph
				{
					SceneObject newObject;

					newObject.ID = topState.m_ID;
					newObject.posX = topState.m_displayRef.m_position.x;
					newObject.posY = topState.m_displayRef.m_position.y;
					newObject.posZ = topState.m_displayRef.m_position.z;

					newObject.rotX = topState.m_displayRef.m_orientation.x;
					newObject.rotY = topState.m_displayRef.m_orientation.x;
					newObject.rotZ = topState.m_displayRef.m_orientation.x;

					newObject.scaX = topState.m_displayRef.m_scale.x;
					newObject.scaY = topState.m_displayRef.m_scale.x;
					newObject.scaZ = topState.m_displayRef.m_scale.x;
				
						
					newObject.model_path = topState.m_sceneRef.model_path;			
					
					newObject.tex_diffuse_path = topState.m_sceneRef.tex_diffuse_path;	
					newObject.chunk_ID = topState.m_sceneRef.chunk_ID;

					sceneGraph.push_back(newObject);
				
				}
				BuildDisplayList(&sceneGraph);	// Build display list

				redoStack->push(topState);		// Add top state to redo stack
				undoStack->pop();				// pop top state from undo stack
			}
			else 
			{
				auto it = std::find_if(m_displayList->begin(), m_displayList->end(),		// find object in display list
					[&](const DisplayObject& obj) { return obj.m_ID == topState.m_ID; });
				
				if (it != m_displayList->end())								// if object exists
				{
					it->m_position.x = topState.m_displayRef.m_position.x;	// set position x
					it->m_position.y = topState.m_displayRef.m_position.y;	// set position y
					it->m_position.z = topState.m_displayRef.m_position.z;	// set position z

					it->m_orientation.x = topState.m_displayRef.m_orientation.x;	// set orientation x
					it->m_orientation.y = topState.m_displayRef.m_orientation.y;	// set orientation y
					it->m_orientation.z = topState.m_displayRef.m_orientation.z;	// set orientation z

					it->m_scale.x = topState.m_displayRef.m_scale.x;	// set scale x
					it->m_scale.y = topState.m_displayRef.m_scale.y;	// set scale y
					it->m_scale.z = topState.m_displayRef.m_scale.z;	// set scale z
				}

				redoStack->push(topState);	// push top state to redo stack
				undoStack->pop();			// pop top state from undo stack
			}
		}
	}
}

void Game::Redo(std::stack<DObjectState>* redoStack, std::stack<DObjectState>* undoStack, std::vector<SceneObject>& sceneGraph)
{
	if (redoStack->size() == 1)
		return;

	if (!redoStack->empty())							// if redo stack is not empty						
	{
		for (int i = 0; i < 2; i++)
		{
			DObjectState topState = redoStack->top();	// Get last state to redo

			if (topState.m_isNewObject)					// check if object is new
			{
				auto sceneObj = std::find_if(sceneGraph.begin(), sceneGraph.end(),    // find object in scene graph
					[&](const SceneObject& obj) { return obj.ID == topState.m_ID; });

				if (sceneObj != sceneGraph.end())		// if object exists in scenegraph
				{
					if (i == 0)
						sceneGraph.erase(sceneObj);		// erase object
				}
				else									// else create new object and add to scene graph
				{ 
					if (i == 0) 
					{
						SceneObject newObject;

						newObject.ID = topState.m_ID;
						newObject.posX = topState.m_displayRef.m_position.x;
						newObject.posY = topState.m_displayRef.m_position.y;
						newObject.posZ = topState.m_displayRef.m_position.z;

						newObject.rotX = topState.m_displayRef.m_orientation.x;
						newObject.rotY = topState.m_displayRef.m_orientation.x;
						newObject.rotZ = topState.m_displayRef.m_orientation.x;

						newObject.scaX = topState.m_displayRef.m_scale.x;
						newObject.scaY = topState.m_displayRef.m_scale.x;
						newObject.scaZ = topState.m_displayRef.m_scale.x;


						newObject.model_path = topState.m_sceneRef.model_path;

						newObject.tex_diffuse_path = topState.m_sceneRef.tex_diffuse_path;
						newObject.chunk_ID = topState.m_sceneRef.chunk_ID;


						sceneGraph.push_back(newObject);
						BuildDisplayObject(&newObject);
					}
				}
				BuildDisplayList(&sceneGraph);	// Build display list
		
				undoStack->push(topState);		// push top state to undo stack
				redoStack->pop();				// pop top state from redo stack							
			}
			else 
			{
				auto it = std::find_if(m_displayList->begin(), m_displayList->end(),		// find object in display list
					[&](const DisplayObject& obj) { return obj.m_ID == topState.m_ID; });

				if (it != m_displayList->end())												// if object exists
				{
					it->m_position.x = topState.m_displayRef.m_position.x;					// set position x
					it->m_position.y = topState.m_displayRef.m_position.y;					// set position y
					it->m_position.z = topState.m_displayRef.m_position.z;					// set position z

					it->m_orientation.x = topState.m_displayRef.m_orientation.x;			// set orientation x
					it->m_orientation.y = topState.m_displayRef.m_orientation.y;			// set orientation y
					it->m_orientation.z = topState.m_displayRef.m_orientation.z;			// set orientation z

					it->m_scale.x = topState.m_displayRef.m_scale.x;						// set scale x
					it->m_scale.y = topState.m_displayRef.m_scale.y;						// set scale y
					it->m_scale.z = topState.m_displayRef.m_scale.z;						// set scale z
				}

				undoStack->push(topState);	// push top state to undo stack
				redoStack->pop();			// pop top state from redo stack
			}
		}
	}
}

void Game::Copy(std::vector<int>& selectedIDs, std::vector<SceneObject>& copiedObjects, std::vector<SceneObject>& m_sceneGraph)
{
	if (selectedIDs.empty() || selectedIDs[0] <= 0 || m_displayList->empty())		// if selectedIDs vector is empty or first element is invalid
		return;

	copiedObjects.clear();									//  clear copied objects vector

	for (int i = 0; i < selectedIDs.size(); i++)			// Loop through selected IDs
	{	
		if (selectedIDs[i] <= -1)							// if first element is invalid, continue
			continue;

		SceneObject& copied = m_sceneGraph[selectedIDs[i]];	// get object from scene graph
		copiedObjects.push_back(copied);					// create a new object and copy the data from the selected object
	}
}	

void Game::Paste(std::vector<SceneObject>& copiedObjects, std::vector<SceneObject>& m_sceneGraph)
{
	if (copiedObjects.empty())						// if copied objects vector is empty
		return;										// return from function

	for (int i = 0; i < copiedObjects.size(); i++)	// Loop through copied objects
	{
		SceneObject& newObject = copiedObjects[i];	// get object from copied objects

		newObject.ID = m_sceneGraph.size() + 1;		// set new ID

		newObject.posX += 1.0f;						// set new position x
		newObject.posY += 1.0f;						// set new position y
		newObject.posZ += 1.0f;						// set new position z			

		m_sceneGraph.push_back(newObject);			// add new object to scene graph
		BuildDisplayObject(&newObject);				// add new object to display list
			
		DisplayObject& object = m_displayList->back();		// get last object in display list

		DObjectState undoState(object, newObject, newObject.ID, true);		// create undo state for new object
		m_undoStack->push(undoState);										// push undo state to undo stack
		m_undoStack->push(undoState);										//
	} 
}

void Game::Delete(std::vector<int>& selectedIDs, std::vector<SceneObject>& m_sceneGraph)
{
	if (selectedIDs.empty() || selectedIDs[0] == -1 || m_displayList->empty())		// if selectedIDs vector is empty or first element is invalid
		return;																		// return from function

	GetSelectedObject(selectedIDs);													// get selected objects

	for (int i = 0; i < m_selectedObjects.size(); i++)								// loop through selected objects
	{
		DisplayObject& object = *m_selectedObjects[i];								// get selected object

		auto sceneObj = std::find_if(m_sceneGraph.begin(), m_sceneGraph.end(),		// find object in scene graph
			[&](const SceneObject& obj) { return obj.ID == object.m_ID; });

		if (sceneObj != m_sceneGraph.end())											// if object exists
		{
			DObjectState undoState(object, *sceneObj, object.m_ID, true);			// create undo state for object
			m_undoStack->push(undoState);											// push undo state to undo stack
			m_undoStack->push(undoState);
			m_sceneGraph.erase(sceneObj);											// erase object from scene graph
		}
	}

	BuildDisplayList(&m_sceneGraph);												// Build display list			

	m_selectedObjects.clear();														// Clear selected object
}

void Game::FocusOnObject(std::vector<int>& selectedIDs)
{
	if (selectedIDs.empty() || selectedIDs[0] == -1 || m_displayList->empty())		// if selectedIDs vector is empty or first element is invalid
		return;																		// return from function
		
	GetSelectedObject(selectedIDs);													// get selected objects

	DirectX::SimpleMath::Vector3 cumulativePos = { 0.f, 0.f, 0.f };					// create cumulative position vector

	for (int i = 0; i < m_selectedObjects.size(); i++)								// Loop through selected objects
	{
		DisplayObject* object = m_selectedObjects[i];								// get selected object

		cumulativePos += object->m_position;										// add position to cumulative position
	}

	cumulativePos /= m_selectedObjects.size();										// divide by number of selected objects to get average position

	m_Camera.FocusOnObject(cumulativePos, 10.f);									// set camera position to average position of selected objects
}

void Game::GetSelectedObject(std::vector<int>& selectedIDs)
{
	m_selectedObjects.clear();						// clear selected objects vector

	for (int i = 0; i < selectedIDs.size(); i++)	// loop through selected IDs
	{
		if (selectedIDs[i] <= -1)					// if first element is invalid, continue
			continue;

		auto selectedObj = std::find_if(m_displayList->begin(), m_displayList->end(),	// find object in display list
			[&](const DisplayObject& obj) { return obj.m_ID == selectedIDs[i]; });

		if (selectedObj != m_displayList->end())										// if object exists
			m_selectedObjects.push_back(&(*selectedObj));								// add object to selected objects vector
	}
}