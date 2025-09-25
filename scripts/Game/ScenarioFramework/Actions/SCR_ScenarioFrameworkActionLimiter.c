[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionLimiter : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "0", desc: "How much time before assigned actions can be activated again.", params: "0 inf")]
	float m_fActivationLimitDelay;
	
	[Attribute(desc: "Actions with limited activation")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;
	
	WorldTimestamp m_LastActivationTimestamp;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		WorldTimestamp currentTimestamp = GetGame().GetWorld().GetTimestamp();
		
		if (m_LastActivationTimestamp && currentTimestamp.Less(m_LastActivationTimestamp.PlusSeconds(m_fActivationLimitDelay)))
			return;
		
		m_LastActivationTimestamp = currentTimestamp;
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActions)
		{
			action.OnActivate(object);
		}
	}
}