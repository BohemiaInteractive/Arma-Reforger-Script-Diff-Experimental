[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_RewindToolbarAction: SCR_EditorToolbarAction
{
	[Attribute(desc: "When enabled, the action will delete the rewind point instead of loading it.")]
	protected bool m_bIsDelete;
	
	protected SCR_BaseToolbarEditorUIComponent m_Toolbar;
	
	//---------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		SCR_RewindComponent rewindManager = SCR_RewindComponent.GetInstance();
		return rewindManager && rewindManager.HasRewindPoint();
	}

	//---------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		if (m_bIsDelete)
		{
			SCR_RewindComponent rewindManager = SCR_RewindComponent.GetInstance();
			rewindManager.DeleteRewindPoint();
			
			SCR_PauseGameTimeEditorComponent pauseManager = SCR_PauseGameTimeEditorComponent.Cast(SCR_PauseGameTimeEditorComponent.GetInstance(SCR_PauseGameTimeEditorComponent));
			if (pauseManager)
				pauseManager.SetPause(true);
		}
		else
		{
			new SCR_RewindDialog();
		}
	}

	//---------------------------------------------------------------------------------------------
	override void OnInit(SCR_ActionsToolbarEditorUIComponent toolbar)
	{
		m_Toolbar = toolbar;
		GetGame().GetSaveGameManager().GetOnSaveGameCreated().Insert(OnSaveChanged);
		GetGame().GetSaveGameManager().GetOnSaveGameDeleted().Insert(OnSaveChanged);
	}

	//---------------------------------------------------------------------------------------------
	override void OnExit(SCR_ActionsToolbarEditorUIComponent toolbar)
	{
		GetGame().GetSaveGameManager().GetOnSaveGameCreated().Remove(OnSaveChanged);
		GetGame().GetSaveGameManager().GetOnSaveGameDeleted().Remove(OnSaveChanged);
	}

	//---------------------------------------------------------------------------------------------
	protected void OnSaveChanged(SaveGame save)
	{
		if (m_Toolbar)
			m_Toolbar.MarkForRefresh();
	}
}
