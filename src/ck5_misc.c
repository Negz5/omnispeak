#include "id_ca.h"
#include "id_in.h"
#include "id_rf.h"
#include "ck_play.h"
#include "ck_phys.h"
#include "ck_def.h"

#include <stdio.h>

// Contains some keen-5 specific functions.


void CK5_TurretSpawn(int tileX, int tileY, int direction);
void CK5_SpawnKorath(int tileX, int tileY);

//StartSprites + 
int CK5_ItemSpriteChunks[] = 
{
	122, 124, 126, 128,
	108, 110, 112, 114, 116, 118,
	120, 131, 105
};

int CK5_ItemNotifyChunks[] =
{
	232, 232, 232, 232,
	195, 196, 197, 198, 199, 200,
	201, 202, 209
};

void CK_BasicDrawFunc1(CK_object *obj)
{
	RF_AddSpriteDraw(&(obj->sde), obj->posX, obj->posY, obj->gfxChunk, false, obj->zLayer); 
}

void CK5_PointItem(CK_object *obj)
{
//	obj->timeUntillThink = 20;
	obj->visible = true;
	if (++obj->gfxChunk == obj->user3)
		obj->gfxChunk = obj->user2;
}

//TODO: Collision boxes.

void CK5_BlockPlatform(CK_object *obj)
{
	int nextPosUnit, nextPosTile;

	if (obj->nextX || obj->nextY) {
		return;
	}
	//TODO: Implement properly.
	obj->nextX = obj->xDirection * 12;
	obj->nextY = obj->yDirection * 12;

	CK_ResetClipRects(obj);
	
	if (obj->xDirection == 1)
	{
		nextPosUnit = obj->clipRects.unitX2 + 12;
		nextPosTile = nextPosUnit >> 8;
		if (obj->clipRects.tileX2 != nextPosTile && CA_mapPlanes[2][CA_MapHeaders[ck_currentMapNumber]->width*obj->clipRects.tileY1+nextPosTile] == 0x1F)
		{
			obj->xDirection = -1;
			//TODO: Change DeltaVelocity
			obj->nextX -= (nextPosUnit&255);
		}
	}
	else if (obj->xDirection == -1)
	{
		nextPosUnit = obj->clipRects.unitX1 - 12;
		nextPosTile = nextPosUnit >> 8;
		if (obj->clipRects.tileX1 != nextPosTile && CA_mapPlanes[2][CA_MapHeaders[ck_currentMapNumber]->width*obj->clipRects.tileY1+nextPosTile] == 0x1F)
		{
			obj->xDirection = 1;
			//TODO: Change DeltaVelocity
			//CK_PhysUpdateX(obj, 256 - nextPosUnit&255);
			obj->nextX += 256 - nextPosUnit&255;
		}
	}
	else if (obj->yDirection == 1)
	{
		nextPosUnit = obj->clipRects.unitY2 + 12;
		nextPosTile = nextPosUnit >> 8;
		if (obj->clipRects.tileY2 != nextPosTile && CA_mapPlanes[2][CA_MapHeaders[ck_currentMapNumber]->width*nextPosTile+obj->clipRects.tileX1+obj->user1] == 0x1F)
		{
			if (CA_TileAtPos(obj->clipRects.unitX1, nextPosTile-2,2) == 0x1F)
			{
				//Stop the platform.
				obj->yDirection = 0;
				obj->visible = true;
				obj->nextY = 0;
			}
			else
			{
				obj->yDirection = -1;
				//TODO: Change DeltaVelocity
				obj->nextY -= ( nextPosUnit&255);
			}
		}
	}
	else if (obj->yDirection == -1)
	{
		nextPosUnit = obj->clipRects.unitY1 - 12;
		nextPosTile = nextPosUnit >> 8;
		if (obj->clipRects.tileY1 != nextPosTile && CA_mapPlanes[2][CA_MapHeaders[ck_currentMapNumber]->width*nextPosTile+obj->clipRects.tileX1+obj->user1] == 0x1F)
		{
			if (CA_TileAtPos(obj->clipRects.unitX1, nextPosTile+2, 2) == 0x1F)
			{
				// Stop the platform.
				obj->yDirection = 0;
				obj->visible = true;
				obj->nextY = 0;
			}
			else
			{
				obj->yDirection = 1;
				//TODO: Change DeltaVelocity
				obj->nextY +=  256-(nextPosUnit&255);
			}
		}
	}
	obj->visible = true;
}


