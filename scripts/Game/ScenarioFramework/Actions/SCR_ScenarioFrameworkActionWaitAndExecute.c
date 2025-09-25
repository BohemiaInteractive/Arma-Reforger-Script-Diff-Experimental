[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionWaitAndExecute : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "How long to wait before executing action")]
	int m_iDelayInSeconds;

	[Attribute(desc: "If this is set to a number larger than  Delay In Seconds, it will randomize resulted delay between these two values")]
	int m_iDelayInSecondsMax;

	[Attribute(UIWidgets.CheckBox, desc: "If true, it will activate actions in looped manner using Delay settings as the frequency. If randomized, it will randomize the time each time it loops.")]
	bool m_bLooped;

	[Attribute(defvalue: "1", desc: "Which actions will be executed once set time passes", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	protected int m_iDelay;
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] object
	void ExecuteActions(IEntity object)
	{
		if (!CanActivate())
			return;
		
		if (m_bLooped)
		{
			m_iDelay = m_iDelayInSeconds;
			if (m_iDelayInSecondsMax > m_iDelayInSeconds)
				m_iDelay = Math.RandomIntInclusive(m_iDelayInSeconds, m_iDelayInSecondsMax);
		}

		int activatableActions;
		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			if (actions.m_iMaxNumberOfActivations == -1 || actions.m_iNumberOfActivations < actions.m_iMaxNumberOfActivations)
				activatableActions++;
			
			actions.OnActivate(object);
		}
		
		if (activatableActions == 0)
			SCR_ScenarioFrameworkSystem.GetCallQueuePausable().Remove(ExecuteActions);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		m_iDelay = m_iDelayInSeconds;
		if (m_iDelayInSecondsMax > m_iDelayInSeconds)
			m_iDelay = Math.RandomIntInclusive(m_iDelayInSeconds, m_iDelayInSecondsMax);

		if (m_aActions.IsEmpty())
			return;
		
		//Used to delay the call as it is the feature of this action
		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(ExecuteActions, m_iDelay * 1000, m_bLooped, object);
	}

	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
	
	//------------------------------------------------------------------------------------------------
	override void RestoreToDefault()
	{
		super.RestoreToDefault();
		
		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().Remove(ExecuteActions);
	}
}