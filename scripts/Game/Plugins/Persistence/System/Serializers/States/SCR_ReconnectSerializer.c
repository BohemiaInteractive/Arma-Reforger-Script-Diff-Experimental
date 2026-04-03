class SCR_PersistentReconnectData
{
	UUID m_sPlayerIdentity;
	float m_fRemainingValidity;
	UUID m_sReservedEntityId;

	//------------------------------------------------------------------------------------------------
	void ReadFrom(notnull PersistenceSystem persistence, notnull SCR_ReconnectData data)
	{
		if (data.m_ValidUntil)
			m_fRemainingValidity = data.m_ValidUntil.DiffSeconds(GetGame().GetWorld().GetTimestamp());

		m_sReservedEntityId = persistence.GetId(data.m_ReservedEntity);
	}

	//------------------------------------------------------------------------------------------------
	void ApplyTo(notnull PersistenceSystem persistence, notnull SCR_ReconnectData data)
	{
		if (m_fRemainingValidity > 0)
			data.m_ValidUntil = GetGame().GetWorld().GetTimestamp().PlusSeconds(m_fRemainingValidity);

		Tuple1<SCR_ReconnectData> context(data);
		PersistenceWhenAvailableTask task(OnEntityAvailable, context);
		persistence.WhenAvailable(m_sReservedEntityId, task);
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnEntityAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto entity = IEntity.Cast(instance);
		if (!entity)
			return;

		auto ctx = Tuple1<SCR_ReconnectData>.Cast(context);
		if (ctx.param1)
			ctx.param1.m_ReservedEntity = entity;
	}
}

class SCR_PlayerReconnectData : PersistentState
{
}

class SCR_PlayerReconnectDataSerializer : ScriptedStateSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_PlayerReconnectData;
	}

	//------------------------------------------------------------------------------------------------
	override ESerializeResult Serialize(notnull Managed instance, notnull SaveContext context)
	{
		const SCR_ReconnectComponent reconnect = SCR_ReconnectComponent.GetInstance();
		if (!reconnect)
			return ESerializeResult.DEFAULT;

		array<ref SCR_PersistentReconnectData> allData();
		foreach (UUID playerIdentity, SCR_ReconnectData data : reconnect.m_mReconnectData)
		{
			SCR_PersistentReconnectData persistentData();
			persistentData.m_sPlayerIdentity = playerIdentity;
			persistentData.ReadFrom(GetSystem(), data);
			allData.Insert(persistentData);
		}

		if (allData.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		const bool prev = context.EnableTypeDiscriminator(false);
		context.WriteValue("data", allData);
		context.EnableTypeDiscriminator(prev);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull LoadContext context)
	{
		const SCR_ReconnectComponent reconnect = SCR_ReconnectComponent.GetInstance();
		if (!reconnect)
			return true;

		int version;
		context.Read(version);

		array<ref SCR_PersistentReconnectData> data;
		const bool prev = context.EnableTypeDiscriminator(false);
		context.Read(data);
		context.EnableTypeDiscriminator(prev);
		if (data)
		{
			foreach (SCR_PersistentReconnectData persistentData : data)
			{
				SCR_ReconnectData entry();
				persistentData.ApplyTo(GetSystem(), entry);
				reconnect.m_mReconnectData.Set(persistentData.m_sPlayerIdentity, entry);
			}
		}

		reconnect.UpdateExpieryCheck();
		return true;
	}
}
