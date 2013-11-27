//=======
//
// Author: Joel Van Eenwyk
// Purpose: Handle sprite setup and rendering
//
//=======

#include "Toolset2D_EnginePluginPCH.h"

#include "SpriteEntity.hpp"
#include "Toolset2dManager.hpp"

#include <Vision/Runtime/EnginePlugins/EnginePluginsImport.hpp>
#include <Vision/Runtime/Base/ThirdParty/tinyXML/tinyxml.h>

#define CURRENT_SPRITE_VERSION 2

V_IMPLEMENT_SERIAL(Sprite, VisBaseEntity_cl, 0, &gToolset2D_EngineModule);

Sprite::Sprite()
{
}

Sprite::~Sprite()
{
}

// Called by the engine when entity is created. Not when it is de-serialized!
void Sprite::InitFunction()
{
	VisBaseEntity_cl::InitFunction();
	SetObjectKey(NULL);

	Clear();
	CommonInit();
}

// called by the engine when entity is destroyed
void Sprite::DeInitFunction()
{
	VisBaseEntity_cl::DeInitFunction();
	CommonDeInit();
}

// called by our InitFunction and our de-serialization code
void Sprite::CommonInit()
{
	Toolset2dManager::Instance()->AddSprite(this);
	SetSpriteSheetData(m_spriteSheetFilename, m_xmlDataFilename);
}

void Sprite::CommonDeInit()
{ 
	Toolset2dManager::Instance()->RemoveSprite(this);	
	Clear();
}

void Sprite::Clear()
{
	m_scrollSpeed.setZero();
	m_fullscreen = false;

	m_currentState = -1;
	m_currentFrame = -1;
	m_frameTime = 0.f;
	m_paused = false;
	m_playOnce = false;
	m_collide = true;
	m_scrollOffset.setZero();

	m_spriteData = NULL;

	m_spriteSheetFilename = NULL;
	m_xmlDataFilename = NULL;
}

void Sprite::UpdateTextures()
{
	m_spriteData = Toolset2dManager::Instance()->GetSpriteData(m_spriteSheetFilename, m_xmlDataFilename);
	if (m_spriteData != NULL)
	{
		if (m_spriteData->states.GetSize() > 0)
		{
			m_currentState = m_currentFrame = 0;
		}
	}
}

const hkvVec2 *Sprite::GetVertices() const
{
	return m_vertices;
}

hkvAlignedBBox Sprite::GetBBox() const
{
	// add a depth to the bounding box even though it's 2D in case we want to put it
	// in a 3D world and maybe add physics to it
	const float depth = 5;

	hkvAlignedBBox bbox;
	bbox.m_vMin.set(FLT_MAX, FLT_MAX, -depth);
	bbox.m_vMax.set(FLT_MIN, FLT_MIN, depth);

	for (int vertexIndex = 0; vertexIndex < 4; vertexIndex++)
	{
		bbox.m_vMin.x = hkvMath::Min(bbox.m_vMin.x, m_vertices[vertexIndex].x);
		bbox.m_vMin.y = hkvMath::Min(bbox.m_vMin.y, m_vertices[vertexIndex].y);
		bbox.m_vMax.x = hkvMath::Max(bbox.m_vMax.x, m_vertices[vertexIndex].x);
		bbox.m_vMax.y = hkvMath::Max(bbox.m_vMax.y, m_vertices[vertexIndex].y);
	}

	return bbox;
}

bool Sprite::SetSpriteSheetData(const char *spriteSheetFilename, const char *xmlFilename)
{
	bool success = false;

	if (m_spriteSheetFilename == spriteSheetFilename &&
		m_xmlDataFilename == xmlFilename &&
		m_spriteData != NULL)
	{
		success = true;
	}
	else
	{
		m_spriteSheetFilename = spriteSheetFilename;
		m_xmlDataFilename = xmlFilename;
		UpdateTextures();
	}

	return success;
}

const VArray<VString> Sprite::GetStateNames() const
{
	VArray<VString> names;

	if (m_spriteData != NULL)
	{
		for (int i = 0; i < m_spriteData->states.GetSize(); i++)
		{
			names.Add(m_spriteData->states[i].name);
		}
	}

	return names;
}

