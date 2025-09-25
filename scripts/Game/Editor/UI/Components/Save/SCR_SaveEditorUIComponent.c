class SCR_SaveEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute()]
	protected bool m_bIsLoad;
	
	[Attribute()]
	protected bool m_bCurrentMissionOnly;
	
	[Attribute("SaveScroller")]
	protected string m_sScrollWidgetName;
	
	[Attribute("SaveList")]
	protected string m_sListWidgetName;
	
	[Attribute("SaveNameInput")]
	protected string m_sNameInputWidgetName;
	
	[Attribute("ExitButton")]
	protected string m_sCloseButtonWidgetName;
	
	[Attribute("DeleteButton")]
	protected string m_sDeleteButtonWidgetName;
	
	[Attribute("OverrideButton")]
	protected string m_sOverrideButtonWidgetName;
	
	[Attribute("ConfirmButton")]
	protected string m_sConfirmButtonWidgetName;
	
	[Attribute("ConfirmPrompt")]
	protected string m_sConfirmPromptWidgetName;
	
	[Attribute("DeletePrompt")]
	protected string m_sDeletePromptWidgetName;
	
	[Attribute("LoadBadVersionPrompt")]
	protected string m_sLoadBadVersionPromptWidgetName;
	
	[Attribute("LoadBadAddonsPrompt")]
	protected string m_sLoadBadAddonsPromptWidgetName;
	
	[Attribute("EntryName")]
	protected string m_sEntryNameWidgetName;
	
	[Attribute("SaveMetaLine")]
	protected string m_sEntryMeta;
	
	[Attribute("PreviewDate")]
	protected string m_sEntryDateWidgetName;
	
	[Attribute("PreviewTime")]
	protected string m_sEntryTimeWidgetName;
	
	[Attribute("PreviewWorld")]
	protected string m_sEntryMissionNameWidgetName;
	
	[Attribute("EntryVersion")]
	protected string m_sEntryVersionWidgetName;
	
	[Attribute("PreviewImage")]
	protected string m_sEntryImageWidgetName;
	
	[Attribute("SaveImage")]
	protected string m_sEntryIconWidgetName;
	
	[Attribute("false", UIWidgets.CheckBox)]
	protected bool m_bVerboseDate;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_sCreateLayout;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_sEntryLayout;
	
	[Attribute("session_save_override")]
	protected string m_sConfirmPrompt;
	
	[Attribute("session_delete")]
	protected string m_sDeletePrompt;
	
	[Attribute("session_load_bad_version")]
	protected string m_sLoadBadVersionPrompt;
	
	[Attribute("session_load_bad_addons")]
	protected string m_sLoadBadAddonsPrompt;
	
	protected Widget m_wRoot;
	protected ScrollLayoutWidget m_wScroll;
	protected Widget m_wList;
	protected Widget m_wLastFocusedEntry;
	
	protected SCR_InputButtonComponent m_DeleteButton;
	protected SCR_InputButtonComponent m_OverrideButton;
	protected SCR_InputButtonComponent m_ConfirmButton;
	protected SCR_EditBoxComponent m_SaveNameInput;
	
	protected SCR_ConfigurableDialogUi m_ConfirmPrompt;
	protected SCR_ConfigurableDialogUi m_DeletePrompt;
	protected SCR_ConfigurableDialogUi m_LoadBadVersionPrompt;
	protected SCR_ConfigurableDialogUi m_LoadBadAddonsPrompt;
	protected SCR_LoadingOverlayDialog m_LoadingOverlay;
	
	protected float m_fSliderPosY = -1;
	protected Widget m_wSelectedWidget;
	protected string m_sSelectedFileName;
	
	protected ref map<Widget, string> m_mEntries = new map<Widget, string>();
	protected ref map<string, string> m_mEntryNames = new map<string, string>(); //--- file name, display name
	protected ref array<Widget> m_aEntriesHidden = {};
	protected ref array<Widget> m_aEntriesToShow = {};

	//////////////////////////////////////////////////////////////////////////////////////////
	// Control buttons
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void OnClose(SCR_InputButtonComponent button, string actionName)
	{
		CloseMenu();
	}

	//----------------------------------------------------------------------------------------
	protected void OnConfirm(SCR_InputButtonComponent button, string actionName)
	{
		// Save dialogs
		if (!m_bIsLoad)
		{
			string customName = m_SaveNameInput.GetValue();
			if (false /*GetGame().GetSaveManager().FileExists(m_eWriteSaveType, customName)*/)
			{
				//--- Confirm prompt
				m_ConfirmPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sConfirmPrompt);
				m_ConfirmPrompt.m_OnConfirm.Insert(OnConfirmPrompt);
			}
			else
			{
				//--- Creating a new file - save directly
				OnConfirmPrompt();
				return;
			}
			
			m_ConfirmPrompt.SetTitle(m_SaveNameInput.GetValue());
			return;
		}
		
		//--- Loading a file leads to restart, ask first
		string displayName = m_mEntryNames[m_sSelectedFileName];
		string fileName;
		if (m_mEntries.Find(GetGame().GetWorkspace().GetFocusedWidget(), fileName))
		{
			/*
			SCR_MetaStruct meta = GetGame().GetSaveManager().GetMeta(fileName);
			if (meta)
			{
				if (!meta.IsVersionCompatible())
				{
					// Warning - incompatible version
					m_LoadBadVersionPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sLoadBadVersionPrompt);
					m_LoadBadVersionPrompt.m_OnConfirm.Insert(LoadEntry);
					m_LoadBadVersionPrompt.SetTitle(displayName);
					return;
				}
				else if (!meta.AreAddonsCompatible())
				{
					// Warning - incompatible addons
					m_LoadBadAddonsPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sLoadBadAddonsPrompt);
					m_LoadBadAddonsPrompt.m_OnConfirm.Insert(LoadEntry);
					m_LoadBadAddonsPrompt.SetTitle(displayName);
					return;
				}
			}
			*/
			
			//--- Confirm prompt
			m_ConfirmPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sConfirmPrompt);
			m_ConfirmPrompt.m_OnConfirm.Insert(OnConfirmPrompt);
			m_ConfirmPrompt.SetTitle(displayName);
		}
	}

	//----------------------------------------------------------------------------------------
	protected void OnConfirmPrompt()
	{
		if (m_bIsLoad)
			LoadEntry();
		else
			SaveEntry();

		CloseMenu();
	}
	
	//----------------------------------------------------------------------------------------
	//! Callback on clicking delete button or actoin
	protected void OnDelete(SCR_InputButtonComponent button, string actionName)
	{	
		// Open delete dialog
		m_DeletePrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sDeletePrompt); //--- ToDo: Unique tag
		m_DeletePrompt.m_OnConfirm.Insert(OnDeletePrompt);
		
		// Setup string
		string displayName = m_mEntryNames[m_sSelectedFileName];
		m_DeletePrompt.SetTitle(displayName);
	}

	//----------------------------------------------------------------------------------------
	protected void OnDeletePrompt()
	{
		//--- Delete the file
		//GetGame().GetSaveManager().Delete(m_sSelectedFileName);
		
		//--- Update GUI
		m_mEntries.Remove(m_wSelectedWidget);
		m_mEntryNames.Remove(m_sSelectedFileName);
		
		m_wSelectedWidget.RemoveFromHierarchy();
		SelectEntry(null, string.Empty);
	}
	
	//----------------------------------------------------------------------------------------
	protected void CloseMenu()
	{
		//--- Confirmation prompt opened, ignore
		if (GetGame().GetWorkspace().GetModal() != m_wRoot)
			return;
		
		MenuBase menu = MenuBase.Cast(m_wRoot.FindHandler(MenuBase));
		menu.Close();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Main operations
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void SaveEntry()
	{
		string customName = m_SaveNameInput.GetValue();
		//GetGame().GetSaveManager().Save(m_eWriteSaveType, customName);
	}
	
	//----------------------------------------------------------------------------------------
	protected void LoadEntry()
	{
		string fileName;
		if (m_mEntries.Find(m_wLastFocusedEntry, fileName))
		{
			/*
			// Load save 		
			SCR_SaveManagerCore saveManager = GetGame().GetSaveManager();
			saveManager.RestartAndLoad(fileName);
			
			// Handle server load
			if (!Replication.IsClient())
				return;
			
			BackendCallback uploadCallback = saveManager.GetUploadCallback();
			if (uploadCallback)
			{
				uploadCallback.SetOnSuccess(OnLoadEntryUploadResponse);
				uploadCallback.SetOnError(OnLoadEntryUploadError);
				m_LoadingOverlay = SCR_LoadingOverlayDialog.Create();
			}
			*/
		}
	}
	
	//----------------------------------------------------------------------------------------
	protected void OnLoadEntryUploadResponse()
	{
		CloseMenu();
	}
	
	//----------------------------------------------------------------------------------------
	protected void OnLoadEntryUploadError()
	{	
		SCR_CommonDialogs.CreateRequestErrorDialog();
		
		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();
		
		CloseMenu();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Interaction
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void SelectEntry(Widget w, string fileName)
	{
		const string customName =  ""; //GetGame().GetSaveManager().GetCustomName(fileName);
		m_SaveNameInput.SetValue(customName);

		m_wSelectedWidget = w;		
		m_sSelectedFileName = fileName;
		
		UpdateButtons();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateButtons()
	{
		string customName = m_SaveNameInput.GetValue();
		bool isValid = !customName.IsEmpty();
		bool isOverride = !m_bIsLoad && customName && false; //GetGame().GetSaveManager().FileExists(m_eWriteSaveType, customName);
		
		if (m_bIsLoad)
			m_DeleteButton.SetEnabled(m_sSelectedFileName && false /*GetGame().GetSaveManager().FileExists(m_sSelectedFileName)*/);
		else
			m_DeleteButton.SetEnabled(isOverride);
		
		m_OverrideButton.SetVisible(isOverride, false);
		m_OverrideButton.SetEnabled(isOverride && isValid);
		
		m_ConfirmButton.SetVisible(!isOverride, false);
		m_ConfirmButton.SetEnabled(!isOverride && isValid);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFrame()
	{
		//--- Ignore if all entries were shown
		if (m_aEntriesHidden.IsEmpty())
		{
			GetGame().GetCallqueue().Remove(OnFrame);
			return;
		}
		
		float sliderPosX, sliderPosY;
		m_wScroll.GetSliderPos(sliderPosX, sliderPosY);
		
		if (sliderPosY != m_fSliderPosY)
		{
			float scrollPosX, scrollPosY, scrollSizeW, scrollSizeH;
			m_wScroll.GetScreenPos(scrollPosX, scrollPosY);
			m_wScroll.GetScreenSize(scrollSizeW, scrollSizeH);
			
			//--- Widget not loaded yet, terminate
			if (scrollSizeH == 0)
				return;
			
			//--- Find widgets in view
			foreach (int i, Widget entryWidget: m_aEntriesHidden)
			{
				//--- Already shown
				if (entryWidget.GetOpacity() > 0)
					continue;
				
				float posX, posY, sizeW, sizeH;
				entryWidget.GetScreenPos(posX, posY);
				entryWidget.GetScreenSize(sizeW, sizeH);
				
				if ((posY + sizeH) > scrollPosY && posY < (scrollPosY + scrollSizeH))
				{
					//--- When the entry is in the scrolled view, mark it for showing
					if (!m_aEntriesToShow.Contains(entryWidget))
						m_aEntriesToShow.Insert(entryWidget);
				}
				else
				{
					//--- When the entry is not in the scrolled view, removing it from queue of entries to show again (to prioritize entries that are actually shown)
					m_aEntriesToShow.RemoveItemOrdered(entryWidget);
				}
			}
		}
		
		//--- Process the queue of entries to show, with one entry per frame
		if (!m_aEntriesToShow.IsEmpty())
		{
			Widget entryWidget = m_aEntriesToShow[0];
			if (entryWidget.GetOpacity() == 0)
			{
				TextWidget entryNameWidget = TextWidget.Cast(entryWidget.FindAnyWidget(m_sEntryNameWidgetName));
				ImageWidget entryImageWidget = ImageWidget.Cast(entryWidget.FindAnyWidget(m_sEntryImageWidgetName));
				ImageWidget entryIconWidget = ImageWidget.Cast(entryWidget.FindAnyWidget(m_sEntryIconWidgetName));
				
				string fileName = m_mEntries[entryWidget];
				string entryName = ""; //GetGame().GetSaveManager().GetCustomName(fileName);
				
				/*
				SCR_MetaStruct meta = GetGame().GetSaveManager().GetMeta(fileName);
				if (meta && meta.IsValid())
				{
					int y, m, d, hh, mm;
					meta.GetDateAndTime(y, m, d, hh, mm);
					
					SCR_UIInfo info = GetGame().GetSaveManager().GetSaveTypeInfo(fileName);
					string displayName = entryName;
					Color imageColor = Color.FromInt(Color.WHITE);
					if (info)
					{
						info.SetIconTo(entryIconWidget);
						displayName = info.GetName() + " " + displayName; //--- Hardcoded space separator, ToDo: Solve using %1 in the string itself?
						
						SCR_ColorUIInfo colorInfo = SCR_ColorUIInfo.Cast(info);
						if (colorInfo)
							imageColor = colorInfo.GetColor();
					}
					m_mEntryNames.Insert(fileName, displayName);
					
					entryNameWidget.SetText(displayName);
					
					TextWidget entryDateWidget = TextWidget.Cast(entryWidget.FindAnyWidget(m_sEntryDateWidgetName));
					entryDateWidget.SetText(SCR_DateTimeHelper.GetDateString(d, m, y, m_bVerboseDate));
					
					TextWidget entryTimeWidget = TextWidget.Cast(entryWidget.FindAnyWidget(m_sEntryTimeWidgetName));
					entryTimeWidget.SetText(SCR_FormatHelper.GetTimeFormattingHoursMinutes(hh, mm));
					
					TextWidget entryVersionWidget = TextWidget.Cast(entryWidget.FindAnyWidget(m_sEntryVersionWidgetName));
					string versionText;
					bool isVersionCompatible = meta.IsVersionCompatible(versionText);
					entryVersionWidget.SetText(versionText);
					if (!isVersionCompatible)
						entryVersionWidget.SetColor(UIColors.WARNING);
					
					ResourceName headerResourceName = meta.GetHeaderResource();
					if (!headerResourceName.IsEmpty())
					{
						Resource missionHeaderResource = Resource.Load(headerResourceName);
						if (missionHeaderResource.IsValid())
						{
							SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(BaseContainerTools.CreateInstanceFromContainer(missionHeaderResource.GetResource().ToBaseContainer()));
							
							TextWidget entryMissionNameWidget = TextWidget.Cast(entryWidget.FindAnyWidget(m_sEntryMissionNameWidgetName));
							entryMissionNameWidget.SetText(missionHeader.m_sName);
							
							if (missionHeader.m_sIcon)
							{
								entryImageWidget.LoadImageTexture(0, missionHeader.m_sIcon);
								entryImageWidget.SetColor(imageColor);
							}
						}
					}
					
					if (!meta.AreAddonsCompatible())
					{
						entryIconWidget.SetColor(UIColors.WARNING);
					}

				}
				else
				{
					//--- Meta file missing or invalid
					entryNameWidget.SetText(entryName);
					entryImageWidget.SetVisible(false);
					
					Widget entryMetaWidget = entryWidget.FindAnyWidget(m_sEntryMeta);
					entryMetaWidget.SetVisible(false);
					
					entryIconWidget.SetColor(Color.FromInt(UIColors.WARNING.PackToInt()));
				}
				*/
				
				AnimateWidget.Opacity(entryWidget, 1, 3);
			}
			else if (entryWidget.GetOpacity() > 0)
			{
				//--- Started fading in, move on to the next entry
				m_aEntriesToShow.RemoveOrdered(0);
				m_aEntriesHidden.RemoveItemOrdered(entryWidget);
			}
		}
		
		m_fSliderPosY = sliderPosY;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Overrides
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w.GetParent() != m_wList)
			return false;
		
		//--- Save entry
		string fileName;
		if (m_mEntries.Find(w, fileName))
			SelectEntry(w, fileName);
		
		return false;
	}
	
	//----------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		string fileName;
		
		if (m_mEntries.Find(w, fileName))
		{
			m_wLastFocusedEntry = w;
			SelectEntry(w, fileName);
		}
		
		return false;
	}
	
	//----------------------------------------------------------------------------------------
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (w.GetParent() != m_wList)
			return false;
		
		//--- Select
		OnClick(w, x, y, button);
		
		//--- Activate
		OnConfirm(null, string.Empty);
		
		return false;
	}
	
	//----------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (!GetGame().InPlayMode())
			return;
		
		//--- Find all widgets
		m_wRoot = w;
		m_wList = w.FindAnyWidget(m_sListWidgetName);
		m_wScroll = ScrollLayoutWidget.Cast(w.FindAnyWidget(m_sScrollWidgetName));
		
		//--- Name input field
		m_SaveNameInput = SCR_EditBoxComponent.GetEditBoxComponent(m_sNameInputWidgetName, w);
		m_SaveNameInput.m_OnChanged.Insert(UpdateButtons);
		
		//--- Assign control buttons
		Widget closeButtonWidget = w.FindAnyWidget(m_sCloseButtonWidgetName);
		Widget deleteButtonWidget = w.FindAnyWidget(m_sDeleteButtonWidgetName);
		Widget overrideButtonWidget = w.FindAnyWidget(m_sOverrideButtonWidgetName);
		Widget confirmButtonWidget = w.FindAnyWidget(m_sConfirmButtonWidgetName);
		
		SCR_InputButtonComponent closeButton = SCR_InputButtonComponent.Cast(closeButtonWidget.FindHandler(SCR_InputButtonComponent));
		m_DeleteButton = SCR_InputButtonComponent.Cast(deleteButtonWidget.FindHandler(SCR_InputButtonComponent));
		m_OverrideButton = SCR_InputButtonComponent.Cast(overrideButtonWidget.FindHandler(SCR_InputButtonComponent));
		m_ConfirmButton = SCR_InputButtonComponent.Cast(confirmButtonWidget.FindHandler(SCR_InputButtonComponent));
		
		closeButton.m_OnActivated.Insert(OnClose);
		m_DeleteButton.m_OnActivated.Insert(OnDelete);
		m_OverrideButton.m_OnActivated.Insert(OnConfirm);
		m_ConfirmButton.m_OnActivated.Insert(OnConfirm);
		
		UpdateButtons();
		
		//--- Clear the list first
		SCR_WidgetHelper.RemoveAllChildren(m_wList);
		
		//--- Create new entries
		array<string> fileNames = {};
		const int fileCount = 0; //GetGame().GetSaveManager().GetLocalSaveFiles(fileNames, m_eReadSaveTypes, m_bCurrentMissionOnly);
		fileNames.Sort();
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		//--- Used for testing to artifically inflate the amount of entries
		const int debugCoef = 1;
		
		Widget entryWidget;
		TextWidget entryNameWidget;
		for (int i = 0; i < fileCount * debugCoef; i++)
		{
			int ii = i % fileCount;
			string fileName = fileNames[ii];
			entryWidget = workspace.CreateWidgets(m_sEntryLayout, m_wList);
			m_mEntries.Insert(entryWidget, fileName);
			m_aEntriesHidden.Insert(entryWidget);
			
			//--- Hide by default
			entryWidget.SetOpacity(0);
			
			if (m_bIsLoad && i == 0)
				SelectEntry(entryWidget, fileName);
		}
		
		//--- Initiate periodic check which will load and show metadata only for entries that are actually shown. Doing it all at once here would be too expensive.
		GetGame().GetCallqueue().CallLater(OnFrame, 1, true);
	}
	
	//----------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		GetGame().GetCallqueue().Remove(OnFrame);
	}
}
