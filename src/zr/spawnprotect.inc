/*
 * ============================================================================
 *
 *   Zombie:Reloaded
 *
 *   File:        spawnprotect.inc
 *   Description: Protects late-joining players from zombies for x seconds.
 *
 * ============================================================================
 */

/**
 * Array for storing spawn protect timer handles per client.
 */
new Handle:tSpawnProtect[MAXPLAYERS + 1];

/**
 * Array for flagging client to be protected.
 */ 
new bool:pSpawnProtect[MAXPLAYERS + 1];

/**
 * Array for storing time left for spawn protection per client.
 */
new pSpawnProtectTime[MAXPLAYERS + 1];
 
/**
 * Client is joining the server.
 * 
 * @param client    The client index.
 */ 
SpawnProtectClientInit(client)
{
    tSpawnProtect[client] = INVALID_HANDLE;
}

/**
 * Player is spawning into the game.
 * 
 * @param client    The client index.
 */   
SpawnProtectPlayerSpawn(client)
{
    // Disable spawn protection on client.
    pSpawnProtect[client] = false;
    
    // If zombie hasn't spawned, then stop.
    if (!zombieSpawned)
    {
        return;
    }
    
    // If protect cvar is invalid or 0, then stop.
    new protect = GetConVarInt(gCvars[CVAR_PROTECT]);
    if (protect <= 0)
    {
        return;
    }
    
    // Get respawn team.
    decl String:respawnteam[32];
    GetConVarString(gCvars[CVAR_RESPAWN_TEAM], respawnteam, sizeof(respawnteam));
    
    // If the respawn team is not set to zombie, and either cvar zr_suicide_world_damage or the client
    // wasn't killed by world is false, then continue to protect client.
    if (!StrEqual(respawnteam, "zombie", false) && !(GetConVarBool(gCvars[CVAR_SUICIDE_WORLD_DAMAGE]) && gKilledByWorld[client]))
    {
        // Set spawn protect flag on client.
        pSpawnProtect[client] = true;
        
        // Set improved attributes
        // (Move to cvar?)
        SetPlayerAlpha(client, 0);
        SetPlayerSpeed(client, 600.0);
        
        // Set time left to zr_protect's value.
        pSpawnProtectTime[client] = protect;
        
        // Tell client they are being protected.
        ZR_PrintToChat(client, "Spawn protection begin", protect);
        
        // Send time left in a hud message.
        ZR_HudHint(client, "Spawn Protect", pSpawnProtectTime[client]);
        
        // If timer is currently running, kill it.
        if (tSpawnProtect[client] != INVALID_HANDLE)
        {
            KillTimer(tSpawnProtect[client]);
        }
        
        // Start repeating timer.
        tSpawnProtect[client] = CreateTimer(1.0, SpawnProtectTimer, client, TIMER_FLAG_NO_MAPCHANGE|TIMER_REPEAT);
    }
}

/**
 * Timer callback function, countdown for spawn protection.
 * 
 * @param timer     The timer handle.
 * @param client    The client index.
 */    
public Action:SpawnProtectTimer(Handle:timer, any:client)
{
    // If client leaves, then stop timer.
    if (!IsClientInGame(client))
    {
        return Plugin_Stop;
    }
    
    // If client has become a zombie, then stop timer.
    if (!IsPlayerHuman(client))
    {
        return Plugin_Stop;
    }
    
    // Decrement time left.
    pSpawnProtectTime[client]--;
    
    // Print time left to client.
    ZR_HudHint(client, "Spawn Protect", pSpawnProtectTime[client]);
    
    // Time has expired.
    if (pSpawnProtectTime[client] <= 0)
    {
        // Remove protect flag.
        pSpawnProtect[client] = false;
        
        // Tell client spawn protection is over.
        ZR_HudHint(client, "Spawn protection end");
        
        // Fix attributes.
        // TODO: Set class attributes.
        SetPlayerAlpha(client, 255);
        SetPlayerSpeed(client, 300.0);
        
        // Clear timer handle.
        tSpawnProtect[client] = INVALID_HANDLE;
        
        // Stop timer.
        return Plugin_Stop;
    }
    
    // Allow timer to continue repeating.
    return Plugin_Continue;
}