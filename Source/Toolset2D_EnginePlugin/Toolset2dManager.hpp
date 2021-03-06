#ifndef SPRITE_MANAGER_HPP_INCLUDED
#define SPRITE_MANAGER_HPP_INCLUDED

#if defined(WIN32)
// needed for hkStringBuf
#include <Common/Base/Ext/hkBaseExt.h>
#endif // defined(WIN32)

class Sprite;
class Camera2D;
class VScriptCreateStackProxyObject;
class vHavokPhysicsModule;

enum ConstraintMode
{
	POINT_TO_PLANE_CONSTRAINT,
	CUSTOM_CONSTRAINT,
	KEYFRAME_UTIL,
	SET_POSITION_ROTATION,
};

enum GameMode
{
	MODE_STOPPED,
	MODE_PLAY_THE_GAME,
	MODE_RUN_IN_EDITOR
};

class SpriteCell
{
public:
	SpriteCell();

	VString name;
	hkvVec2 offset;
	hkvVec2 pivot;
	float width;
	float height;
	float originalWidth;
	float originalHeight;
	int index;

#if USE_HAVOK_PHYSICS_2D
	hkArray<int> verticesPerFace;
	hkArray<int> vertexIndices;
	hkArray<hkVector4> vertexPositions;

	hkpConvexVerticesShape *shape;
	hkpConvexVerticesShape *shape3d;
#endif // USE_HAVOK_PHYSICS_2D
};

class SpriteState
{
public:
	VString name;
	VArray<int> cells;
	float framerate;
};

class SpriteData
{
public:
	SpriteData();
	~SpriteData();

	void Cleanup();

	bool GenerateConvexHull();

	//-----

	float sourceWidth;
	float sourceHeight;

	VString spriteSheetFilename;
	VString xmlDataFilename;

	VArray<SpriteCell> cells;
	VArray<SpriteState> states;

	VTextureObject *spriteSheetTexture;
	VisTextureAnimInstance_cl *textureAnimation;

	VDictionary<int> stateNameToIndex;
};

#if defined(WIN32)
/// \brief Returns true if the given path is relative to one of the asset libraries (a.k.a. data directories).
TOOLSET_2D_IMPEXP bool convertToAssetPath(const char* absolutePath, hkStringBuf& out_relativePath);
#endif // defined(WIN32)

class Toolset2dManager
: public IVisCallbackHandler_cl
#if USE_HAVOK_PHYSICS_2D
, public IHavokStepper
#endif
{
public:
	TOOLSET_2D_IMPEXP VOVERRIDE void OnHandleCallback(IVisCallbackDataObject_cl *pData);

	// Called when plugin is loaded/unloaded
	TOOLSET_2D_IMPEXP void OneTimeInit();
	TOOLSET_2D_IMPEXP void OneTimeDeInit();
	
	TOOLSET_2D_IMPEXP void AddSprite(Sprite *sprite);
	TOOLSET_2D_IMPEXP int FindSprite(Sprite *sprite);
	TOOLSET_2D_IMPEXP void RemoveSprite(Sprite *sprite);
	
	TOOLSET_2D_IMPEXP const SpriteData *GetSpriteData(const VString &spriteSheetFilename, const VString &xmlDataFilename);

	TOOLSET_2D_IMPEXP void Render();
	TOOLSET_2D_IMPEXP void Update(float deltaTime);

	TOOLSET_2D_IMPEXP VOVERRIDE void Step( float dt );

	TOOLSET_2D_IMPEXP bool InSimulationMode() const;

	//----- Statics

	// Register our LUA library with the script manager
	static void RegisterLua();

	// Access one global instance of the frame manager
	static Toolset2dManager *Instance()
	{
		static Toolset2dManager spriteManager;
		return &spriteManager;
	}

	//----- Script functions

	TOOLSET_2D_IMPEXP static Sprite *CreateSprite(const hkvVec3 &position, const char *spriteSheetFilename, const char *xmlDataFilename = "");
	TOOLSET_2D_IMPEXP int GetNumSprites();

	TOOLSET_2D_IMPEXP void SetCamera(Camera2D *camera);
	TOOLSET_2D_IMPEXP Camera2D *GetCamera();

#if USE_HAVOK_PHYSICS_2D
	TOOLSET_2D_IMPEXP hkpWorld *GetPhysicsWorld();
#endif

protected:
	bool CreateLuaCast(VScriptCreateStackProxyObject *scriptData, const char *typeName, VType *type);

	void InitializeHavokPhysics();
	void UnintializeHavokPhysics();

	void RemoveSpriteData();

private:
	// Hold weak pointers so that if they get removed in some unexpected way we don't
	// have a dead pointer hanging around
	VArray< VWeakPtr<VisBaseEntity_cl>* > m_sprites;

	Camera2D *m_camera;

	GameMode m_gameMode;

#if USE_HAVOK_PHYSICS_2D
	hkpWorld *m_world;
	vHavokPhysicsModule *m_physicsModule;
	hkpPhysicsContext* m_pContext;
#endif // USE_HAVOK_PHYSICS_2D

	// We store the sprite data in the manager since sprites will most likely share
	// the same data and we don't want to re-parse the same information multiple times
	VArray<SpriteData*> m_spriteData;
};

#endif // SPRITE_MANAGER_HPP_INCLUDED