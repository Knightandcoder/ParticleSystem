#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace DirectX;
using namespace std::experimental;

GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

GameApp::~GameApp()
{
	
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	// ����ȳ�ʼ��������Ⱦ״̬���Թ��������Чʹ��
	RenderStates::InitAll(m_pd3dDevice);

	if (!m_BasicEffect.InitAll(m_pd3dDevice))
		return false;
	if (!m_ParticleEffect.InitAll(m_pd3dDevice1))
		return false;
	if (!InitResource())
		return false;

	// ��ʼ����꣬���̲���Ҫ
	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);

	return true;
}

void GameApp::OnResize()
{
	assert(m_pd2dFactory);
	assert(m_pdwriteFactory);
	// �ͷ�D2D�������Դ
	m_pColorBrush.Reset();
	m_pd2dRenderTarget.Reset();

	D3DApp::OnResize();

	// ΪD2D����DXGI������ȾĿ��
	ComPtr<IDXGISurface> surface;
	HR(m_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(surface.GetAddressOf())));
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
	HRESULT hr = m_pd2dFactory->CreateDxgiSurfaceRenderTarget(surface.Get(), &props, m_pd2dRenderTarget.GetAddressOf());
	surface.Reset();

	if (hr == E_NOINTERFACE)
	{
		OutputDebugString(L"\n���棺Direct2D��Direct3D�������Թ������ޣ��㽫�޷������ı���Ϣ�����ṩ������ѡ������\n"
			"1. ����Win7ϵͳ����Ҫ������Win7 SP1������װKB2670838������֧��Direct2D��ʾ��\n"
			"2. �������Direct3D 10.1��Direct2D�Ľ�����������ģ�"
			"https://docs.microsoft.com/zh-cn/windows/desktop/Direct2D/direct2d-and-direct3d-interoperation-overview""\n"
			"3. ʹ�ñ������⣬����FreeType��\n\n");
	}
	else if (hr == S_OK)
	{
		// �����̶���ɫˢ���ı���ʽ
		HR(m_pd2dRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::White),
			m_pColorBrush.GetAddressOf()));
		HR(m_pdwriteFactory->CreateTextFormat(L"����", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"zh-cn",
			m_pTextFormat.GetAddressOf()));
	}
	else
	{
		// �����쳣����
		assert(m_pd2dRenderTarget);
	}

	// ����������ʾ
	if (m_pCamera != nullptr)
	{
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());
	}
}

