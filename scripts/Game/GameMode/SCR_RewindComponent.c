[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "")]
class SCR_RewindComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_RewindComponent : SCR_BaseGameModeComponent
{
	protected const static string s_RewindPointName = "Rewind";
	protected SaveGame m_RewindPoint;

	//------------------------------------------------------------------------------------------------
	//! \return Local instance of this component
	static SCR_RewindComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;

		return SCR_RewindComponent.Cast(gameMode.FindComponent(SCR_RewindComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! \return Check if a rewind point exists for this session.
	bool CanRewind()
	{
		return GetGame().GetSaveGameManager().IsSavingPossible();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Check if a rewind point exists for this session.
	bool HasRewindPoint()
	{
		return m_RewindPoint != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a rewind point for this session.
	void CreateRewindPoint()
	{
		if (!Replication.IsServer() || HasRewindPoint())
			return;

		GetGame().GetSaveGameManager().RequestSavePoint(ESaveGameType.AUTO, s_RewindPointName, new SaveGameOperationCb(handler: OnRewindPointCreated));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRewindPointCreated(Managed context, bool success)
	{
		if (!success)
			return;

		m_RewindPoint = GetGame().GetSaveGameManager().GetActiveSave();
	}

	//------------------------------------------------------------------------------------------------
	//! Remove existing rewind point for this session.
	void DeleteRewindPoint()
	{
		if (!m_RewindPoint)
			return;
		
		GetGame().GetSaveGameManager().Delete(m_RewindPoint);
		m_RewindPoint = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Rewind to existing rewind point of this session.
	void Rewind()
	{
		if (!m_RewindPoint)
			return;

		GetGame().GetSaveGameManager().Load(m_RewindPoint);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;

		const SaveGameManager manager = GetGame().GetSaveGameManager();
		manager.GetOnSaveGameCreated().Insert(OnSavedCreated);

		const ResourceName currentMission = manager.GetCurrentMissionResource();
		manager.RetrieveSaveGameInfo({currentMission}, new SaveGameOperationCb(handler: OnSavesLoaded))
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;
		
		GetGame().GetSaveGameManager().GetOnSaveGameCreated().Remove(OnSavedCreated);
		DeleteRewindPoint();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSavesLoaded(Managed context, bool success)
	{
		RemoveUnusedRewindPoints();
	}

	//------------------------------------------------------------------------------------------------
	//! Remove any rewind points that were not cleaned up from last playthrough.
	protected void RemoveUnusedRewindPoints()
	{
		const SaveGameManager manager = GetGame().GetSaveGameManager();
		const SaveGame activeSave = manager.GetActiveSave();
		const ResourceName currentMission = manager.GetCurrentMissionResource();

		array<SaveGame> saves();
		manager.GetSaves(saves, currentMission);
		foreach (SaveGame save : saves)
		{
			if (save == activeSave || save.GetType() != ESaveGameType.AUTO || save.GetSavePointName() != s_RewindPointName)
				continue;

			manager.Delete(save);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsRewindPoint(SaveGame save)
	{
		return (save == m_RewindPoint) || (save.GetType() == ESaveGameType.AUTO && save.GetSavePointName() == s_RewindPointName);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSavedCreated(SaveGame save)
	{
		if (save.GetType() == ESaveGameType.MANUAL)
			DeleteRewindPoint();
	}
}

class SCR_RewindDialog : SCR_ConfigurableDialogUi
{
	//------------------------------------------------------------------------------------------------
	void SCR_RewindDialog()
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "rewind", this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		SCR_RewindComponent.GetInstance().Rewind();
	}
}
