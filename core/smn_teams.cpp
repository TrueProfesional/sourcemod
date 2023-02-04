/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * =============================================================================
 * SourceMod
 * Copyright (C) 2004-2010 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#include "sourcemod.h"
#include "HalfLife2.h"
#include "logic_bridge.h"
#include <IGameConfigs.h>

const char *m_iScore;

static cell_t GetTeamCount(IPluginContext *pContext, const cell_t *params)
{
	return g_HL2.GetTeamInfoVector().size();
}

static cell_t GetTeamName(IPluginContext *pContext, const cell_t *params)
{
	int teamindex = params[1];
	if (teamindex >= (int)g_HL2.GetTeamInfoVector().size() || !g_HL2.GetTeamInfo(teamindex).ClassName)
		return pContext->ThrowNativeError("Team index %d is invalid", teamindex);

	if (g_HL2.GetTeamnameOffset() == 0)
		return pContext->ThrowNativeError("Team names are not available on this game.");

	const char *name = g_HL2.GetTeamName(teamindex);
	if (name == nullptr)
		return pContext->ThrowNativeError("Team names are not available on this game.");

	pContext->StringToLocalUTF8(params[2], params[3], name, nullptr);

	return 1;
}

static cell_t GetTeamScore(IPluginContext *pContext, const cell_t *params)
{
	int teamindex = params[1];
	if (teamindex >= (int)g_HL2.GetTeamInfoVector().size() || !g_HL2.GetTeamInfo(teamindex).ClassName)
	{
		return pContext->ThrowNativeError("Team index %d is invalid", teamindex);
	}

	if (!m_iScore)
	{
		m_iScore = g_pGameConf->GetKeyValue("m_iScore");
		if (!m_iScore)
		{
			return pContext->ThrowNativeError("Failed to get m_iScore key");
		}
	}

	static int offset = -1;

	if (offset == -1)
	{
		SendProp *prop = g_HL2.FindInSendTable(g_HL2.GetTeamInfo(teamindex).ClassName, m_iScore);
		if (!prop)
		{
			return pContext->ThrowNativeError("Failed to get m_iScore prop");
		}
		offset = prop->GetOffset();
	}


	return *(int *)((unsigned char *)g_HL2.GetTeamInfo(teamindex).pEnt + offset);
}

static cell_t SetTeamScore(IPluginContext *pContext, const cell_t *params)
{
	if (!g_SourceMod.IsMapRunning())
	{
		return pContext->ThrowNativeError("Cannot set team score when no map is running");
	}
	
	int teamindex = params[1];
	if (teamindex >= (int)g_HL2.GetTeamInfoVector().size() || !g_HL2.GetTeamInfo(teamindex).ClassName)
	{
		return pContext->ThrowNativeError("Team index %d is invalid", teamindex);
	}

	if (m_iScore == nullptr)
	{
		m_iScore = g_pGameConf->GetKeyValue("m_iScore");
		if (m_iScore == nullptr)
		{
			return pContext->ThrowNativeError("Failed to get m_iScore key");
		}
	}

	static int offset = -1;

	if (offset == -1)
	{
		SendProp *prop = g_HL2.FindInSendTable(g_HL2.GetTeamInfo(teamindex).ClassName, m_iScore);
		if (!prop)
		{
			return pContext->ThrowNativeError("Failed to get m_iScore prop");
		}
		offset = prop->GetOffset();
	}

	CBaseEntity *pTeam = g_HL2.GetTeamInfo(teamindex).pEnt;
	*(int *)((unsigned char *)pTeam + offset) = params[2];

	edict_t *pEdict;
	g_HL2.SetEdictStateChanged(pEdict, offset);

	return 1;
}

static cell_t GetTeamClientCount(IPluginContext *pContext, const cell_t *params)
{
	int teamindex = params[1];
	if (teamindex >= (int)g_HL2.GetTeamInfoVector().size() || !g_HL2.GetTeamInfo(teamindex).ClassName)
	{
		return pContext->ThrowNativeError("Team index %d is invalid", teamindex);
	}

	SendProp *pProp = g_HL2.FindInSendTable(g_HL2.GetTeamInfo(teamindex).ClassName, "\"player_array\"");
	ArrayLengthSendProxyFn fn = pProp->GetArrayLengthProxy();

	return fn(g_HL2.GetTeamInfo(teamindex).pEnt, 0);
}

static cell_t GetTeamEntity(IPluginContext *pContext, const cell_t *params)
{
	int teamindex = params[1];
	if (teamindex >= (int)g_HL2.GetTeamInfoVector().size() || !g_HL2.GetTeamInfo(teamindex).ClassName)
	{
		return pContext->ThrowNativeError("Team index %d is invalid", teamindex);
	}

	return g_HL2.EntityToBCompatRef(g_HL2.GetTeamInfo(teamindex).pEnt);
}

REGISTER_NATIVES(teamNatives)
{
	{"GetTeamCount",			GetTeamCount},
	{"GetTeamName",				GetTeamName},
	{"GetTeamScore",			GetTeamScore},
	{"SetTeamScore",			SetTeamScore},
	{"GetTeamClientCount",		GetTeamClientCount},
	{"GetTeamEntity",			GetTeamEntity},
	{NULL,						NULL}
};
