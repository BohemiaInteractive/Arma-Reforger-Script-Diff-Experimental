[ComponentEditorProps(category: "GameScripted/Editor", description: "Game simulation manager. Works only with SCR_EditorManagerEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_PauseGameTimeEditorComponentClass : SCR_BaseEditorComponentClass
{
}

/** @ingroup Editor_Components
*/
class SCR_PauseGameTimeEditorComponent : SCR_BaseEditorComponent
{
	[Attribute("0", desc: "When enabled, the editor will be paused when opened.")]
	protected bool m_bPauseOnOpen;

	protected ChimeraWorld m_World;
	protected bool m_bPendingUnpause;

	//------------------------------------------------------------------------------------------------
	/*!
	Toggle pause mode, e.g., pause the game when it's not paused.
	*/
	void TogglePause()
	{
		SetPause(!m_World.IsGameTimePaused());
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set pause mode.
	\param pause True to pause the game, false to unpause it
	*/
	void SetPause(bool pause)
	{
		//--- Unpausing game, create a rewind point
		if (!pause)
		{
			SCR_RewindComponent rewindManager = SCR_RewindComponent.GetInstance();
			if (rewindManager && rewindManager.CanRewind() && !rewindManager.HasRewindPoint())
			{
				m_bPendingUnpause = true;
				rewindManager.CreateRewindPoint();
				return;
			}
		}

		DoPause(pause);
	}

	//------------------------------------------------------------------------------------------------
	protected void DoPause(bool pause)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.PauseGame(pause, SCR_EPauseReason.EDITOR | SCR_EPauseReason.MUSIC);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set if the game should be paused when the editor is opened.
	\param pause True to pause the game whent he editor is opened
	*/
	void SetPauseOnOpen(bool pause)
	{
		m_bPauseOnOpen = pause;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	\return True if the game is set to be paused when the editor is opened
	*/
	bool IsPauseOnOpen()
	{
		return m_bPauseOnOpen;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSaved(SaveGame save)
	{
		if (m_bPendingUnpause)
		{
			m_bPendingUnpause = false;
			DoPause(false);
			return;
		}

		if (save.GetType() == ESaveGameType.MANUAL)
			DoPause(true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpen()
	{
		GetGame().GetSaveGameManager().GetOnSaveGameCreated().Insert(OnSaved);

		if (m_bPauseOnOpen)
		{
			//--- Pause game time when the editor is opened (only after a delay, to give player camera chance to iniliazed after rewinding)
			GetGame().GetCallqueue().Call(SetPause, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorClose()
	{
		GetGame().GetSaveGameManager().GetOnSaveGameCreated().Remove(OnSaved);

		//--- Always unpause the game when leaving the editor
		SetPause(false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorInit()
	{
		m_World = GetGame().GetWorld();

		//--- Don't pause on open when disabled by CLI param in the dev version
		if (GetGame().IsDev() && System.IsCLIParam("editorDoNotPauseOnOpen"))
			m_bPauseOnOpen = false;
	}
}
