#ifndef SPRITE_ENTITY_HPP_INCLUDED
#define SPRITE_ENTITY_HPP_INCLUDED

struct SpriteState;
struct SpriteData;

class Sprite : public VisBaseEntity_cl
{
public:
	V_DECLARE_SERIAL_DLLEXP(Sprite, TOOLSET_2D_IMPEXP);

	IMPLEMENT_OBJ_CLASS(Sprite);

	enum VERTEX_INDICES
	{
		VERTEX_TOP_LEFT = 0,
		VERTEX_TOP_RIGHT = 1,
		VERTEX_BOTTOM_LEFT = 2,
		VERTEX_BOTTOM_RIGHT = 3
	};

	// Overridden entity functions
	TOOLSET_2D_IMPEXP VOVERRIDE void InitFunction();
	TOOLSET_2D_IMPEXP VOVERRIDE void DeInitFunction();
	TOOLSET_2D_IMPEXP VOVERRIDE void ThinkFunction();
  
	TOOLSET_2D_IMPEXP VOVERRIDE void OnVariableValueChanged(VisVariable_cl *pVar, const char * value);
	TOOLSET_2D_IMPEXP VOVERRIDE BOOL AddComponent(IVObjectComponent *pComponent);

	// Serialization and type management
	TOOLSET_2D_IMPEXP VOVERRIDE void Serialize( VArchive &ar );
	TOOLSET_2D_IMPEXP VOVERRIDE void OnSerialized( VArchive &ar );

	// Sender the shape in editor mode (or for debugging)
	TOOLSET_2D_IMPEXP void DebugRender(IVRenderInterface *pRenderer, float fSize, VColorRef iColor, bool bRenderConnections=false) const;

	TOOLSET_2D_IMPEXP void Render(IVRender2DInterface *pRender, VSimpleRenderState_t& state);

	TOOLSET_2D_IMPEXP bool SetSpriteSheetData(const char *spriteSheetFilename, const char *xmlFilename);

	TOOLSET_2D_IMPEXP const VArray<VString> GetStateNames() const;
	TOOLSET_2D_IMPEXP const SpriteState *GetCurrentState() const;

	TOOLSET_2D_IMPEXP int GetCurrentFrame() const;
	TOOLSET_2D_IMPEXP void SetCurrentFrame(int frame);

	TOOLSET_2D_IMPEXP bool IsOverlapping(Sprite *other) const;

	TOOLSET_2D_IMPEXP void Update();

	TOOLSET_2D_IMPEXP const hkvVec2 *GetVertices() const;

	//----- Utility functions exposed to LUA

	TOOLSET_2D_IMPEXP void OnCollision(Sprite *other);

	TOOLSET_2D_IMPEXP bool SetState(const char *state);

	// Specify a value between 0 and 1 and it will update the frame
	TOOLSET_2D_IMPEXP void SetFramePercent(float percent);

	TOOLSET_2D_IMPEXP void Pause();
	TOOLSET_2D_IMPEXP void Play();

	TOOLSET_2D_IMPEXP void SetScrollSpeed(hkvVec2 scrollSpeed);
	TOOLSET_2D_IMPEXP const hkvVec2 &GetScrollSpeed() const;

	TOOLSET_2D_IMPEXP void SetFullscreenMode(bool enabled);
	TOOLSET_2D_IMPEXP bool IsFullscreenMode() const;

	TOOLSET_2D_IMPEXP void SetPlayOnce(bool enabled);
	TOOLSET_2D_IMPEXP bool IsPlayOnce() const;

	TOOLSET_2D_IMPEXP void SetCollision(bool enabled);
	TOOLSET_2D_IMPEXP bool IsColliding() const;

	TOOLSET_2D_IMPEXP hkvVec3 GetPoint(float x, float y, float z = 0.0f) const;
	TOOLSET_2D_IMPEXP void SetCenterPosition(const hkvVec3 &position);
	TOOLSET_2D_IMPEXP hkvVec3 GetCenterPosition();

	TOOLSET_2D_IMPEXP float GetWidth() const;
	TOOLSET_2D_IMPEXP float GetHeight() const;

protected:
	void CommonInit();
	void CommonDeInit();

	void Clear();

	bool UpdateTextures();
	VTextureObject *GetTexture() const;

	hkvVec2 GetDimensions() const;

private:

	//----- Properties

	float TextureScale;

	hkvVec2 ScrollSpeed;
	//int BlendMode;
	bool Fullscreen;

	//----- Member variables

	// Tracks whether or not the spritesheet and XML data have been loaded
	bool m_loaded;

	int m_currentState;
	int m_currentFrame;
	float m_frameTime;
	bool m_paused;
	bool m_playOnce;
	bool m_collide;
	hkvVec2 m_scrollOffset;

	const SpriteData *m_spriteData;

	hkvVec2 m_vertices[4];
	hkvVec2 m_texCoords[4];

	VString m_spriteSheetFilename;
	VString m_xmlDataFilename;
};

#endif // SPRITE_ENTITY_HPP_INCLUDED