#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "Camera.h"
#include "GameObject.h"
#include "ObjReader.h"
#include "Collision.h"
#include <DXGItype.h>  
#include <dxgi1_2.h>  
#include <dxgi1_3.h>  
#include <DXProgrammableCapture.h>

class GameApp : public D3DApp
{
public:
	// �����ģʽ
	enum class CameraMode { FirstPerson, ThirdPerson, Free };
	
public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt, float gametime);
	void DrawScene();
	
private:
	bool InitResource();
	void CreateRandomTrees();
	void InitParticle();
	void UpdateParticle(float dt, float gametime);
	void ResetParticle();
	void DrawParticle();
private:
	
	ComPtr<ID2D1SolidColorBrush> m_pColorBrush;				    // ��ɫ��ˢ
	ComPtr<IDWriteFont> m_pFont;								// ����
	ComPtr<IDWriteTextFormat> m_pTextFormat;					// �ı���ʽ
	ComPtr<ID3D11Buffer> m_pVertexBuffers[3];                   //���㻺��������

	GameObject m_Trees;										    // ��
	GameObject m_Ground;										// ����
	std::vector<DirectX::XMMATRIX> m_InstancedData;			    // ����ʵ������
	Collision::WireFrameData m_TreeBoxData;					    // ����Χ���߿�����

	ComPtr<ID3D11ShaderResourceView> m_pRainTexture;			    // ������
	ComPtr<ID3D11ShaderResourceView> m_pRandomTexture;                //�������
	BasicEffect m_BasicEffect;								    // ������Ⱦ��Ч����
	ParticleEffect m_ParticleEffect;                            //������Ⱦ��Ч����
	bool m_EnableFrustumCulling;								// ��׶��ü�����
	bool m_EnableInstancing;									// Ӳ��ʵ��������

	std::shared_ptr<Camera> m_pCamera;						    // �����
	CameraMode m_CameraMode;									// �����ģʽ
	ObjReader m_ObjReader;									    // ģ�Ͷ�ȡ����

	UINT m_MaxParticles;                                         //��������
	bool m_FirstRun;                                             //�Ƿ��һ����

	float m_GameTime;                                             //��Ϸʱ��
	float m_TimeStep;                                             //��Ϸ����
	float m_Age;                                                  //����

	bool m_FogEnabled;										    // �Ƿ�����Ч
	bool m_IsNight;											    // �Ƿ��ҹ
	bool m_EnableAlphaToCoverage;							    // �Ƿ���Alpha-To-Coverage
	float m_FogRange;										    // ��Ч��Χ
};


#endif