const SpriteState *Sprite::GetCurrentState() const
{
	const SpriteState *state = NULL;
	if (m_spriteData != NULL && m_currentState >=0)
	{
		state = &m_spriteData->states[m_currentState];
	}
	return state;
}

int Sprite::GetCurrentFrame() const
{
	return m_currentFrame;
}

void Sprite::SetCurrentFrame(int currentFrame)
{
	if (m_spriteData != NULL && m_currentState >= 0)
	{
		m_paused = true;

		const SpriteState *s = &m_spriteData->states[m_currentState];
		const int numCells = s->cells.GetSize();		
		m_currentFrame = hkvMath::clamp(currentFrame, 0, numCells - 1);
	}
}

float Sprite::GetCellWidth() const
{
	return ( GetCurrentCell() ? GetCurrentCell()->width : 0.f );
}

float Sprite::GetCellHeight() const
{
	return ( GetCurrentCell() ? GetCurrentCell()->height : 0.f );
}

float Sprite::GetOriginalCellWidth() const
{
	return ( GetCurrentCell() ? GetCurrentCell()->originalWidth : 0.f );
}

float Sprite::GetOriginalCellHeight() const
{
	return ( GetCurrentCell() ? GetCurrentCell()->originalHeight : 0.f );
}

void Sprite::ThinkFunction()
{
	if (m_spriteData != NULL && m_currentState >= 0 && !m_paused)
	{
		const float dt = Vision::GetTimer()->GetTimeDifference();

		m_frameTime += dt;
		m_scrollOffset += m_scrollSpeed * dt;

		if (!hkvMath::isFloatEqual(m_scrollSpeed.x, 0.0f))
		{
			if (m_scrollSpeed.x > 0 && m_scrollOffset.x > 1.0f)
			{
				m_scrollOffset.x -= 1.0f;
			}
			else if (m_scrollSpeed.x < 0 && m_scrollOffset.x < 0.0f)
			{
				m_scrollOffset.x += 1.0f;
			}
		}

		if (!hkvMath::isFloatEqual(m_scrollSpeed.y, 0.0f))
		{
			if (m_scrollSpeed.y > 0 && m_scrollOffset.y > 1.0f)
			{
				m_scrollOffset.y -= 1.0f;
			}
			else if (m_scrollSpeed.y < 0 && m_scrollOffset.y < 0.0f)
			{
				m_scrollOffset.y += 1.0f;
			}
		}

		const SpriteState *state = &m_spriteData->states[m_currentState];
		const float inverseFramerate = 1.0f / state->framerate;
		if (m_frameTime >= inverseFramerate)
		{
			const int numCells = state->cells.GetSize();
			const int lastFrame = m_currentFrame;

			m_currentFrame = (lastFrame + 1) % numCells;
			m_frameTime -= dt;

			if (m_currentFrame < lastFrame)
			{
				this->TriggerScriptEvent("OnSpriteStateEnd");
				if (m_playOnce)
				{
					m_currentFrame = numCells - 1;
					Pause();
				}
			}
		}
	}
}

bool Sprite::SetState(const char *state)
{
	int index = -1;
	if (m_spriteData != NULL)
	{
		index = m_spriteData->stateNameToIndex.Find(state);
		if (index != -1)
		{
			m_currentState = index;
			m_currentFrame = 0;
			m_frameTime = 0.f;
		}

	}
	return (index != -1);
}

void Sprite::SetFramePercent(float percent)
{
	if (m_spriteData != NULL && m_currentState >= 0)
	{
		m_paused = true;

		const SpriteState *s = &m_spriteData->states[m_currentState];
		const int numCells = s->cells.GetSize();		
		m_currentFrame = static_cast<int>( hkvMath::clamp(percent, 0.f, 1.f) * static_cast<float>(numCells - 1) );
	}
}

void Sprite::Play()
{
	m_paused = false;
}