CK_action CK_act_item2;
CK_action CK_act_item = {0, 0, AT_UnscaledOnce, 0, 0, 20, 0, 0, &CK5_PointItem, 0,  &CK_BasicDrawFunc1, &CK_act_item2};
CK_action CK_act_item2 = {0, 0, AT_UnscaledOnce, 0, 0, 20, 0, 0, &CK5_PointItem, 0,  &CK_BasicDrawFunc1, &CK_act_item};
CK_action CK_act_redBlockPlatform = {446, 446, AT_Frame, 0, 0, 0, 0, 0, &CK5_BlockPlatform, 0, &CK_BasicDrawFunc1, 0};
CK_action CK_act_purpleBlockPlatform = {423, 423, AT_UnscaledFrame, 0, 0, 0, 0, 0, &CK5_BlockPlatform, 0, &CK_BasicDrawFunc1, 0};

void CK5_SpawnItem(int tileX, int tileY, int itemNumber)
{
	CK_object *obj = CK_GetNewObj(false);

	obj->clipped = false;
	obj->visible = true;
	obj->active = true;
	obj->type = 4;	//OBJ_ITEM
	obj->zLayer = 3;
	obj->posX = tileX << 8;
	obj->posY = tileY << 8;
	obj->user1 = itemNumber;
	obj->gfxChunk = CK5_ItemSpriteChunks[itemNumber] + ca_gfxInfoE.offSprites;	
	obj->user2 = obj->gfxChunk;
	obj->user3 = obj->gfxChunk + 2;
	obj->user4 = CK5_ItemNotifyChunks[itemNumber];
	//obj->currentAction = &CK_act_item;
	CK_SetAction(obj, &CK_act_item);
	CA_CacheGrChunk(obj->gfxChunk);
	CA_CacheGrChunk(obj->gfxChunk+1);
	//printf("Adding item %d at (%d, %d)\n", itemNumber, tileX, tileY);
}

void CK5_SpawnRedBlockPlatform(int tileX, int tileY, int direction, bool purple)
{
	CK_object *obj = CK_GetNewObj(false);

	obj->type = 0;
	obj->active = true;
	obj->visible = true;
	obj->zLayer = 0;
	obj->posX = tileX << 8;
	obj->posY = tileY << 8;
	obj->user1 = obj->user2 = 0;

	switch(direction)
	{
	case 0:
		obj->xDirection = 0;
		obj->yDirection = -1;
		break;
	case 1:
		obj->xDirection = 1;
		obj->yDirection = 0;
		break;
	case 2:
		obj->xDirection = 0;
		obj->yDirection = 1;
		break;
	case 3:
		obj->xDirection = -1;
		obj->yDirection = 0;
		break;
	}

	if (purple)
	{
		obj->user1 = 1;
		obj->posX += 0x40;
		obj->posY += 0x40;
		CK_SetAction(obj,&CK_act_purpleBlockPlatform);
	}
	else
	{
		obj->user1 = 0;	
		CK_SetAction(obj,&CK_act_redBlockPlatform);
	}
	obj->gfxChunk = obj->currentAction->chunkLeft;
	CA_CacheGrChunk(obj->gfxChunk);
	CK_ResetClipRects(obj);
	printf("Spwaning platform %d at %d %d in dir %d\n", purple, tileX, tileY, direction);
}




void CK5_ScanInfoLayer()
{

	//TODO: Work out where to store current map number, etc.
	int mapW = CA_MapHeaders[ck_currentMapNumber]->width;
	int mapH = CA_MapHeaders[ck_currentMapNumber]->height;

	for (int y = 0; y < mapH; ++y)
	{
		for (int x = 0; x < mapW; ++x)
		{
			int infoValue = CA_mapPlanes[2][y*mapW+x];
			switch (infoValue)
			{
			case 1:
				CK_SpawnKeen(x,y,1);
				break;
			case 2:
				CK_SpawnKeen(x,y,-1);
				break;
				
			case 25:
				RF_SetScrollBlock(x,y,true);
				break;
			case 26:
				RF_SetScrollBlock(x,y,false);
				break;
			case 27:
			case 28:
			case 29:
			case 30:
				CK5_SpawnRedBlockPlatform(x,y,infoValue - 27, false);
				break;
			case 45:
			case 46:
			case 47:
			case 48:
				CK5_TurretSpawn(x,y,infoValue-45);

			case 57:
			case 58:
			case 59:
			case 60:
			case 61:
			case 62:
			case 63:
			case 64:
			case 65:
			case 66:
			case 67:
			case 68:
				CK5_SpawnItem(x,y, infoValue - 57);
				break;
			case 70:
				CK5_SpawnItem(x,y, infoValue - 58); // Omegamatic Keycard
				break;


			case 84:
			case 85:
			case 86:
			case 87:
				CK5_SpawnRedBlockPlatform(x,y,infoValue-84,true);
				break;

			case 124:
				CK5_SpawnKorath(x,y);
				break;
			}
		}
	}
}
