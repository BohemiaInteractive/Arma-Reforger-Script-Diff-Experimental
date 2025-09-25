class SCR_PSNActivityComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_PSNActivityComponent : SCR_BaseGameModeComponent
{
	[Attribute(defvalue: "Default value", desc: "Activity ID")]
	protected string m_sActivityID;
	
	//------------------------------------------------------------------------------------------------
	override void OnGameModeStart()
	{
		//TODO: PS5 Activity start (ACTIVITY_ID)
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameModeEnd(SCR_GameModeEndData data)
	{
		//TODO: PS5 Activity complete (ACTIVITY_ID)
	}
}