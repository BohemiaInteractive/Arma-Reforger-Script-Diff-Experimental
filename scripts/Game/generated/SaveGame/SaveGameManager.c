/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup SaveGame
\{
*/

sealed class SaveGameManager
{
	//! Event for listening for save game creation externally, mostly for UI purposes.
	protected ref ScriptInvokerBase<SaveGameDelegate> m_OnSaveGameCreated = new ScriptInvokerBase<SaveGameDelegate>();
	ScriptInvokerBase<SaveGameDelegate> GetOnSaveGameCreated()
	{
	    return m_OnSaveGameCreated;
	}
	//! Event for listening for save game removal.
	protected ref ScriptInvokerBase<SaveGameDelegate> m_OnSaveGameDeleted = new ScriptInvokerBase<SaveGameDelegate>();
	ScriptInvokerBase<SaveGameDelegate> GetOnSaveGameDeleted()
	{
	    return m_OnSaveGameDeleted;
	}

	//! Remember info for save-point creation and kick off transition to mission load
	proto external void StartPlaythrough(ResourceName missionResource, string optionalName = string.Empty, bool doTransition = true);
	//! True if saving is currently possible
	proto external bool IsSavingPossible();
	/*!
	Allow or disallow saving. Can be used to avoid auto-save or manual save during cutscenes or other non-saveable events.
	If an auto-save was due it will execute as soon as it allowed again. Manual saves requested are ignored and will not be resumed later.
	*/
	proto external void SetSavingAllowed(bool allowed);
	//! True if saving is currently possible AND allowed
	proto external bool IsSavingAllowed();
	/*!
	Request a new save point to be created. It will be created as soon as saving is possible.
	Callback is invoked after save was created and comitted to storage.
	Returns false if a save was already requested or saving is currently not possible (e.g. scripted scene).
	*/
	proto external bool RequestSavePoint(ESaveGameType type, string displayName = string.Empty, SaveGameOperationCb callback = null);
	//! Trigger a SHUTDOWN type save and exit (to menu or kill app on DS). Callback invokes after save, but before exit.
	proto external void RequestSaveAndExit(string displayName = string.Empty, SaveGameOperationCb callback = null);
	//! Get busy state
	proto external bool IsBusy();
	/*!
	Attempts to retrieve save game info from storage source.
	Filter can be filled with mission resource names. If no filter is provided it loads all.
	Returns false if an operation is already in progress
	*/
	proto external bool RetrieveSaveGameInfo(array<ResourceName> missionfilter = null, SaveGameOperationCb callback = null);
	/*!
	Access data for known save game info (RetrieveSaveGameInfo has to complete first to load existing saves).
	Returns count of save games matching missing filter.
	*/
	proto external int GetSaves(notnull out array<SaveGame> outSaveGames, ResourceName missionfilter = ResourceName.Empty);
	//! Get save game currently being played
	proto external SaveGame GetActiveSave();
	//! Resets the active save to none. So e.g. quick load no longer uses it.
	proto external void ResetActiveSave();
	//! Continue the game from this save point
	proto external void Load(notnull SaveGame saveGame, bool doTransition = true);
	//! Delete the save point
	proto external void Delete(notnull SaveGame saveGame, SaveGameOperationCb callback = null);
	//! Delete all save points of the given playthrough
	proto external void DeletePlaythrough(ResourceName mission, int playthrough, SaveGameOperationCb callback = null);
	//! Get the currently active mission name (Mission header or world file)
	static proto ResourceName GetCurrentMissionResource();

	// callbacks

	//! Called when a new save point was created
	event void OnSaveCreated(SaveGame save) { m_OnSaveGameCreated.Invoke(save); };
	//! Called when a new save point was created
	event void OnSaveDeleted(SaveGame save) { m_OnSaveGameDeleted.Invoke(save); };
}

/*!
\}
*/
