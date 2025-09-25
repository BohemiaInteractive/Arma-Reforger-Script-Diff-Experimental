class SCR_MapCampaignBuildingUI : SCR_MapUIBaseComponent
{
	protected SCR_MapCampaignUI m_MapCampaignUI

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();

		m_MapCampaignUI = SCR_MapCampaignUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapCampaignUI));
		if (!m_MapCampaignUI)
			return;

		m_MapCampaignUI.GetOnBasesInited().Insert(OnBasesInited);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBasesInited()
	{
		// register to base OnClick Event
		array<SCR_CampaignMapUIBase> bases = {};
		int count = m_MapCampaignUI.GetBases(bases);
		for (int i = 0; i < count; i++)
		{
			bases[i].m_OnClick.Insert(OnBaseClicked);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseClicked(SCR_CampaignMapUIBase baseUI)
	{
		array<SCR_CampaignBuildingProviderComponent> campaignBuildingProvides = {};
		int providerCount = baseUI.GetBase().GetBuildingProviders(campaignBuildingProvides);
		SCR_CampaignBuildingProviderComponent m_ProviderComponent;
		if (providerCount > 0)
		{
			for (int i = 0; i < providerCount; i++)
			{
				if (campaignBuildingProvides[i].UseAllAvailableProviders())
				{
					m_ProviderComponent = campaignBuildingProvides[i];
					break;
				}
			}
		}

		if (!m_ProviderComponent)
			return;

		IEntity playerEntity = SCR_PlayerController.GetLocalControlledEntity();

		// check player faction
		if (!m_ProviderComponent.IsEntityFactionSame(playerEntity, m_ProviderComponent.GetOwner()))
			return;

		int playerID = SCR_PlayerController.GetLocalPlayerId();
		m_ProviderComponent.RequestBuildingMode(playerID, true);
	}
}