void GameApp::UpdateScene(float dt,float gametime)
{

	// ��������¼�����ȡ���ƫ����
	Mouse::State mouseState = m_pMouse->GetState();
	Mouse::State lastMouseState = m_MouseTracker.GetLastState();
	m_MouseTracker.Update(mouseState);

	Keyboard::State keyState = m_pKeyboard->GetState();
	m_KeyboardTracker.Update(keyState);

	// ��ȡ����
	auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);

	// ******************
	// ����������Ĳ���
	//

	// �����ƶ�
	if (keyState.IsKeyDown(Keyboard::W))
		cam1st->MoveForward(dt * 3.0f);
	if (keyState.IsKeyDown(Keyboard::S))
		cam1st->MoveForward(dt * -3.0f);
	if (keyState.IsKeyDown(Keyboard::A))
		cam1st->Strafe(dt * -3.0f);
	if (keyState.IsKeyDown(Keyboard::D))
		cam1st->Strafe(dt * 3.0f);

	// ��Ұ��ת����ֹ��ʼ�Ĳ�ֵ�����µ�ͻȻ��ת
	cam1st->Pitch(mouseState.y * dt * 1.25f);
	cam1st->RotateY(mouseState.x * dt * 1.25f);


	// ******************
	// ������������
	//

	// ��λ��������[-119.9f, 119.9f]��������
	// ��������
	XMFLOAT3 adjustedPos;
	XMStoreFloat3(&adjustedPos, XMVectorClamp(cam1st->GetPositionXM(),
		XMVectorSet(-119.9f, 0.0f, -119.9f, 0.0f), XMVectorSet(119.9f, 99.9f, 119.9f, 0.0f)));
	cam1st->SetPosition(adjustedPos);
	// ******************
	// ������Ч
	//
	/*if (m_KeyboardTracker.IsKeyPressed(Keyboard::D1))
	{
		m_FogEnabled = !m_FogEnabled;
		m_BasicEffect.SetFogState(m_FogEnabled);
	}*/
	// ******************
	// ������ķ�Χ
	//
	if (mouseState.scrollWheelValue != 0)
	{
		// һ�ι��ֹ�������С��λΪ120
		m_FogRange += mouseState.scrollWheelValue / 120;
		if (m_FogRange < 15.0f)
			m_FogRange = 15.0f;
		else if (m_FogRange > 175.0f)
			m_FogRange = 175.0f;
		m_BasicEffect.SetFogRange(m_FogRange);
	}

	// ���¹۲����
	m_BasicEffect.SetEyePos(m_pCamera->GetPositionXM());
	m_pCamera->UpdateViewMatrix();
	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());
	m_ParticleEffect.SetViewMatrix(m_pCamera->GetViewXM());
	//��������
	UpdateParticle(dt, gametime);
	// ******************
	// ��׶��ü�����
	//
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D1))
	{
		m_EnableInstancing = !m_EnableInstancing;
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D2))
	{
		m_EnableFrustumCulling = !m_EnableFrustumCulling;
	}


	// ���ù���ֵ
	m_pMouse->ResetScrollWheelValue();

	// �˳���������Ӧ�򴰿ڷ���������Ϣ
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::Escape))
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	// ******************
	// ����Direct3D����
	//
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Silver));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	DrawParticle();
	// ͳ��ʵ�ʻ��Ƶ�������Ŀ
	std::vector<XMMATRIX> acceptedData;
	// �Ƿ�����׶��ü�
	if (m_EnableFrustumCulling)
	{
		acceptedData = Collision::FrustumCulling(m_InstancedData, m_Trees.GetLocalBoundingBox(),
			m_pCamera->GetViewXM(), m_pCamera->GetProjXM());
	}
	// ȷ��ʹ�õ����ݼ�
	const std::vector<XMMATRIX>& refData = m_EnableFrustumCulling ? acceptedData : m_InstancedData;
	// �Ƿ���Ӳ��ʵ����
	if (m_EnableInstancing)
	{
		// Ӳ��ʵ��������
		m_BasicEffect.SetRenderDefault(m_pd3dImmediateContext, BasicEffect::RenderInstance);
		m_BasicEffect.Apply(m_pd3dImmediateContext);
		m_Trees.DrawInstanced(m_pd3dImmediateContext, m_BasicEffect, refData);
	}
	else
	{
		// ��������ʽ�������
		m_BasicEffect.SetRenderDefault(m_pd3dImmediateContext, BasicEffect::RenderObject);
		for (FXMMATRIX mat : refData)
		{
			m_Trees.SetWorldMatrix(mat);
			m_BasicEffect.Apply(m_pd3dImmediateContext);
			m_Trees.Draw(m_pd3dImmediateContext, m_BasicEffect);
		}
	}

	// ���Ƶ���
	m_BasicEffect.SetRenderDefault(m_pd3dImmediateContext, BasicEffect::RenderObject);
	m_BasicEffect.Apply(m_pd3dImmediateContext);
	m_Ground.Draw(m_pd3dImmediateContext, m_BasicEffect);

	// ******************
	// ����Direct2D����
	//
	if (m_pd2dRenderTarget != nullptr)
	{
		m_pd2dRenderTarget->BeginDraw();
		std::wstring text = L"��ǰ�����ģʽ: �����ӽ�  Esc�˳�\n"
			"����ƶ�������Ұ WSAD�ƶ�\n"
			"���ּ�1:Ӳ��ʵ�������� 2:��׶��ü�����\n"
			"Ӳ��ʵ����: ";
		text += (m_EnableInstancing ? L"��" : L"��");
		text += L" ��׶��ü�: ";
		text += (m_EnableFrustumCulling ? L"��" : L"��");
		text += L"\n������ľ��: " + std::to_wstring(refData.size());
		text += L"\n��Ч��Χ: "+std::to_wstring((int)m_FogRange);
		m_pd2dRenderTarget->DrawTextW(text.c_str(), (UINT32)text.length(), m_pTextFormat.Get(),
			D2D1_RECT_F{ 0.0f, 0.0f, 600.0f, 200.0f }, m_pColorBrush.Get());
		HR(m_pd2dRenderTarget->EndDraw());
	}

	HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitResource()
{
	// Ĭ�Ͽ�����׶��ü���Ӳ��ʵ����
	m_EnableInstancing = true;
	m_EnableFrustumCulling = true;

	// ******************
	// ��ʼ����Ϸ����
	//

	// �����������
	CreateRandomTrees();

	// ��ʼ������
	m_ObjReader.Read(L"Model\\ground.mbo", L"Model\\ground.obj");
	m_Ground.SetModel(Model(m_pd3dDevice, m_ObjReader));

	// ******************
	// ��ʼ�������
	//
	m_CameraMode = CameraMode::Free;
	auto camera = std::shared_ptr<FirstPersonCamera>(new FirstPersonCamera);
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	camera->LookTo(
		XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
		XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	camera->UpdateViewMatrix();
	m_BasicEffect.SetViewMatrix(camera->GetViewXM());
	m_BasicEffect.SetProjMatrix(camera->GetProjXM());

	m_ParticleEffect.SetViewMatrix(camera->GetViewXM());
	m_ParticleEffect.SetProjMatrix(camera->GetProjXM());
	m_ParticleEffect.SetEmitDirection(camera->GetPosition());
	m_ParticleEffect.SetEmitPosition(camera->GetPosition());
	// ******************
	// ��ʼ������仯��ֵ
	//
	m_EnableAlphaToCoverage = true;

	// ��״̬Ĭ�Ͽ���
	m_FogEnabled = 1;
	m_FogRange = 75.0f;
	m_BasicEffect.SetFogState(m_FogEnabled);
	m_BasicEffect.SetFogColor(XMVectorSet(0.75f, 0.75f, 0.75f, 1.0f));
	m_BasicEffect.SetFogStart(15.0f);
	m_BasicEffect.SetFogRange(75.0f);

	// �����
	DirectionalLight dirLight[4];
	dirLight[0].Ambient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
	dirLight[0].Diffuse = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	dirLight[0].Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	dirLight[0].Direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
	dirLight[1] = dirLight[0];
	dirLight[1].Direction = XMFLOAT3(0.577f, -0.577f, 0.577f);
	dirLight[2] = dirLight[0];
	dirLight[2].Direction = XMFLOAT3(0.577f, -0.577f, -0.577f);
	dirLight[3] = dirLight[0];
	dirLight[3].Direction = XMFLOAT3(-0.577f, -0.577f, -0.577f);
	for (int i = 0; i < 4; ++i)
		m_BasicEffect.SetDirLight(i, dirLight[i]);
	InitParticle();
	return true;
}

void GameApp::CreateRandomTrees()
{
	srand((unsigned)time(nullptr));
	// ��ʼ����
	m_ObjReader.Read(L"Model\\tree.mbo", L"Model\\tree.obj");
	m_Trees.SetModel(Model(m_pd3dDevice, m_ObjReader));
	XMMATRIX S = XMMatrixScaling(0.015f, 0.015f, 0.015f);
	
	BoundingBox treeBox = m_Trees.GetLocalBoundingBox();
	// ��ȡ����Χ�ж���
	m_TreeBoxData = Collision::CreateBoundingBox(treeBox, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	// ����ľ�ײ���������λ��y = -2��ƽ��
	treeBox.Transform(treeBox, S);
	XMMATRIX T0 = XMMatrixTranslation(0.0f, -(treeBox.Center.y - treeBox.Extents.y + 2.0f), 0.0f);
	// �������256������������
	float theta = 0.0f;
	for (int i = 0; i < 16; ++i)
	{
		// ȡ5-125�İ뾶�����������
		for (int j = 0; j < 4; ++j)
		{
			// ����ԽԶ����ľԽ��
			for (int k = 0; k < 2 * j + 1; ++k)
			{
				float radius = (float)(rand() % 30 + 30 * j + 5);
				float randomRad = rand() % 256 / 256.0f * XM_2PI / 16;
				XMMATRIX T1 = XMMatrixTranslation(radius * cosf(theta + randomRad), 0.0f, radius * sinf(theta + randomRad));
				XMMATRIX R = XMMatrixRotationY(rand() % 256 / 256.0f * XM_2PI);
				XMMATRIX World = S * R * T0 * T1;
				m_InstancedData.push_back(World);
			}
		}
		theta += XM_2PI / 16;
	}
}

void GameApp::InitParticle()
{
	m_MaxParticles = 10000;
	m_FirstRun = true;
	m_GameTime = 0.0f;
	m_TimeStep = 0.0f;
	m_Age = 0.0f;

	//��ʼ�������ӵ�����
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(),
		L"Model\\raindrop.dds",
		nullptr,
		m_pRainTexture.GetAddressOf()));
	m_ParticleEffect.SetTexture(m_pRainTexture);
	//��ʼ���������
	m_pRandomTexture =CreateRandomTexture1DSRV(m_pd3dDevice.Get());
	m_ParticleEffect.SetRamdonTexture(m_pRandomTexture);

	// ���ö��㻺��������
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_DEFAULT;	// ������Ҫ����������׶�ͨ��GPUд��
	vbd.ByteWidth = sizeof(Particle);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER ;	
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	Particle p;
	ZeroMemory(&p, sizeof(Particle));
	p.Age = 0.0f;
	p.Type = 0;
	// �½����㻺����
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &p;
	HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffers[0].ReleaseAndGetAddressOf()));  //���ڳ�ʼ��

	vbd.ByteWidth = sizeof(Particle)*m_MaxParticles;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
    HR(m_pd3dDevice->CreateBuffer(&vbd, nullptr, m_pVertexBuffers[1].ReleaseAndGetAddressOf()));   
	HR(m_pd3dDevice->CreateBuffer(&vbd, nullptr, m_pVertexBuffers[2].ReleaseAndGetAddressOf()));   
}

