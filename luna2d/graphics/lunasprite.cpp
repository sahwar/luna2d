//-----------------------------------------------------------------------------
// luna2d engine
// Copyright 2014 Stepan Prokofjev
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "lunasprite.h"
#include "lunaengine.h"
#include "lunagraphics.h"
#include "lunasizes.h"
#include "lunaassets.h"
#include "lunalog.h"
#include <cmath>

using namespace luna2d;

LUNASprite::LUNASprite(int assetId) :
	x(0),
	y(0),
	originX(0),
	originY(0),
	width(0),
	height(0),
	scaleX(1),
	scaleY(1),
	angle(0),
	u1(0),
	v1(0),
	u2(0),
	v2(0),
	color(LUNAColor::WHITE)
{
	LUNAAssets* assets = LUNAEngine::SharedAssets();

	// Create sprite from texture
	if(assets->IsAssetA(assetId, LUNAAssetType::TEXTURE))
	{
		texture = assets->GetAsset<LUNATexture>(assetId, LUNAAssetType::TEXTURE);
		if(texture.expired()) return;

		u1 = 0;
		v1 = 0;
		u2 = 1;
		v2 = 1;

		// Convert sizes to virtual resolution
		auto sharedTexture = texture.lock();
		width = std::floor(sharedTexture->GetWidth() * LUNAEngine::SharedSizes()->GetTextureScale());
		height = std::floor(sharedTexture->GetHeight() * LUNAEngine::SharedSizes()->GetTextureScale());
		return;
	}

	// Create sprite from texture region
	if(assets->IsAssetA(assetId, LUNAAssetType::TEXTURE_REGION))
	{
		auto weakRegion = assets->GetAsset<LUNATextureRegion>(assetId, LUNAAssetType::TEXTURE_REGION);
		if(weakRegion.expired()) return;

		auto region = weakRegion.lock();
		texture = region->GetTexture();
		u1 = region->GetU1();
		v1 = region->GetV1();
		u2 = region->GetU2();
		v2 = region->GetV2();

		// Convert sizes to virtual resolution
		width = std::floor(region->GetWidth() * LUNAEngine::SharedSizes()->GetTextureScale());
		height = std::floor(region->GetHeight() * LUNAEngine::SharedSizes()->GetTextureScale());
		return;
	}

	LUNA_LOGE("Asset with id \"%d\" is not texture or texture region", assetId);
}

float LUNASprite::GetX()
{
	return x;
}

float LUNASprite::GetY()
{
	return y;
}

void LUNASprite::SetX(float x)
{
	this->x = x;
}

void LUNASprite::SetY(float y)
{
	this->y = y;
}

LuaTable LUNASprite::GetPos()
{
	LuaTable tblPos(LUNAEngine::SharedLua());
	tblPos.SetField("x", x);
	tblPos.SetField("y", y);

	return std::move(tblPos);
}

void LUNASprite::SetPos(float x, float y)
{
	SetX(x);
	SetY(y);
}

float LUNASprite::GetWidth()
{
	return width;
}

float LUNASprite::GetHeight()
{
	return height;
}

void LUNASprite::SetWidth(float width)
{
	this->width = width;
}

void LUNASprite::SetHeight(float height)
{
	this->height = height;
}

void LUNASprite::SetSize(float width, float height)
{
	SetWidth(width);
	SetHeight(height);
}

float LUNASprite::GetOriginX()
{
	return originX;
}

float LUNASprite::GetOriginY()
{
	return originY;
}

void LUNASprite::SetOriginX(float originX)
{
	this->originX = originX;
}

void LUNASprite::SetOriginY(float originY)
{
	this->originY = originY;
}

void LUNASprite::SetOrigin(float originX, float originY)
{
	SetOriginX(originX);
	SetOriginY(originY);
}

// Set origin to center of sprite
void LUNASprite::SetOriginToCenter()
{
	SetOrigin(width / 2, height / 2);
}

float LUNASprite::GetScaleX()
{
	return scaleX;
}

float LUNASprite::GetScaleY()
{
	return scaleY;
}

void LUNASprite::SetScaleX(float scaleX)
{
	this->scaleX = scaleX;
}

void LUNASprite::SetScaleY(float scaleY)
{
	this->scaleY = scaleY;
}

void LUNASprite::SetScale(float scale)
{
	SetScaleX(scale);
	SetScaleY(scale);
}

void LUNASprite::Render()
{
	if(texture.expired())
	{
		LUNA_LOGE("Attempt to render invalid sprite");
		return;
	}

	LUNARenderer *renderer = LUNAEngine::SharedGraphics()->GetRenderer();

	// Sprite geometry
	float x1 = 0;
	float y1 = 0;
	float x2 = 0;
	float y2 = height * scaleY;
	float x3 = width * scaleX;
	float y3 = height * scaleY;
	float x4 = width * scaleX;
	float y4 = 0;

	// Offset by origin
	float offsetX = originX * scaleX;
	float offsetY = originY * scaleY;
	x1 -= offsetX;
	y1 -= offsetY;
	x2 -= offsetX;
	y2 -= offsetY;
	x3 -= offsetX;
	y3 -= offsetY;
	x4 -= offsetX;
	y4 -= offsetY;

	// Rotate
	if(angle != 0)
	{
		float sin = std::sin(angle * M_PI / 180.0f);
		float cos = std::cos(angle * M_PI / 180.0f);

		float rx1 = x1 * cos - y1 * sin;
		float ry1 = x1 * sin + y1 * cos;
		float rx2 = x2 * cos - y2 * sin;
		float ry2 = x2 * sin + y2 * cos;
		float rx3 = x3 * cos - y3 * sin;
		float ry3 = x3 * sin + y3 * cos;
		float rx4 = x4 * cos - y4 * sin;
		float ry4 = x4 * sin + y4 * cos;

		x1 = rx1;
		y1 = ry1;
		x2 = rx2;
		y2 = ry2;
		x3 = rx3;
		y3 = ry3;
		x4 = rx4;
		y4 = ry4;
	}

	renderer->RenderQuad(texture.lock().get(),
		x + x1, y + y1, u1, v2, x + x2, y + y2, u1, v1, x + x3, y + y3, u2, v1, x + x4, y + y4, u2, v2, color);
}

// Get rotation angle (in degrees)
float LUNASprite::GetAngle()
{
	return angle;
}

// Set rotation angle (in degrees)
void LUNASprite::SetAngle(float angle)
{
	this->angle = angle;
}

void LUNASprite::SetColor(float r, float g, float b)
{
	color.r = r / 255.0f;
	color.g = g / 255.0f;
	color.b = b / 255.0f;
}

LuaTable LUNASprite::GetColor()
{
	LuaTable tblColor(LUNAEngine::SharedLua());
	tblColor.SetField("r", color.r * 255.0f);
	tblColor.SetField("g", color.g * 255.0f);
	tblColor.SetField("b", color.b * 255.0f);

	return std::move(tblColor);
}

void LUNASprite::SetAlpha(float alpha)
{
	color.a = alpha;
}

float LUNASprite::GetAlpha()
{
	return color.a;
}