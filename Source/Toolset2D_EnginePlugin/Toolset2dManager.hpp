#ifndef SPRITE_MANAGER_HPP_INCLUDED
#define SPRITE_MANAGER_HPP_INCLUDED

#include "HUD.hpp"

class Sprite;
class Camera2D;
class VScriptCreateStackProxyObject;

struct SpriteCell
{
	VString name;
	hkvVec2 offset;
	hkvVec2 pivot;
	float width;
	float height;
	float originalWidth;
	float originalHeight;
	int index;

	hkArray<int> verticesPerFace;
	hkArray<int> vertexIndices;
	hkArray<hkVector4> vertexPositions;
};

struct SpriteState
{
	VString name;
	VArray<int> cells;
	float framerate;
};

class SpriteData
{
public:
	bool GenerateConvexHull();

	//-----

	float sourceWidth;
	float sourceHeight;

	VString spriteSheetFilename;
	VString xmlDataFilename;

	VArray<SpriteCell> cells;
	VArray<SpriteState> states;

	VTextureObject *spriteSheetTexture;
	VisTextureAnimInstance_cl *spTextureAnimation;

	VDictionary<int> stateNameToIndex;
};

/// \brief Returns true if the given path is relative to one of the asset libraries (a.k.a. data directories).
TOOLSET_2D_IMPEXP bool convertToAssetPath(const char* absolutePath, hkStringBuf& out_relativePath);

class Toolset2dManager : public IVisCallbackHandler_cl
{
public:
	VOVERRIDE void OnHandleCallback(IVisCallbackDataObject_cl *pData);

	// Called when plugin is loaded/unloaded
	TOOLSET_2D_IMPEXP void OneTimeInit();
	TOOLSET_2D_IMPEXP void OneTimeDeInit();
	
	TOOLSET_2D_IMPEXP void AddSprite(Sprite *sprite);
	TOOLSET_2D_IMPEXP void RemoveSprite(Sprite *sprite);
	
	TOOLSET_2D_IMPEXP const SpriteData *GetSpriteData(const VString &spriteSheetFilename, const VString &xmlDataFilename);

	TOOLSET_2D_IMPEXP void Render();
	TOOLSET_2D_IMPEXP void Update();

	// Register our LUA library with the script manager
	static void RegisterLua();

	// Access one global instance of the frame manager
	static Toolset2dManager *Instance()
	{
		static Toolset2dManager spriteManager;
		return &spriteManager;
	}

	//----- Script functions

	TOOLSET_2D_IMPEXP Sprite *CreateSprite(const hkvVec3 &position, const VString &spriteSheetFilename, const VString &xmlDataFilename);
	TOOLSET_2D_IMPEXP int GetNumSprites();

	TOOLSET_2D_IMPEXP void SetCamera(Camera2D *camera);
	TOOLSET_2D_IMPEXP Camera2D *GetCamera();

protected:
	void SetPlayTheGame(bool bStatus);

	bool CreateLuaCast(VScriptCreateStackProxyObject *scriptData, const char *typeName, VType *type);

private:
	VArray<Sprite*> m_sprites;
	Camera2D *m_camera;

	bool m_bPlayingTheGame;

	HUDGUIContextPtr m_spHUD;

	// We store the sprite data in the manager since sprites will most likely share
	// the same data and we don't want to re-parse the same information multiple times
	VArray<SpriteData*> m_spriteData;
};

#endif // SPRITE_MANAGER_HPP_INCLUDED