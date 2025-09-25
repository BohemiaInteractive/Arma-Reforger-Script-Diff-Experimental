//------------------------------------------------------------------------------------------------
class SCR_DamageAreaComponentClass: DamageAreaComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_DamageAreaComponent : DamageAreaComponent
{
	protected ref array<ref DamageArea> m_damageAreas = new array<ref DamageArea>;
	
	override protected void OnPostInit(IEntity owner)
	{
		if(!GetGame().InPlayMode())
			return;
		
		GetDamageAreas(m_damageAreas);
		
		foreach(DamageArea area : m_damageAreas)
		{
			RegisterDamageArea(area);
		}
	}
	
	override protected void OnDelete(IEntity owner)
	{
		if(!GetGame().InPlayMode())
			return;
		
		foreach(DamageArea area : m_damageAreas)
		{
			RegisterDamageArea(area);
		}
	}
}