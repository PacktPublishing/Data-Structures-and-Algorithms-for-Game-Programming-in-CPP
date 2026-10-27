
#pragma once

#include <vector>

#include "raylib.h"

#include "Component.h"

#define LOAD_FLAG_COUNT  (MATERIAL_MAP_BRDF + 1)

class ModelComponent : public Component
{
protected:

	enum eLoadFlag : unsigned int
	{
		Loaded_Model = 0x01,
		Loaded_Animations = 0x02,
		DiffuseMap = 0x04,
		SpecularMap = 0x08,
		NormalMap = 0x10,
		MetalicMap = 0x20,
		RoughnessMap = 0x40,
		HeightMap = 0x80,
		CubeMap = 0x100,
		EmmissionMap = 0x200,
		OcclusionMap = 0x400
	};
public:
	enum eAnimMode
	{
		Default = 0,
		Linear_interpolation,
		Exponential_interpolation
	};

	enum eAnimTransitionMode
	{
		Immediate = 0,
		Linear,
		EaseInEaseOut
	};

public:

	ModelComponent();
	~ModelComponent();

	void Update(float ElapsedSeconds, RenderHints* pRH = nullptr) override;
	void Draw(RenderHints *pRH = nullptr) override;

	void Load3DModel(const char* ModelPath,	
		const char* DiffuseMapPath = nullptr, 
		const char* SpecularMapPath = nullptr,
		const char* NormalMapPath = nullptr,
		const char* MetalicMapPath = nullptr,
		const char* RoughnessMapPath = nullptr,
		const char* HeightMapPath = nullptr,
		const char* CubeMapPath = nullptr,
		const char* EmissionMapPath = nullptr,
		const char* OcclusionMapPath = nullptr,
		Color Color = WHITE);

	void LoadFromMesh(Mesh mesh, 
		const char* DiffuseMapPath = nullptr,
		const char* SpecularMapPath = nullptr,
		const char* NormalMapPath = nullptr,
		const char* MetalicMapPath = nullptr,
		const char* RoughnessMapPath = nullptr,
		const char* HeightMapPath = nullptr,
		const char* CubeMapPath = nullptr,
		const char* EmissionMapPath = nullptr,
		const char* OcclusionMapPath = nullptr,
		Color Color = WHITE);

	void LoadMaterialTextures(int idx, 
		const char* DiffuseMapPath = nullptr,
		const char* SpecularMapPath = nullptr,
		const char* NormalMapPath = nullptr,
		const char* MetalicMapPath = nullptr,
		const char* RoughnessMapPath = nullptr,
		const char* HeightMapPath = nullptr,
		const char* CubeMapPath = nullptr,
		const char* EmissionMapPath = nullptr,
		const char* OcclusionMapPath = nullptr);

	int GetAnimationIndex();

	bool SetAnimation(int AnimationIndex);
	int GetAnimation() { return _AnimationIndex; }
	void SetAnimationMode(eAnimMode AnimationMode);
	void SetFrameDuration(float Seconds);
	eAnimMode GetAnimationMode();

	Model* GetModel() { return &_Model; }

	/* Funciton: TransitionAnimation
	*  Description; Start transitioning current animation to a new animation
	*  Paramaters: 
	*		AnimationIndex: the index of the next animation clip
	*		TransitionSeconds: seconds to transition from the current to the next animation 
	*  Return: true-tranition started successfully; false-failed to transition to the next animation.
	*/
	bool TransitionAnimation(int AnimationIndex, float TransitionSeconds = 0.1f);

	/* Funciton: SetTransitionMode
	*  Description:  Set current active animation transition mode
	*  Parameter: TransitionMode: the transition mode to set
	*/
	void SetTransitionMode(eAnimTransitionMode TransitionMode); 
	eAnimTransitionMode GetTransitionMode();

	bool DrawBoundingBox = false;
	BoundingBox GetBoundingBox();
	ModelAnimation* _Animations = nullptr;
	int _AnimationsCount;

	bool _AlwaysSmoothNormal = true; // If true, recalculate smooth vertex normals for the model

protected:
	unsigned char _LoadState;

	Model _Model = { 0 };
	int _AnimationIndex;
	int _CurrentFrame[2] = { 0 };			//Current frame indices for the current and next animations

	eAnimMode _AnimationMode = eAnimMode::Linear_interpolation;
	int _PrevFrame[2] = { 0 };				//Previous frame indices for the current and next animations
	float _FrameDuration;			//Seconds from prev frame to current frame
	float _InterpolationTime[2] = { 0 };	//Interpolation times for the current and next animations

	eAnimTransitionMode _AnimTranistionMode;
	float _TransitionDuration;
	int _TransiteToAnimationIndex;
	float _TransitionTime;

	Texture2D _Texture2DMaps[LOAD_FLAG_COUNT];
	Color _Color;

	std::vector<BoundingBox> _MeshBoundingBoxes;
	
	int GetNextFrame(float InterpolationTime = 0.0f, int Channel = 0);
	void UpdateModelAnimationWithInterpolation(float ElapsedSeconds);
	void UpdateAnimationVertexBuffers();
	void InterpolateAnimation(int ChannelCount);

	void UpdateMeshBoundingBoxes();
};