void GameApp::UpdateParticle(float dt,float gametime)
{
	m_GameTime = gametime;
	m_TimeStep = dt;
	m_Age += dt;
}

void GameApp::DrawParticle()
{
	m_ParticleEffect.SetGameTime(m_GameTime);
	m_ParticleEffect.SetTimeStep(m_TimeStep);
	UINT stride = sizeof(Particle);
	UINT offset = 0;
	static int inputIndex = 1;

	if (m_FirstRun)
	{
        m_ParticleEffect.SetRenderStreamOutParticle(m_pd3dImmediateContext, m_pVertexBuffers[0], m_pVertexBuffers[1]);
	}
	else
	{
		
		m_ParticleEffect.SetRenderStreamOutParticle(m_pd3dImmediateContext, m_pVertexBuffers[inputIndex], m_pVertexBuffers[inputIndex % 2 + 1]);
		inputIndex = 3 - inputIndex;
	}
	m_ParticleEffect.Apply(m_pd3dImmediateContext);
	if (m_FirstRun)
	{
		m_pd3dImmediateContext->Draw(1, 0);
		m_FirstRun = false;
	}
	else
	{
		m_pd3dImmediateContext->DrawAuto();
	}
	m_ParticleEffect.SetRenderDefault(m_pd3dImmediateContext);
	m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffers[inputIndex].GetAddressOf(), &stride, &offset);
    m_ParticleEffect.Apply(m_pd3dImmediateContext);
    m_pd3dImmediateContext->DrawAuto();
	
}