void Sprite::Pause()
{
	m_frameTime = 0.f;
	m_paused = true;
}

void Sprite::SetScrollSpeed(hkvVec2 scrollSpeed)
{
	m_scrollSpeed = scrollSpeed;
}

const hkvVec2 &Sprite::GetScrollSpeed() const
{
	return m_scrollSpeed;
}

void Sprite::SetFullscreenMode(bool enabled)
{
	m_fullscreen = enabled;
}

bool Sprite::IsFullscreenMode() const
{
	return m_fullscreen;
}

void Sprite::SetPlayOnce(bool enabled)
{
	m_playOnce = enabled;
}

bool Sprite::IsPlayOnce() const
{
	return m_playOnce;
}

void Sprite::SetCollision(bool enabled)
{
	m_collide = enabled;
}

bool Sprite::IsColliding() const
{
	return m_collide && !IsFullscreenMode();
}

void Sprite::SetCenterPosition(const hkvVec3 &position)
{
	const hkvVec2 dimensions = GetDimensions();
	SetPosition(position.x - dimensions.x / 2.f, position.y - dimensions.y / 2.f, position.z);
}

hkvVec3 Sprite::GetCenterPosition()
{
	const hkvVec2 dimensions = GetDimensions();
	const hkvVec3 &position = GetPosition();
	return hkvVec3(position.x + dimensions.x / 2.f, position.y + dimensions.y / 2.f, position.z);
}

float Sprite::GetWidth() const
{
	hkvVec3 scale = GetScaling();
	return GetDimensions().x * scale.x;
}

float Sprite::GetHeight() const
{
	hkvVec3 scale = GetScaling();
	return GetDimensions().y * scale.y;
}

void Sprite::SetWidth(float width)
{
	hkvVec3 scale = GetScaling();
	scale.x = width / GetDimensions().x;
	SetScaling(scale);
}

void Sprite::SetHeight(float height)
{
	hkvVec3 scale = GetScaling();
	scale.y = height / GetDimensions().y;
	SetScaling(scale);
}

hkvVec2 Sprite::GetDimensions() const
{
	float width = 0.f;
	float height = 0.f;
	VTextureObject *texture = GetTexture();

	if (m_currentState != -1)
	{
		const SpriteState *spriteState = &m_spriteData->states[m_currentState];
		const SpriteCell *cell = &m_spriteData->cells[spriteState->cells[m_currentFrame]];
		width = static_cast<float>(cell->originalWidth);
		height = static_cast<float>(cell->originalHeight);
	}
	else if (texture != NULL)
	{
		width = static_cast<float>(texture->GetTextureWidth());
		height = static_cast<float>(texture->GetTextureHeight());
	}
	
	return hkvVec2(width, height);
}

const SpriteCell *Sprite::GetCurrentCell() const
{
	const SpriteCell *cell = NULL;
	if (m_spriteData)
	{
		cell = &m_spriteData->cells[m_spriteData->states[m_currentState].cells[m_currentFrame]];
	}
	return cell;
}

hkvVec3 Sprite::GetPoint(float x, float y, float z) const
{
	hkvVec2 point(x, y);

	if (m_currentState != -1)
	{
		const SpriteState *spriteState = &m_spriteData->states[m_currentState];
		const SpriteCell *cell = &m_spriteData->cells[spriteState->cells[m_currentFrame]];
		const float width = static_cast<float>(cell->originalWidth);
		const float height = static_cast<float>(cell->originalHeight);

		hkvVec2 offset(width / 2.0f, height / 2.0f);

		point -= offset;

		// rotate the corners
		hkvMat3 rotation;
		rotation.setRotationMatrixZ(m_vOrientation.z);

		const hkvVec2 &scale = GetScaling().getAsVec2();

		point = (rotation * point.getAsVec3(0.0f)).getAsVec2().compMul(scale);

		// offset it back to the corner and final position
		point += offset;

		point += GetPosition().getAsVec2();
	}

	return point.getAsVec3(z);
}

