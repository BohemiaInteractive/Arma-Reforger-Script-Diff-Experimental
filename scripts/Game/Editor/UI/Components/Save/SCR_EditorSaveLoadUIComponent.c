class SCR_EditorSaveLoadUIComponent : SCR_SaveDialogUIComponent
{
	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm(SCR_InputButtonComponent button, string actionName)
	{
		// Save dialogs
		if (!m_bIsLoad)
		{
			string customName = m_SaveNameInput.GetValue();
			/*if (!GetGame().GetSaveManager().FileExists(m_eWriteSaveType, customName))
			{
				//--- Creating a new file - save directly
				OnConfirmPrompt();
				return;
			}*/

			//--- Confirm prompt
			m_ConfirmPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sConfirmPrompt);
			m_ConfirmPrompt.m_OnConfirm.Insert(OnConfirmPrompt);
			m_ConfirmPrompt.SetTitle(customName);
			return;
		}

		//--- Loading a file leads to restart, ask first
		
		const Widget focusedEntry = GetGame().GetWorkspace().GetFocusedWidget();
		const SCR_SaveDialogEntry entry = GetEntryByWidget(focusedEntry);
		if (!entry)
			return;

		const SaveGame save = entry.m_Save;
		
		const string displayName = GetSaveDisplayName(save);
		if (!save.IsSavePointGameVersionCompatible())
		{
			// Warning - incompatible version
			m_LoadBadVersionPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sLoadBadVersionPrompt);
			m_LoadBadVersionPrompt.m_OnConfirm.Insert(LoadEntry);
			m_LoadBadVersionPrompt.SetTitle(displayName);
			return;
		}

		if (!save.AreSavePointAddonsCompatible())
		{
			// Warning - incompatible addons
			m_LoadBadAddonsPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sLoadBadAddonsPrompt);
			m_LoadBadAddonsPrompt.m_OnConfirm.Insert(LoadEntry);
			m_LoadBadAddonsPrompt.SetTitle(displayName);
			return;
		}

		//--- Confirm prompt
		m_ConfirmPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sConfirmPrompt);
		m_ConfirmPrompt.m_OnConfirm.Insert(OnConfirmPrompt);
		m_ConfirmPrompt.SetTitle(displayName);
	}

	//------------------------------------------------------------------------------------------------
	override protected void SaveEntry()
	{
		const string customName = m_SaveNameInput.GetValue();
		GetGame().GetSaveGameManager().RequestSavePoint(ESaveGameType.MANUAL, customName);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeletePrompt()
	{
		if (!m_SelectedSave)
			return;

		GetGame().GetSaveGameManager().Delete(m_SelectedSave, new SaveGameOperationCb(handler: OnSaveDeleted));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSaveDeleted(Managed context, bool success)
	{
		if (!success)
			return;
		
		//--- Update GUI
		super.OnDeletePrompt();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void LoadEntry()
	{
		if (!m_SelectedSave)
			return;

		GetGame().GetSaveGameManager().Load(m_SelectedSave);

		/*
		SCR_ServerSaveRequestCallback uploadCallback = saveManager.GetUploadCallback();
		if (uploadCallback)
		{
			uploadCallback.SetOnSuccess(OnLoadEntryUploadResponse);
			uploadCallback.SetOnError(OnLoadEntryUploadError);
			m_LoadingOverlay = SCR_LoadingOverlayDialog.Create();
		}
		*/
	}

	//------------------------------------------------------------------------------------------------
	override protected void SelectEntry(Widget w, SaveGame save)
	{
		const string customName = GetSaveDisplayName(save);
		m_SaveNameInput.SetValue(customName);
		super.SelectEntry(w, save);
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdateButtons()
	{
		string customName = m_SaveNameInput.GetValue();
		bool isValid = !customName.IsEmpty();
		bool isOverride = false; //!m_bIsLoad && customName && GetGame().GetSaveManager().FileExists(m_eWriteSaveType, customName);

		if (m_bIsLoad)
			m_DeleteButton.SetEnabled(m_SelectedSave != null);
		else
			m_DeleteButton.SetEnabled(isOverride);

		m_OverrideButton.SetVisible(isOverride, false);
		m_OverrideButton.SetEnabled(isOverride && isValid);

		m_ConfirmButton.SetVisible(!isOverride, false);
		m_ConfirmButton.SetEnabled(!isOverride && isValid);
	}
}
