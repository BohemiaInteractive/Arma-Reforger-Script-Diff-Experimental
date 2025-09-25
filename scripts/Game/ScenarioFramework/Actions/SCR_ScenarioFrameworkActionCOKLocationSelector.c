[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionCOKLocationSelector : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "", desc: "Selection of Areas")]
	ref array<string> m_aSelectionOfAreas;

	[Attribute(defvalue: "Exfil_Area", desc: "Exfil Area Name")]
	string m_sExfilAreaName;

	[Attribute(defvalue: "10", desc: "Marker Search Distance", params: "0 inf 0.01")]
	float m_fMarkerSearchDistance;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_aSelectionOfAreas || m_aSelectionOfAreas.Count() < 2)
			return;

		const string startingLocation = m_aSelectionOfAreas.GetRandomElement();
		m_aSelectionOfAreas.RemoveItem(startingLocation);

		const string endingLocation = m_aSelectionOfAreas.GetRandomElement();
		IEntity endLocationEnt = GetGame().GetWorld().FindEntityByName(endingLocation);
		if (!endLocationEnt)
			return;

		SpawnObjects({startingLocation}, SCR_ScenarioFrameworkEActivationType.ON_INIT);
		IEntity startLocationEnt = GetGame().GetWorld().FindEntityByName(startingLocation);
		if (!startLocationEnt)
			return;
		
		GetGame().GetWorld().QueryEntitiesBySphere(startLocationEnt.GetOrigin(), m_fMarkerSearchDistance, HideMarker);
		GetGame().GetWorld().QueryEntitiesBySphere(endLocationEnt.GetOrigin(), m_fMarkerSearchDistance, MoveExfill);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HideMarker(notnull IEntity ent)
	{
		SCR_ScenarioFrameworkSlotMarker existingMarkerSlot = SCR_ScenarioFrameworkSlotMarker.Cast(ent.FindComponent(SCR_ScenarioFrameworkSlotMarker));
		if (!existingMarkerSlot)
			return true;

		existingMarkerSlot.SetIsTerminated(true);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool MoveExfill(notnull IEntity ent)
	{
		// Look for location marker at the exfil coords
		SCR_ScenarioFrameworkSlotMarker existingMarkerSlot = SCR_ScenarioFrameworkSlotMarker.Cast(ent.FindComponent(SCR_ScenarioFrameworkSlotMarker));
		if (!existingMarkerSlot)
			return true;

		SCR_MapMarkerBase locationMarker = existingMarkerSlot.GetMapMarker();
		if (!locationMarker)
			return true;

		SCR_MapMarkerBase exfilMarker;
		IEntity exfilArea = GetGame().GetWorld().FindEntityByName(m_sExfilAreaName);
		IEntity child = exfilArea.GetChildren();
		
		// Place exfil logic area at the area marker position
		exfilArea.SetOrigin(ent.GetOrigin());
		exfilArea.Update();
		
		while (child)
		{
			SCR_ScenarioFrameworkSlotMarker exfilSlotMarker = SCR_ScenarioFrameworkSlotMarker.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotMarker));
			if (exfilSlotMarker)
			{
				SpawnObjects({exfilSlotMarker.GetOwner().GetName()}, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
				const SCR_MapMarkerBase marker = exfilSlotMarker.GetMapMarker();
				if (marker)
				{
					exfilMarker = marker;
					break;
				}
			}

			child = child.GetSibling();
		}

		if (!exfilMarker)
			return true;

		// Copy random exfil location name onto exfil marker
		exfilMarker.SetCustomText(locationMarker.GetCustomText());

		// Terminate default area marker as we do not want its name to show next to exfil marker who also has it
		existingMarkerSlot.SetIsTerminated(true);
		return false;
	}
}