void Sprite::OnVariableValueChanged(VisVariable_cl *pVar, const char *value)
{
	if ( !strcmp(pVar->name, "TextureFilename") )
	{
		if (value &&
			value[0] &&
			m_spriteSheetFilename != value)
		{
			m_spriteSheetFilename = value;
			UpdateTextures();
		}
	}
	else if ( !strcmp(pVar->name, "XmlDataFilename") )
	{
		if (value &&
			value[0] &&
			m_xmlDataFilename != value)
		{
			m_xmlDataFilename = value;
			UpdateTextures();
		}
	}
}

void Sprite::DebugRender(IVRenderInterface *pRenderer, float fSize, VColorRef iColor, bool bRenderConnections) const
{
	VSimpleRenderState_t state(VIS_TRANSP_NONE, RENDERSTATEFLAG_LINE2D);
	int edges[8] = {0, 1,
					1, 3,
					3, 2,
					2, 0};

	for (int i = 0; i < 4; i++)
	{
		const int p1 = edges[i * 2 + 0];
		const int p2 = edges[i * 2 + 1];
		const float z = 100.f;
		pRenderer->DrawLine(m_vertices[p1].getAsVec3(z), m_vertices[p2].getAsVec3(z), VColorRef(0, 0, 255), 5.0f);
	}
}

VTextureObject *Sprite::GetTexture() const
{
	VTextureObject *texture = NULL;
	if (m_spriteData != NULL)
	{
		texture = m_spriteData->spriteSheetTexture;
		if (m_spriteData->spTextureAnimation)
		{
			texture = m_spriteData->spTextureAnimation->GetCurrentFrame();
		}
	}
	return texture;
}

