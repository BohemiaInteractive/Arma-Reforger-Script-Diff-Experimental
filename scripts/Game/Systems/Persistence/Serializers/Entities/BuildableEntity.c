class BuildableEntitySerializer : GenericEntitySerializer
{
	protected bool m_bIgnoreSpawning;
	
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return GenericEntity;
	}

	//------------------------------------------------------------------------------------------------
	override bool DeserializeSpawnData(out ResourceName prefab, out EntitySpawnParams params, notnull BaseSerializationLoadContext context)
	{
		// Avoid buildable compositions loading as finished
		m_bIgnoreSpawning = SCR_EditorLinkComponent.IsSpawningIgnored();
		SCR_EditorLinkComponent.IgnoreSpawning(true);
		return super.DeserializeSpawnData(prefab, params,  context);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull BaseSerializationLoadContext context)
	{
		// Allow spawning again
		SCR_EditorLinkComponent.IgnoreSpawning(m_bIgnoreSpawning);
		return super.Deserialize(entity, context);
	}
}
