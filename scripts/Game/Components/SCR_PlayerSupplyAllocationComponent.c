class SCR_PlayerSupplyAllocationComponentClass : ScriptComponentClass
{
}

class SCR_PlayerSupplyAllocationComponent : ScriptComponent
{
	[RplProp(onRplName: "MilitarySupplyAllocationReplicated", condition: RplCondition.OwnerOnly)]
	protected int m_iPlayerMilitarySupplyAllocation;

	[RplProp(onRplName: "AvailableAllocatedSuppliesReplicated", condition: RplCondition.OwnerOnly)]
	protected int m_iPlayerAvailableAllocatedSupplies;

	protected ref ScriptInvokerInt m_AvailableAllocatedSuppliesChanged;

	protected ref ScriptInvokerInt m_MilitarySupplyAllocationChanged;

	protected SCR_PlayerController m_PlayerController;

	protected ref SCR_MilitarySupplyAllocationConfig m_MilitarySupplyAllocationConfig;

	protected float m_fAvailableAllocatedSuppliesReplenishmentTimer = -1;

	protected bool m_bIsEnabled;

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		if (!SetUpConfigData())
			return;

		m_PlayerController = SCR_PlayerController.Cast(owner);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerConnected().Insert(OnPlayerConnected);
			gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
			gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned);
		}

		SCR_PlayerXPHandlerComponent playerXPHandlerComponent = SCR_PlayerXPHandlerComponent.Cast(owner.FindComponent(SCR_PlayerXPHandlerComponent));
		if (!playerXPHandlerComponent)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		SCR_ECharacterRank defaultPlayerRank = factionManager.GetRankByXP(playerXPHandlerComponent.GetPlayerXP());
		if (defaultPlayerRank)
			SetSupplyAllocationValuesByRank(defaultPlayerRank);
		
		SCR_ResourcePlayerControllerInventoryComponent.GetOnArsenalItemRequested().Insert(OnArsenalRequestItem);

		#ifdef ENABLE_DIAG
		ConnectToDiagSystem(owner);
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_MILITARY_SUPPLY_ALLOCATION, "Military Supply Allocation", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_MILITARY_SUPPLY_ALLOCATION_ENABLE, "", "Enable MSA", "Military Supply Allocation");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_MILITARY_SUPPLY_ALLOCATION_DEBUG, "", "Enable MSA debug", "Military Supply Allocation");
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_MILITARY_SUPPLY_ALLOCATION_ENABLE, m_bIsEnabled);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! \return Whether config is set up correctly
	protected bool SetUpConfigData()
	{
		SCR_ArsenalManagerComponent arsenalManagerComponent;
		SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManagerComponent);

		if (!arsenalManagerComponent)
			return false;

		m_bIsEnabled = arsenalManagerComponent.IsMilitarySupplyAllocationEnabled();
		arsenalManagerComponent.GetOnMilitarySupplyAllocationToggle().Insert(OnMilitarySupplyAllocationEnabledChanged);

		m_MilitarySupplyAllocationConfig = arsenalManagerComponent.GetMilitarySupplyApplicationConfigData();
		return m_MilitarySupplyAllocationConfig != null;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnAvailableAllocatedSuppliesChanged()
	{
		if (!m_AvailableAllocatedSuppliesChanged)
			m_AvailableAllocatedSuppliesChanged = new ScriptInvokerInt();

		return m_AvailableAllocatedSuppliesChanged;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnMilitarySupplyAllocationChanged()
	{
		if (!m_MilitarySupplyAllocationChanged)
			m_MilitarySupplyAllocationChanged = new ScriptInvokerInt();

		return m_MilitarySupplyAllocationChanged;
	}

	//------------------------------------------------------------------------------------------------
	protected void AvailableAllocatedSuppliesReplicated()
	{
		if (m_AvailableAllocatedSuppliesChanged)
			m_AvailableAllocatedSuppliesChanged.Invoke(m_iPlayerAvailableAllocatedSupplies);
	}

	//------------------------------------------------------------------------------------------------
	protected void MilitarySupplyAllocationReplicated()
	{
		if (m_MilitarySupplyAllocationChanged)
			m_MilitarySupplyAllocationChanged.Invoke(m_iPlayerMilitarySupplyAllocation);
	}

	//------------------------------------------------------------------------------------------------
	int GetPlayerMilitarySupplyAllocation()
	{
		return m_iPlayerMilitarySupplyAllocation;
	}

	//------------------------------------------------------------------------------------------------
	int GetPlayerAvailableAllocatedSupplies()
	{
		return m_iPlayerAvailableAllocatedSupplies;
	}

	//------------------------------------------------------------------------------------------------
	float GetAvailableAllocatedSuppliesReplenishmentTimer()
	{
		return m_fAvailableAllocatedSuppliesReplenishmentTimer;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds param[in] amount to player Available Allocated Supplies
	//! New Available Allocated Supplies value is clamped so it does not go below 0 ar above Military Supply Allocation value
	//! \param[in] amount
	void AddPlayerAvailableAllocatedSupplies(int amount)
	{
		if (IsProxy())
			return;

		if (amount == 0)
			return;

		int newAvailableAllocatedSupplies = m_iPlayerAvailableAllocatedSupplies + amount;

		m_iPlayerAvailableAllocatedSupplies = Math.ClampInt(newAvailableAllocatedSupplies, 0, m_iPlayerMilitarySupplyAllocation);

		if (m_AvailableAllocatedSuppliesChanged)
			m_AvailableAllocatedSuppliesChanged.Invoke(m_iPlayerAvailableAllocatedSupplies);

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets Military Supply Allocation to new value
	//! Adds the difference between old and new Military Supply Allocation value to the current Available Allocated Supplies value
	//! \param[in] amount
	void SetPlayerMilitarySupplyAllocation(int amount)
	{
		if (IsProxy())
			return;

		if (amount == m_iPlayerMilitarySupplyAllocation)
			return;

		int availableAllocatedSuppliesToAdd = amount - m_iPlayerMilitarySupplyAllocation;

		m_iPlayerMilitarySupplyAllocation = amount;

		if (m_MilitarySupplyAllocationChanged)
			m_MilitarySupplyAllocationChanged.Invoke(m_iPlayerMilitarySupplyAllocation);

		AddPlayerAvailableAllocatedSupplies(availableAllocatedSuppliesToAdd);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] amount
	//! \return true if player has at least the same amount of Available Allocated Supplies as param[in] amount
	bool HasPlayerEnoughAvailableAllocatedSupplies(int amount)
	{
		return m_iPlayerAvailableAllocatedSupplies - amount >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! If player's Available Allocated Supplies are below the threshold value it gets replenished to threshold value
	protected void ReplenishAvailableAllocatedSupplies()
	{
		SCR_PlayerXPHandlerComponent playerXPHandlerComponent = SCR_PlayerXPHandlerComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerXPHandlerComponent));
		if (!playerXPHandlerComponent)
			return;

		int thresholdValue = m_MilitarySupplyAllocationConfig.GetAvailableAllocatedSuppliesReplenishmentThresholdValueAtRank(playerXPHandlerComponent.GetPlayerRankByXP());

		if (m_iPlayerAvailableAllocatedSupplies < thresholdValue)
			AddPlayerAvailableAllocatedSupplies(thresholdValue - m_iPlayerAvailableAllocatedSupplies);

		m_fAvailableAllocatedSuppliesReplenishmentTimer = m_MilitarySupplyAllocationConfig.GetAvailableAllocatedSuppliesReplenishmentTimer();
	}

	//------------------------------------------------------------------------------------------------
	//! Start replenishment timer and listen to player rank changes
	//! \param[in] playerId
	//! \param[in] controlledEntity
	void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		if (m_PlayerController.GetMainEntity() != controlledEntity)
			return;

		SCR_PlayerXPHandlerComponent playerXPHandler = SCR_PlayerXPHandlerComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerXPHandlerComponent));
		if (playerXPHandler)
		{
			playerXPHandler.GetOnPlayerXPChanged().Remove(OnUnspawnedPlayerXPChanged);
		}

		SCR_CharacterRankComponent rankComp = SCR_CharacterRankComponent.Cast(controlledEntity.FindComponent(SCR_CharacterRankComponent));
		if (!rankComp)
			return;

		rankComp.s_OnRankChanged.Insert(OnRankChanged);

		SetEventMask(GetOwner(), EntityEvent.FRAME);
		m_fAvailableAllocatedSuppliesReplenishmentTimer = m_MilitarySupplyAllocationConfig.GetAvailableAllocatedSuppliesReplenishmentTimer();
	}

	//------------------------------------------------------------------------------------------------
	//! Subscribes to player XP changed invoker
	//! \param[in] playerId
	protected void OnPlayerConnected(int playerId)
	{
		if (m_PlayerController.GetPlayerId() != playerId)
			return;

		SCR_PlayerXPHandlerComponent playerXPHandler = SCR_PlayerXPHandlerComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerXPHandlerComponent));
		if (!playerXPHandler)
			return;

		playerXPHandler.GetOnPlayerXPChanged().Insert(OnUnspawnedPlayerXPChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! Updates Military Supply Allocation values based on player rank in an event of a unspawned player getting XP
	//! \param[in] playerId
	//! \param[in] currentXP
	//! \param[in] XPToAdd
	//! \param[in] rewardId
	protected void OnUnspawnedPlayerXPChanged(int playerId, int currentXP, int XPToAdd, SCR_EXPRewards rewardId)
	{
		if (m_PlayerController.GetPlayerId() != playerId)
			return;

		if (m_PlayerController.GetMainEntity())
			return;

		SCR_PlayerXPHandlerComponent playerXPHandler = SCR_PlayerXPHandlerComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerXPHandlerComponent));
		if (!playerXPHandler)
			return;

		SCR_ECharacterRank rank = playerXPHandler.GetPlayerRankByXP();
		SetSupplyAllocationValuesByRank(rank);
	}

	//------------------------------------------------------------------------------------------------
	//! Reset Available Allocated Supplies value to value of Military Supply Allocation
	//! Stops Available Allocated Supplies replenishment timer
	//! \param[in] investigatorContextData
	void OnPlayerKilled(SCR_InstigatorContextData investigatorContextData)
	{
		if (m_PlayerController.GetMainEntity() != investigatorContextData.GetVictimEntity())
			return;

		ClearEventMask(GetOwner(), EntityEvent.FRAME);
		m_fAvailableAllocatedSuppliesReplenishmentTimer = -1;
		AddPlayerAvailableAllocatedSupplies(m_iPlayerMilitarySupplyAllocation);
	}

	//------------------------------------------------------------------------------------------------
	//! Player rank changed - set the Supply Allocation values according to new rank
	//! \param[in] prevRank
	//! \param[in] newRank
	//! \param[in] owner
	//! \param[in] silent
	void OnRankChanged(SCR_ECharacterRank prevRank, SCR_ECharacterRank newRank, IEntity owner, bool silent)
	{
		if (m_PlayerController.GetMainEntity() != owner || prevRank == newRank)
			return;

		SetSupplyAllocationValuesByRank(newRank);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets new Military Supply Allocation value corresponding to new rank
	//! \param[in] rank
	protected void SetSupplyAllocationValuesByRank(SCR_ECharacterRank rank)
	{
		SetPlayerMilitarySupplyAllocation(m_MilitarySupplyAllocationConfig.GetMilitarySupplyAllocationValueAtRank(rank));
	}

	//------------------------------------------------------------------------------------------------
	//! Adjust player Available Allocated Supplies value after requesting an item from arsenal
	//! \param[in] resourceComponent
	//! \param[in] resourceName
	//! \param[in] inventoryStorageComponent
	//! \param[in] resourceType
	//! \param[in] itemCost
	void OnArsenalRequestItem(notnull SCR_ResourceComponent resourceComponent, ResourceName resourceName, IEntity requesterEntity, notnull BaseInventoryStorageComponent inventoryStorageComponent, EResourceType resourceType, int itemCost)
	{
		if (itemCost == 0 || resourceType != EResourceType.SUPPLIES)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(requesterEntity);
		if (!playerController || playerController != m_PlayerController)
			return;

		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return;

		IEntity resourcesOwner = resourceComponent.GetOwner();
		if (!resourcesOwner)
			return;

		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(resourcesOwner);
		if (!arsenalComponent || !arsenalComponent.IsArsenalUsingSupplies())
			return;

		SCR_Faction faction = arsenalComponent.GetAssignedFaction();
		SCR_Faction playerFaction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerController.GetPlayerId()));
		if (!faction || !playerFaction || faction != playerFaction)
			return;

		SCR_EntityCatalogEntry entry = entityCatalogManager.GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType.ITEM, resourceName, faction);
		if (!entry)
			return;

		SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
		if (!data || !data.GetUseMilitarySupplyAllocation())
			return;

		SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		if (!consumer)
			return;

		AddPlayerAvailableAllocatedSupplies(-1 * itemCost * consumer.GetBuyMultiplier());
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	protected bool IsProxy()
	{
		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));

		return rpl && rpl.IsProxy();
	}

	//------------------------------------------------------------------------------------------------
	//! Enables or disables Military Supply Allocation by Rank
	//! \param[in] enable
	protected void SetEnabled(bool enable)
	{
		if (m_bIsEnabled == enable)
			return;

		m_bIsEnabled = enable;
		Rpc(RpcAsk_SetEnabled, enable);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetEnabled(bool enable)
	{
		SCR_ArsenalManagerComponent arsenalManagerComponent;
		SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManagerComponent);

		if (!arsenalManagerComponent)
			return;

		arsenalManagerComponent.SetMilitarySupplyAllocationEnabled(enable);
		m_bIsEnabled = enable;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMilitarySupplyAllocationEnabledChanged(bool enable)
	{
		if (m_bIsEnabled == enable)
			return;

		m_bIsEnabled = enable;
		#ifdef ENABLE_DIAG
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_MILITARY_SUPPLY_ALLOCATION_ENABLE, enable);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_fAvailableAllocatedSuppliesReplenishmentTimer -= timeSlice;
		if (m_fAvailableAllocatedSuppliesReplenishmentTimer < 0)
			ReplenishAvailableAllocatedSupplies();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnDelete(IEntity owner)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerConnected().Remove(OnPlayerConnected);
			gameMode.GetOnPlayerKilled().Remove(OnPlayerKilled);
			gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawned);
		}

		if (m_PlayerController)
		{
			SCR_PlayerXPHandlerComponent playerXPHandler = SCR_PlayerXPHandlerComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerXPHandlerComponent));
			if (playerXPHandler)
			{
				playerXPHandler.GetOnPlayerXPChanged().Remove(OnUnspawnedPlayerXPChanged);
			}
		}

		SCR_ResourcePlayerControllerInventoryComponent.GetOnArsenalItemRequested().Remove(OnArsenalRequestItem);

		SCR_ArsenalManagerComponent arsenalManagerComponent;
		SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManagerComponent);

		if (arsenalManagerComponent)
			arsenalManagerComponent.GetOnMilitarySupplyAllocationToggle().Remove(OnMilitarySupplyAllocationEnabledChanged);

		#ifdef ENABLE_DIAG
		DisconnectFromDiagSystem(owner);
		#endif
	}

	#ifdef ENABLE_DIAG	
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_MILITARY_SUPPLY_ALLOCATION_ENABLE) && !m_bIsEnabled)
			SetEnabled(true);

		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_MILITARY_SUPPLY_ALLOCATION_ENABLE) && m_bIsEnabled)
			SetEnabled(false);

		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_MILITARY_SUPPLY_ALLOCATION_DEBUG))
			return;

		if (!m_bIsEnabled)
		{
			DbgUI.Text("Military Supply Allocation is disabled.");
			return;
		}

		DbgUI.Begin("SCR_PlayerSupplyAllocationComponent");
		DbgUI.Text(string.Format("Player ID: %1", m_PlayerController.GetPlayerId()));
		DbgUI.Text(string.Format("Available allocated supplies: %1 / %2", GetPlayerAvailableAllocatedSupplies(), GetPlayerMilitarySupplyAllocation()));

		DbgUI.Text(string.Format("Replenishment timer: %1s",  Math.Round(GetAvailableAllocatedSuppliesReplenishmentTimer())));
		DbgUI.Spacer(8);
		DbgUI.End();
	}
	#endif
}