void Sprite::Update()
{
	if (m_spriteData == NULL)
	{
		return;
	}

	const VTextureObject *texture = GetTexture();

	float width = static_cast<float>(m_spriteData->sourceWidth);
	float height = static_cast<float>(m_spriteData->sourceHeight);

	hkvVec2 worldPosition = GetPosition().getAsVec2();	
	hkvVec2 topLeft;
	hkvVec2 bottomRight;
	hkvVec2 uvTopLeft(0, 0);
	hkvVec2 uvBottomRight(1, 1);

	if (m_currentState != -1)
	{
		const SpriteState *spriteState = &m_spriteData->states[m_currentState];
		const SpriteCell *cell = &m_spriteData->cells[spriteState->cells[m_currentFrame]];

		float w = (float)cell->width / width;
		float h = (float)cell->height / height;
		float x = cell->offset.x / width;
		float y = cell->offset.y / height;

		uvTopLeft.x = x + m_scrollOffset.x;
		uvTopLeft.y = y + m_scrollOffset.y;
		uvBottomRight.x = uvTopLeft.x + w;
		uvBottomRight.y = uvTopLeft.y + h;

		if (IsFullscreenMode())
		{
			topLeft.x = 0;
			topLeft.y = 0;

			{
				int x, y, w, h;
				Vision::Contexts.GetMainRenderContext()->GetViewport(x, y, w, h);

				// Stretch it out to make sure it fits the screen
				if (w - width > h - height)
				{
					bottomRight.x = static_cast<float>(w);
					bottomRight.y = static_cast<float>(w * height) / width;
				}
				else
				{
					bottomRight.y = static_cast<float>(h);
					bottomRight.x = static_cast<float>(width * h) / height;
				}
			}

			uvTopLeft.x += worldPosition.x / width;
			uvBottomRight.x += worldPosition.x / width;
			uvTopLeft.y += worldPosition.y / height;
			uvBottomRight.y += worldPosition.y / height;

			m_vertices[VERTEX_TOP_LEFT] = topLeft;
			m_vertices[VERTEX_TOP_RIGHT] = hkvVec2(bottomRight.x, topLeft.y);
			m_vertices[VERTEX_BOTTOM_LEFT] = hkvVec2(topLeft.x, bottomRight.y);
			m_vertices[VERTEX_BOTTOM_RIGHT] = bottomRight;
		}
		else
		{
			const hkvVec2 &scale = GetScaling().getAsVec2();

			topLeft = cell->pivot;
			bottomRight.x = topLeft.x + cell->width;
			bottomRight.y = topLeft.y + cell->height;

			const float ww = bottomRight.x - topLeft.x;
			const float hh = bottomRight.y - topLeft.y;
			hkvVec2 offset(cell->originalWidth / 2.0f, cell->originalHeight / 2.0f);

			// offset it so that we rotate around the center of the shape
			topLeft -= offset;
			bottomRight -= offset;

			hkvVec2 topRight = topLeft;
			topRight.x += ww;

			hkvVec2 bottomLeft = bottomRight;
			bottomLeft.x -= ww;

			// generate a matrix that rotates around Z
			hkvMat3 rotation;
			rotation.setRotationMatrixZ(m_vOrientation.z);
			
			// rotate all the corners
			topLeft = (rotation * topLeft.getAsVec3(0.0f)).getAsVec2().compMul(scale);
			bottomRight = (rotation * bottomRight.getAsVec3(0.0f)).getAsVec2().compMul(scale);
			topRight = (rotation * topRight.getAsVec3(0.0f)).getAsVec2().compMul(scale);
			bottomLeft = (rotation * bottomLeft.getAsVec3(0.0f)).getAsVec2().compMul(scale);
			
			// offset it back to the corner and final position
			topLeft += worldPosition + offset;
			bottomRight += worldPosition + offset;
			topRight += worldPosition + offset;
			bottomLeft += worldPosition + offset;

			// save off the final position
			m_vertices[VERTEX_TOP_LEFT] = topLeft;
			m_vertices[VERTEX_TOP_RIGHT] = topRight;
			m_vertices[VERTEX_BOTTOM_LEFT] = bottomLeft;
			m_vertices[VERTEX_BOTTOM_RIGHT] = bottomRight;
		}
	}
	else
	{
		bottomRight.x = topLeft.x + width;
		bottomRight.y = topLeft.y + height;

		m_vertices[VERTEX_TOP_LEFT] = topLeft;
		m_vertices[VERTEX_TOP_RIGHT] = hkvVec2(bottomRight.x, topLeft.y);
		m_vertices[VERTEX_BOTTOM_LEFT] = hkvVec2(topLeft.x, bottomRight.y);
		m_vertices[VERTEX_BOTTOM_RIGHT] = bottomRight;
	}

	m_texCoords[VERTEX_TOP_LEFT] = uvTopLeft;
	m_texCoords[VERTEX_TOP_RIGHT] = hkvVec2(uvBottomRight.x, uvTopLeft.y);
	m_texCoords[VERTEX_BOTTOM_LEFT] = hkvVec2(uvTopLeft.x, uvBottomRight.y);
	m_texCoords[VERTEX_BOTTOM_RIGHT] = uvBottomRight;

	// Setup the render vertices
	m_renderVertices[0].Set(m_vertices[VERTEX_TOP_LEFT].x, m_vertices[VERTEX_TOP_LEFT].y, m_texCoords[VERTEX_TOP_LEFT].x, m_texCoords[VERTEX_TOP_LEFT].y);
	m_renderVertices[1].Set(m_vertices[VERTEX_BOTTOM_LEFT].x, m_vertices[VERTEX_BOTTOM_LEFT].y, m_texCoords[VERTEX_BOTTOM_LEFT].x, m_texCoords[VERTEX_BOTTOM_LEFT].y);
	m_renderVertices[2].Set(m_vertices[VERTEX_TOP_RIGHT].x, m_vertices[VERTEX_TOP_RIGHT].y, m_texCoords[VERTEX_TOP_RIGHT].x, m_texCoords[VERTEX_TOP_RIGHT].y);
	m_renderVertices[3].Set(m_vertices[VERTEX_TOP_RIGHT].x, m_vertices[VERTEX_TOP_RIGHT].y, m_texCoords[VERTEX_TOP_RIGHT].x, m_texCoords[VERTEX_TOP_RIGHT].y);
	m_renderVertices[4].Set(m_vertices[VERTEX_BOTTOM_LEFT].x, m_vertices[VERTEX_BOTTOM_LEFT].y, m_texCoords[VERTEX_BOTTOM_LEFT].x, m_texCoords[VERTEX_BOTTOM_LEFT].y);
	m_renderVertices[5].Set(m_vertices[VERTEX_BOTTOM_RIGHT].x, m_vertices[VERTEX_BOTTOM_RIGHT].y, m_texCoords[VERTEX_BOTTOM_RIGHT].x, m_texCoords[VERTEX_BOTTOM_RIGHT].y);
}

