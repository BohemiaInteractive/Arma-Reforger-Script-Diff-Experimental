[BaseContainerProps(configRoot: true)]
class SCR_MainMenuConfiguration : Managed
{
	[Attribute("", UIWidgets.ResourceAssignArray, "MainMenu scenarios headers", "conf")]
	ref array<ResourceName> m_aMainMenuScenarios;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "MainMenu tutorial header", "conf")]
	ResourceName m_TutorialScenario;
}