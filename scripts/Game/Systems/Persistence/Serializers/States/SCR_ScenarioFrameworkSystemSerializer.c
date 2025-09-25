//! Dummy state proxy to access scenario framework system instance
class SCR_ScenarioFrameworkSystemData : PersistentState
{
}

class SCR_ScenarioFrameworkSystemSerializer : ScriptedStateSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_ScenarioFrameworkSystemData;
	}

	//------------------------------------------------------------------------------------------------
	override ESerializeResult Serialize(notnull Managed instance, notnull BaseSerializationSaveContext context)
	{
		const SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return ESerializeResult.DEFAULT;

		if (scenarioFrameworkSystem.GetIsMatchOver())
		{
			context.WriteValue("version", 1);
			context.WriteValue("matchOver", true);
		}

		const EGameOverTypes gameOverType = scenarioFrameworkSystem.m_eGameOverType;

		array<ref SCR_ScenarioFrameworkAreaSave> areas();
		StoreAreaStates(GetSystem(), scenarioFrameworkSystem, areas);

		if ((gameOverType == EGameOverTypes.COMBATPATROL_DRAW) && areas.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteValueDefault("matchOver", false, false);
		context.Write(gameOverType);
		context.Write(areas);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull BaseSerializationLoadContext context)
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return true; // SF system no longer present, so we can ignore it.

		int version;
		context.Read(version);

		bool matchOver;
		context.ReadDefault(matchOver, false);

		scenarioFrameworkSystem.m_bIsSaveGameLoad = true;

		if (!matchOver)
		{
			EGameOverTypes gameOverType;
			context.Read(gameOverType);
			scenarioFrameworkSystem.m_eGameOverType = gameOverType;

			array<ref SCR_ScenarioFrameworkAreaSave> areas();
			context.Read(areas);
			LoadAreaStates(GetSystem(), scenarioFrameworkSystem, areas);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void StoreAreaStates(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, out array<ref SCR_ScenarioFrameworkAreaSave> saves)
	{
		for (int i = scenarioFrameworkSystem.m_aAreas.Count() - 1; i >= 0; i--)
		{
			SCR_ScenarioFrameworkAreaSave save();
			save.Save(persistence, scenarioFrameworkSystem.m_aAreas[i]);
			if (!save.IsDefault())
				saves.Insert(save);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadAreaStates(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, array<ref SCR_ScenarioFrameworkAreaSave> saves)
	{
		foreach (SCR_ScenarioFrameworkAreaSave areaSave : saves)
		{
			auto entity = GetGame().GetWorld().FindEntityByName(areaSave.m_sName);
			if (!entity)
				continue;

			auto area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (!area)
				continue;

			areaSave.Load(persistence, scenarioFrameworkSystem, area);
		}
	}
}

class SCR_ScenarioFrameworkLayerSave
{
	string											m_sName;
	bool 											m_bIsTerminated;
	int												m_iActivationType;
	int 											m_iRepeatedSpawnNumber;
	ref array<string>								m_aRandomlySpawnedChildren = {};
	ref array<ref SCR_ScenarioFrameworkLogicSave>	m_aLogics = {};
	ref array<ref SCR_ScenarioFrameworkLayerSave>	m_aChildLayers = {};

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkLayerSave GetLayerTypeSave(const SCR_ScenarioFrameworkLayerBase layer)
	{
		if (layer.IsInherited(SCR_ScenarioFrameworkLayerTaskDefend))
			return new SCR_ScenarioFrameworkLayerTaskDefendSave();
		
		if (layer.IsInherited(SCR_ScenarioFrameworkLayerTask))
			return new SCR_ScenarioFrameworkLayerTaskSave();

		if (layer.IsInherited(SCR_ScenarioFrameworkSlotBase))
			return new SCR_ScenarioFrameworkSlotSave();

		return new SCR_ScenarioFrameworkLayerSave();
	}

	//------------------------------------------------------------------------------------------------
	void Save(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		m_sName = layer.GetName();
		m_iActivationType = layer.GetActivationType();
		if (m_iActivationType == layer.m_eActivationTypeDefault)
			m_iActivationType = -1; // Default

		m_bIsTerminated = layer.GetIsTerminated();

		m_iRepeatedSpawnNumber = layer.GetRepeatedSpawnNumber();
		if (m_iRepeatedSpawnNumber == layer.m_iRepeatedSpawnNumberDefault)
			m_iRepeatedSpawnNumber = 0;

		// Logics
		array<SCR_ScenarioFrameworkLogic> logics();
		layer.GetLogics(logics);
		foreach (SCR_ScenarioFrameworkLogic logic : logics)
		{
			auto logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(logic);
			if (!logicCounter)
				continue;

			SCR_ScenarioFrameworkLogicCounterSave logicSave();
			logicSave.Read(logicCounter);
			if (!logicSave.IsDefault())
				m_aLogics.Insert(logicSave);
		}

		// Randomization
		if (layer.GetSpawnChildrenType() != SCR_EScenarioFrameworkSpawnChildrenType.ALL)
		{
			foreach (auto randomlayer : layer.GetRandomlySpawnedChildren())
			{
				m_aRandomlySpawnedChildren.Insert(randomlayer.GetName());
			}
		}

		// Children
		array<SCR_ScenarioFrameworkLayerBase> children();
		layer.GetChildren(children);
		foreach (auto childLayer : children)
		{
			auto childSave = GetLayerTypeSave(childLayer);
			childSave.Save(persistence, childLayer);
			if (!childSave.IsDefault())
				m_aChildLayers.Insert(childSave);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsDefault()
	{
		return
			m_aChildLayers.IsEmpty() &&
			m_aLogics.IsEmpty() &&
			m_aRandomlySpawnedChildren.IsEmpty() &&
			m_iActivationType == -1 &&
			m_bIsTerminated == false &&
			m_iRepeatedSpawnNumber == 0;
	}

	//------------------------------------------------------------------------------------------------
	void Load(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		if (m_iRepeatedSpawnNumber != 0)
		{
			layer.SetEnableRepeatedSpawn(true);
			layer.SetRepeatedSpawnNumber(m_iRepeatedSpawnNumber);
		}

		if (m_iActivationType != -1)
			layer.SetActivationType(m_iActivationType);

		layer.SetIsTerminated(m_bIsTerminated);

		if (!m_aRandomlySpawnedChildren.IsEmpty())
			layer.SetRandomlySpawnedChildren(m_aRandomlySpawnedChildren);

		foreach (auto logic : m_aLogics)
		{
			logic.Load();
		}

		BaseWorld world = GetGame().GetWorld();
		foreach (auto saveData : m_aChildLayers)
		{
			IEntity entity = world.FindEntityByName(saveData.m_sName);
			if (!entity)
				continue;

			auto childlayer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (childlayer)
				saveData.Load(persistence, scenarioFrameworkSystem, childlayer);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationSave(BaseSerializationSaveContext context)
	{
		context.Write(m_sName);

		if (!context.CanSeekMembers() || !m_aChildLayers.IsEmpty())
			context.Write(m_aChildLayers);

		if (!context.CanSeekMembers() || !m_aLogics.IsEmpty())
			context.Write(m_aLogics);

		if (!context.CanSeekMembers() || !m_aRandomlySpawnedChildren.IsEmpty())
			context.Write(m_aRandomlySpawnedChildren);

		context.WriteDefault(m_iActivationType, -1);
		context.WriteDefault(m_iRepeatedSpawnNumber, 0);
		context.WriteDefault(m_bIsTerminated, false);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationLoad(BaseSerializationLoadContext context)
	{
		context.Read(m_sName);

		if (!context.Read(m_aChildLayers) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aLogics) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aRandomlySpawnedChildren) && !context.CanSeekMembers())
			return false;

		context.ReadDefault(m_iActivationType, -1);
		context.ReadDefault(m_iRepeatedSpawnNumber, 0);
		context.ReadDefault(m_bIsTerminated, false);
		return true;
	}
}

class SCR_ScenarioFrameworkAreaSave : SCR_ScenarioFrameworkLayerSave
{
	bool m_bAreaSelected;
	string m_sItemDeliveryPointName;
	string m_sLayerTaskName;

	//------------------------------------------------------------------------------------------------
	override void Save(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Save(persistence, layer);

		auto area = SCR_ScenarioFrameworkArea.Cast(layer);
		m_bAreaSelected = area.GetIsAreaSelected();
		m_sItemDeliveryPointName = area.GetDeliveryPointName();
		if (area.GetLayerTask())
			m_sLayerTaskName = area.GetLayerTaskName();
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return
			super.IsDefault() &&
			!m_bAreaSelected &&
			m_sItemDeliveryPointName.IsEmpty() &&
			m_sLayerTaskName.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override void Load(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Load(persistence, scenarioFrameworkSystem, layer);

		auto area = SCR_ScenarioFrameworkArea.Cast(layer);

		if (m_bAreaSelected)
		{
			scenarioFrameworkSystem.m_aAreasTasksToSpawn.Insert(m_sName);
			scenarioFrameworkSystem.m_aLayersTaskToSpawn.Insert(m_sLayerTaskName);
		}

		if (m_sItemDeliveryPointName)
			area.StoreDeliveryPoint(m_sItemDeliveryPointName);
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_bAreaSelected, false);
		context.WriteDefault(m_sItemDeliveryPointName, string.Empty);
		context.WriteDefault(m_sLayerTaskName, string.Empty);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_bAreaSelected, false);
		context.ReadDefault(m_sItemDeliveryPointName, string.Empty);
		context.ReadDefault(m_sLayerTaskName, string.Empty);
		return true;
	}
}

class SCR_ScenarioFrameworkLayerTaskSave : SCR_ScenarioFrameworkLayerSave
{
	bool m_bTaskActive;

	//------------------------------------------------------------------------------------------------
	override void Save(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Save(persistence, layer);

		auto layerTask = SCR_ScenarioFrameworkLayerTask.Cast(layer);
		auto task = layerTask.GetSlotTask();
		m_bTaskActive = task != null;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && !m_bTaskActive;
	}

	//------------------------------------------------------------------------------------------------
	override void Load(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Load(persistence, scenarioFrameworkSystem, layer);

		auto taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		auto task = SCR_ScenarioFrameworkTask.Cast(taskSystem.GetTaskFromTaskID(layer.GetName()));
		if (!task)
			return;

		auto layerTask = SCR_ScenarioFrameworkLayerTask.Cast(layer);
		layerTask.m_Task = task;
		task.SetLayerTask(layerTask);
		task.SetSlotTask(layerTask.GetSlotTask());
	}
}

class SCR_ScenarioFrameworkLayerTaskDefendSave : SCR_ScenarioFrameworkLayerTaskSave
{
	float m_fSecondsRemaining = -1.0;

	//------------------------------------------------------------------------------------------------
	override void Save(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Save(persistence, layer);

		auto layerTaskDefend = SCR_ScenarioFrameworkLayerTaskDefend.Cast(layer);
		m_fSecondsRemaining = layerTaskDefend.GetSecondsRemaining();
		const float defendTime = layerTaskDefend.GetDefendTime();
		if (float.AlmostEqual(m_fSecondsRemaining, defendTime))
			m_fSecondsRemaining = -1.0; // Default tiime
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && float.AlmostEqual(m_fSecondsRemaining, -1.0);
	}

	//------------------------------------------------------------------------------------------------
	override void Load(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Load(persistence, scenarioFrameworkSystem, layer);

		auto layerTaskDefend = SCR_ScenarioFrameworkLayerTaskDefend.Cast(layer);
		layerTaskDefend.SetSecondsRemaining(m_fSecondsRemaining);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_fSecondsRemaining, -1.0);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_fSecondsRemaining, -1.0);
		return true;
	}
}

class SCR_ScenarioFrameworkSlotSave : SCR_ScenarioFrameworkLayerSave
{
	ResourceName m_sRandomlySpawnedObject;
	UUID m_sSlotEntity;
	ref array<UUID> m_aMiscEntities = {};

	//------------------------------------------------------------------------------------------------
	override void Save(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Save(persistence, layer);

		auto slot = SCR_ScenarioFrameworkSlotBase.Cast(layer);
		m_sRandomlySpawnedObject = slot.GetRandomlySpawnedObject();
		auto spawnedEntity = slot.GetSpawnedEntity();
		m_sSlotEntity = persistence.GetId(spawnedEntity);
		foreach (auto entity : slot.m_aSpawnedEntities)
		{
			if (entity == spawnedEntity)
				continue;

			const UUID id = persistence.GetId(entity);
			if (!id.IsNull())
				m_aMiscEntities.Insert(id);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return
			super.IsDefault() &&
			m_sRandomlySpawnedObject.IsEmpty() &&
			m_sSlotEntity.IsNull() &&
			m_aMiscEntities.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override void Load(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Load(persistence, scenarioFrameworkSystem, layer);

		auto slot = SCR_ScenarioFrameworkSlotBase.Cast(layer);

		if (m_sRandomlySpawnedObject)
			slot.SetRandomlySpawnedObject(m_sRandomlySpawnedObject);

		if (!m_sSlotEntity.IsNull())
		{
			Tuple2<SCR_ScenarioFrameworkSlotBase, bool> entityContext(slot, true);
			PersistenceWhenAvailableTask slotEntityTask(entityContext, OnEntityAvailable);
			persistence.WhenAvailable(m_sSlotEntity, slotEntityTask);
		}

		foreach (auto miscEntity : m_aMiscEntities)
		{
			Tuple2<SCR_ScenarioFrameworkSlotBase, bool> entityContext(slot, false);
			PersistenceWhenAvailableTask slotEntityTask(entityContext, OnEntityAvailable);
			persistence.WhenAvailable(miscEntity, slotEntityTask);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnEntityAvailable(Managed context, Managed instance, PersistenceDeferredDeserializeTask task, bool expired)
	{
		auto entity = IEntity.Cast(instance);
		if (!entity)
			return;

		auto entityContext = Tuple2<SCR_ScenarioFrameworkSlotBase, bool>.Cast(context);
		if (!entityContext.param1)
			return;

		if (entityContext.param2)
		{
			entityContext.param1.SetEntity(entity);

			auto slotWP = SCR_ScenarioFrameworkSlotWaypoint.Cast(entityContext.param1);
			if (slotWP && slotWP.m_Waypoint)
			{
				slotWP.m_Waypoint.m_SlotWaypoint = slotWP;
				slotWP.m_Waypoint.SetupWaypoint(entity);
			}
		}
		else
		{
			entityContext.param1.m_aSpawnedEntities.Insert(entity);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_sRandomlySpawnedObject, ResourceName.Empty);
		context.WriteDefault(m_sSlotEntity, UUID.NULL_UUID);

		if (!context.CanSeekMembers() || !m_aMiscEntities.IsEmpty())
			context.Write(m_aMiscEntities);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_sRandomlySpawnedObject, ResourceName.Empty);
		context.ReadDefault(m_sSlotEntity, UUID.NULL_UUID);

		if (!context.Read(m_aMiscEntities) && !context.CanSeekMembers())
			return false;

		return true;
	}
}

class SCR_ScenarioFrameworkLogicSave
{
	string m_sName;
	bool m_bIsTerminated;

	//------------------------------------------------------------------------------------------------
	void Read(SCR_ScenarioFrameworkLogic logic)
	{
		m_sName = logic.GetName();
		m_bIsTerminated = logic.GetIsTerminated();
	}

	//------------------------------------------------------------------------------------------------
	bool IsDefault()
	{
		return !m_bIsTerminated;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkLogic Load()
	{
		if (m_sName.IsEmpty())
			return null;

		auto logic = SCR_ScenarioFrameworkLogic.Cast(GetGame().GetWorld().FindEntityByName(m_sName));
		if (logic && m_bIsTerminated)
			logic.SetIsTerminated(true);

		return logic;
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationSave(BaseSerializationSaveContext context)
	{
		context.Write(m_sName);
		context.WriteDefault(m_bIsTerminated, false);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationLoad(BaseSerializationLoadContext context)
	{
		context.Read(m_sName);
		context.ReadDefault(m_bIsTerminated, false);
		return true;
	}
}

class SCR_ScenarioFrameworkLogicCounterSave : SCR_ScenarioFrameworkLogicSave
{
	int m_iCounterValue;

	//------------------------------------------------------------------------------------------------
	override void Read(SCR_ScenarioFrameworkLogic logic)
	{
		m_iCounterValue = SCR_ScenarioFrameworkLogicCounter.Cast(logic).GetCounterValue();
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && m_iCounterValue == 0;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkLogic Load()
	{
		auto logic = super.Load();
		auto counter = SCR_ScenarioFrameworkLogicCounter.Cast(logic);
		if (counter)
			counter.SetCounterValue(m_iCounterValue);

		return logic;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_iCounterValue, 0);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_iCounterValue, 0);
		return true;
	}
}