void Sprite::Render(IVRender2DInterface *pRender, VSimpleRenderState_t& state)
{
	if ( m_spriteData != NULL && (GetVisibleBitmask() & VIS_ENTITY_VISIBLE) )
	{
		pRender->Draw2DBuffer(6, m_renderVertices, GetTexture(), state);
	}
}

void Sprite::OnCollision(Sprite *other)
{
	this->TriggerScriptEvent("OnSpriteCollision", "*o", other);
}

bool Sprite::IsOverlapping(Sprite *other) const
{
	const hkvVec2 *otherVertices = other->GetVertices();
	bool inside = true;

	int edges[8] = {0, 1,
					1, 3,
					3, 2,
					2, 0};

	// Loop through each vertex and see if any of them are inside all
	// of the edges
	for (int vertexIndex = 0; vertexIndex < 4; vertexIndex++)
	{
		inside = true;

		// Test each edge
		for (int edgeIndex = 0; edgeIndex < 4; edgeIndex++)
		{
			int p1 = edges[edgeIndex * 2 + 0];
			int p2 = edges[edgeIndex * 2 + 1];

			const hkvVec3 e = (m_vertices[p2] - m_vertices[p1]).getNormalized().getAsVec3(0.f);
			const hkvVec3 l = (otherVertices[vertexIndex] - m_vertices[p1]).getNormalized().getAsVec3(0.f);
			const hkvVec3 up = e.cross(l);

			if (up.z < 0)
			{
				inside = false;
				break;
			}
		}

		if (inside)
		{
			break;
		}
	}
	
	return inside;
}

void Sprite::Serialize(VArchive &ar)
{
	VisBaseEntity_cl::Serialize(ar);

	if (ar.IsLoading())
	{
		Clear();

		char spriteVersion;
		ar >> spriteVersion;
		VASSERT(spriteVersion <= CURRENT_SPRITE_VERSION);

		char spriteSheetBuffer[FS_MAX_PATH + 1];
		ar.ReadStringBinary(spriteSheetBuffer, FS_MAX_PATH);
		m_spriteSheetFilename = spriteSheetBuffer;

		char xmlFilenameBuffer[FS_MAX_PATH + 1];
		ar.ReadStringBinary(xmlFilenameBuffer, FS_MAX_PATH);
		m_xmlDataFilename = xmlFilenameBuffer;

		ar >> m_scrollSpeed.x;
		ar >> m_scrollSpeed.y;
		ar >> m_fullscreen;
		ar >> m_playOnce;
		ar >> m_collide;
	} 
	else
	{
		ar << (char)CURRENT_SPRITE_VERSION;

		ar.WriteStringBinary(m_spriteSheetFilename);
		ar.WriteStringBinary(m_xmlDataFilename);

		ar << m_scrollSpeed.x;
		ar << m_scrollSpeed.y;
		ar << m_fullscreen;
		ar << m_playOnce;
		ar << m_collide;
	}
}

void Sprite::OnSerialized(VArchive &ar)
{
	VisBaseEntity_cl::OnSerialized(ar);

	CommonInit();
}

START_VAR_TABLE(Sprite, VisBaseEntity_cl, "Sprite", 0, "")
	DEFINE_VAR_STRING_CALLBACK(Sprite, TextureFilename, "Sprite sheet", "white.dds", DISPLAY_HINT_TEXTUREFILE, NULL);
	DEFINE_VAR_STRING_CALLBACK(Sprite, XmlDataFilename, "Xml Data", "", DISPLAY_HINT_CUSTOMFILE, NULL);
END_VAR_TABLE