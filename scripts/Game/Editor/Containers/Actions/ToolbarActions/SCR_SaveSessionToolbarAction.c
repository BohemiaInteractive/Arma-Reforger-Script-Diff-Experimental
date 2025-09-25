[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SaveSessionToolbarAction : SCR_EditorToolbarAction
{
	[Attribute(desc: "When enabled, the operation will always bring up a save dialog.")]
	protected bool m_bSaveAs;

	//------------------------------------------------------------------------------------------------
	override bool IsServer()
	{
		//--- The action opens local UI
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		//--- Disallow in MP
		if (!Replication.IsServer())
			return false;

		//--- Disallow if mission is not configured for saving or currently suppressed
		return GetGame().GetSaveGameManager().IsSavingAllowed();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}

	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		if (m_bSaveAs)
		{
			new SCR_CreateNewSaveDialog();
			return;
		}

		GetGame().GetSaveGameManager().RequestSavePoint(ESaveGameType.MANUAL);
	}
}
