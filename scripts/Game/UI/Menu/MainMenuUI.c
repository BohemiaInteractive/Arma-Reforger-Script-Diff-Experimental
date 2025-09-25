class SCR_MissionMetaData
{
	MissionWorkshopItem m_Item;
	int m_iScore;
	bool m_bFavorite;
	bool m_bRecent;
	bool m_bRecommended;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] item
	//! \param[in] score
	void SCR_MissionMetaData(MissionWorkshopItem item, int score)
	{
		m_Item = item;
		m_iScore = score;
		m_bFavorite = false;
		m_bRecent = false;
		m_bRecommended = false;
	}
}

class MainMenuUI : ChimeraMenuBase
{
	protected const ResourceName TILES_LAYOUT = "{2D4191089E061C18}UI/layouts/Menus/PlayMenu/MainMenuTile_Vertical.layout";
	protected const ResourceName TILES_GM_LAYOUT = "{ECA6FD4399DCD621}UI/layouts/Menus/PlayMenu/MainMenuTile_Menu_Vertical.layout";
	protected const ResourceName TILES_KS_LAYOUT = "{0DE8AE59C27B5181}UI/layouts/Menus/PlayMenu/MainMenuTile_Campaign.layout";
	protected const ResourceName CONFIG = "{6DDC861718434B94}Configs/ContentBrowser/MainMenu/MainMenuEntries.conf";

	protected SCR_MenuTileComponent m_FocusedTile;
	protected DialogUI m_BannedDetectionDialog;

	protected ref array<string> m_aBannedItems = {};
	protected static ref array<ref SCR_NewsEntry> m_aNews = {};
	protected static ref array<ref SCR_NewsEntry> m_aNotifications = {};
	protected ref array<ref Widget> m_aTiles = {};

	protected static bool s_bDidCheckNetwork;
	protected static const int SERVICES_STATUS_CHECK_DELAY = 2000; // needed for backend API to prepare
	protected bool m_bFirstLoad;

	// Privileges callback
	protected ref SCR_ScriptPlatformRequestCallback m_CallbackGetPrivilege;

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpen()
	{
		super.OnMenuOpen();

		// Init common Workshop UI
		SCR_WorkshopUiCommon.OnGameStart();

		Widget w = GetRootWidget();
		Widget bottomContent = w.FindAnyWidget("BottomContentHorizontal");
		Widget descriptionRoot = w.FindAnyWidget("Description");
		Widget footer = w.FindAnyWidget("Footer");
		if (!bottomContent || !descriptionRoot || !footer)
			return;

		PrepareTiles();

		// Listen to buttons in grid
		Widget child = bottomContent.GetChildren();
		SCR_MenuTileComponent tile;
		while (child)
		{
			tile = SCR_MenuTileComponent.Cast(child.FindHandler(SCR_MenuTileComponent));
			if (!tile)
				break;

			if (!m_FocusedTile)
				m_FocusedTile = tile;

			tile.m_OnClicked.Insert(OnTileClick);
			tile.m_OnFocused.Insert(OnTileFocus);
			child = child.GetSibling();
		}

		Widget mainTile = w.FindAnyWidget("MainMenuTile");
		tile = SCR_MenuTileComponent.Cast(mainTile.FindHandler(SCR_MenuTileComponent));
		tile.m_OnClicked.Insert(OnTileClick);
		tile.m_OnFocused.Insert(OnTileFocus);

		// Subscribe to buttons
		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, footer);
		if (back)
		{
			// If on console, don't show the 'Exit Game' button
			if (GetGame().IsPlatformGameConsole())
				back.SetVisible(false, false);
			else
				back.m_OnActivated.Insert(OnBack);
		}

		// Services Status button
		SCR_InputButtonComponent servicesStatus = SCR_InputButtonComponent.GetInputButtonComponent("ServicesStatus", footer);
		if (servicesStatus)
			servicesStatus.m_OnActivated.Insert(OnServicesStatus);

		SCR_InputButtonComponent feedback = SCR_InputButtonComponent.GetInputButtonComponent("Feedback", footer);
		if (feedback)
			feedback.m_OnActivated.Insert(OnFeedback);

		SCR_InputButtonComponent credits = SCR_InputButtonComponent.GetInputButtonComponent("Credits", footer);
		if (credits)
			credits.m_OnActivated.Insert(OnCredits);

		#ifdef WORKBENCH
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_BACK_WB, EActionTrigger.DOWN, OnBack);
		#endif

		GetNotificationEntries(null);

		// Check Service Statuses
		if (!s_bDidCheckNetwork)
		{
			s_bDidCheckNetwork = true;
			GetGame().GetCallqueue().CallLater(CheckServicesStatusIfFocused, SERVICES_STATUS_CHECK_DELAY);
		}

		Widget logoWidget = w.FindAnyWidget("LogoButton");
		if (logoWidget)
		{
			SCR_ModularButtonComponent logo = SCR_ModularButtonComponent.FindComponent(logoWidget);
		if (logo)
			logo.m_OnClicked.Insert(OnLogoClicked);
		}

		// Hide news menu button (top right corner) on PS
		if (GetGame().GetPlatformService().GetLocalPlatformKind() == PlatformKind.PSN)
		{
			Widget newsButton = w.FindAnyWidget("NewsButton");
			if (newsButton)
			{
				newsButton.SetVisible(false);
				newsButton.SetEnabled(false);
			}
		}
		else
		{
			// Get the entries for the first time
			GetNewsEntries(null);
		}

		// Check ping sites
		GetGame().GetBackendApi().GetClientLobby().MeasureLatency(null);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpened()
	{
		super.OnMenuOpened();

		// Opening Last menu
		SCR_MenuLoadingComponent.LoadLastMenu();
		SCR_MenuLoadingComponent.ClearLastMenu();

		BaseContainer cont = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");
		if (cont)
		{
			cont.Get("m_bFirstTimePlay", m_bFirstLoad);
			cont.Set("m_bFirstTimePlay", false);
		}

		//PrintFormat("[OnMenuOpened] m_bFirstLoad: %1", m_bFirstLoad);

		// Save changes to the settings
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();

		// Check first start of session
		bool firstLoadInSession = GameSessionStorage.s_Data["m_bMenuFirstOpening"].IsEmpty();
		GameSessionStorage.s_Data["m_bMenuFirstOpening"] = "false";

		//PrintFormat("[OnMenuOpened] firstLoadInSession: %1", firstLoadInSession);

		if (!firstLoadInSession)
			return;

		if (!m_bFirstLoad)
			return;

		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.WelcomeDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuFocusLost()
	{
		GetRootWidget().SetEnabled(false);

		super.OnMenuFocusLost();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuFocusGained()
	{
		GetRootWidget().SetEnabled(true);

		if (m_FocusedTile)
			GetGame().GetWorkspace().SetFocusedWidget(m_FocusedTile.GetRootWidget(), true);

		PrepareTiles();
		
		super.OnMenuFocusGained();
	}
	//------------------------------------------------------------------------------------------------
	protected void LoadTutorial(SCR_MainMenuConfiguration menuConfig, notnull Widget root)
	{
		WorkshopApi workshopAPI = GetGame().GetBackendApi().GetWorkshop();
		ResourceName tutorial = menuConfig.m_TutorialScenario;
		MissionWorkshopItem itemTutorial = workshopAPI.GetInGameScenario(tutorial);

		bool playedTutorial;
		int playedTutorialCount;
		int playedTutorialMax;
		if (itemTutorial)
			playedTutorial = itemTutorial.GetTimeSinceLastPlay() > -1;

		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");
		if (settings)
		{
			settings.Get("m_iPlayTutorialShowCount", playedTutorialCount);
			settings.Get("m_iPlayTutorialShowMax", playedTutorialMax);
		}

		bool canShowTutorial = !playedTutorial && playedTutorialCount < playedTutorialMax;
		if (canShowTutorial)
			CreateTile(tutorial, root);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateTile(ResourceName mission, notnull Widget root, bool isRecommended = false)
	{
		MissionWorkshopItem item = SCR_ScenarioUICommon.GetInGameScenario(mission);
		if (!item)
			return;

		Widget childWidget = GetGame().GetWorkspace().CreateWidgets(TILES_LAYOUT, root);
		if (!childWidget)
			return;

		SCR_MainMenuTileComponent tile = SCR_MainMenuTileComponent.Cast(childWidget.GetChildren().FindHandler(SCR_MainMenuTileComponent));
		if (!tile)
			return;

		tile.ShowMission(item, isRecommended);
	}

	//------------------------------------------------------------------------------------------------
	protected void PrepareTiles()
	{
		Resource resource = BaseContainerTools.LoadContainer(CONFIG);
		if (!resource)
			return;

		BaseContainer container = resource.GetResource().ToBaseContainer();
		SCR_MainMenuConfiguration menuConfig = SCR_MainMenuConfiguration.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!menuConfig)
			return;

		Widget root = GetRootWidget().FindAnyWidget("ScenariosHorizontalLayoutDeep");
		const int maxCount = 8;

		// delete previous content
		SCR_WidgetHelper.RemoveAllChildren(root);
		
		// Tutorial Missions
		LoadTutorial(menuConfig, root);

		// GameMaster Tile
		//GetGame().GetWorkspace().CreateWidgets(TILES_GM_LAYOUT, root);
		// Campaign tile
		GetGame().GetWorkspace().CreateWidgets(TILES_KS_LAYOUT, root);

		// Get missions from Workshop API
		array<MissionWorkshopItem> missionItemsAll = {};
		array<ref SCR_MissionMetaData> topMissions = new array<ref SCR_MissionMetaData>();
		topMissions.Reserve(maxCount);

		WorkshopApi workshopApi = GetGame().GetBackendApi().GetWorkshop();
		workshopApi.GetPageScenarios(missionItemsAll, 0, SCR_WorkshopUiCommon.PAGE_SCENARIOS); // Get all missions at once

		// Remove scenarios from disabled addons
		SCR_WorkshopItem scriptedItem = null;
		
		for (int i = missionItemsAll.Count() - 1; i >= 0; i--)
		{
			MissionWorkshopItem m = missionItemsAll[i];
			WorkshopItem addon = m.GetOwner();

			if (!addon)
				continue;

			scriptedItem = SCR_AddonManager.GetInstance().GetItem(addon.Id());

			if (scriptedItem && scriptedItem.GetAnyDependencyMissing())
				missionItemsAll.Remove(i);
		}

		for (int i = missionItemsAll.Count() - 1; i >= 0; i--)
		{
			if (SCR_ScenarioSequenceProgress.IsScenarioLocked(missionItemsAll[i]))
				missionItemsAll.Remove(i);
		}

		// Store total number of entries
		int iEntriesTotal = missionItemsAll.Count();
		array<ResourceName> missionsRecommended = menuConfig.m_aMainMenuScenarios;

		foreach (MissionWorkshopItem item : missionItemsAll)
		{
			SCR_MissionMetaData newItem = new SCR_MissionMetaData(item, 0);

			int score = 0;
			if (missionsRecommended.Find(item.Id()) != -1)
			{
				newItem.m_bRecommended = true;
				score = 3;
			}

			if (item.IsFavorite())
			{
				score = score + 2;
				newItem.m_bFavorite = true;
			}
			int dt = item.GetTimeSinceLastPlay();
			if (dt >= 0)
			{
				score++;
				newItem.m_bRecent = true;
			}
			newItem.m_iScore = score;

			// Find insertion position (descending by score)
			int pos = topMissions.Count(); // default: append at end (smallest side)
			foreach (int j, SCR_MissionMetaData metadata : topMissions)
			{
				if (score > metadata.m_iScore) 
				{ 
					pos = j; 
					break; 
				}
			}
			if (pos == topMissions.Count())
			{
				// Append only if we still have room
				if (topMissions.Count() < maxCount)
					topMissions.Insert(newItem);
				// else: smaller than current topN tail → skip
			}
			else
			{
				// Insert and trim to maxCount
				topMissions.InsertAt(newItem, pos);
				if (topMissions.Count() > maxCount)
					topMissions.RemoveOrdered(topMissions.Count() - 1); // drop the smallest at the end
			}
		}

		foreach (SCR_MissionMetaData p : topMissions)
		{
			CreateTile(p.m_Item.Id(), root, p.m_bRecommended);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack()
	{
		if (!IsFocused())
			return;

		TryExitGame();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCredits()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CreditsMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileClick(notnull SCR_MenuTileComponent comp)
	{
		if (!IsFocused())
			return;

		switch (comp.m_eMenuPreset)
		{
			case ChimeraMenuPreset.FeedbackDialog:
			{
				GetGame().GetMenuManager().OpenDialog(comp.m_eMenuPreset);
				break;
			}
			case ChimeraMenuPreset.ContentBrowser:
			{
				SCR_WorkshopUiCommon.TryOpenWorkshop();
				break;
			}
			case ChimeraMenuPreset.ServerBrowserMenu:
			{
				ServerBrowserMenuUI.TryOpenServerBrowser();
				break;
			}
			default:
			{
				GetGame().GetMenuManager().OpenMenu(comp.m_eMenuPreset);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileFocus(SCR_MenuTileComponent comp)
	{
		if (!IsFocused())
			return;

		m_FocusedTile = comp;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonSelect()
	{
		if (!IsFocused())
			return;

		if (m_FocusedTile)
	//!
			OnTileClick(m_FocusedTile);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnServicesStatus()
	{
		if (!IsFocused())
			return;

		SCR_CommonDialogs.CreateServicesStatusDialog();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFeedback()
	{
		if (!IsFocused())
			return;

		SCR_FeedbackDialogUI.OpenFeedbackDialog();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLogoClicked()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.WelcomeDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected void RestoreFocus()
	{
	//!
	//! \param[out] entries
	//! \return
		if (m_FocusedTile)
			GetGame().GetWorkspace().SetFocusedWidget(m_FocusedTile.GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] entries
	//! \return
	static int GetNewsEntries(out array<ref SCR_NewsEntry> entries = null)
	{
		if (entries)
			entries.Clear();

		m_aNews.Clear();

		// Print( "NEWS: Count - " + GetGame().GetBackendApi().GetNewsCount() );
		for (int i, cnt = GetGame().GetBackendApi().GetNewsCount(); i < cnt; i++)
		{
			NewsFeedItem item = GetGame().GetBackendApi().GetNewsItem(i);
			if (item)
				m_aNews.Insert(new SCR_NewsEntry(item));
		}
	//!
	//! \param[out] entries
	//! \return

		if (entries)
			entries = m_aNews;

		return m_aNews.Count();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] entries
	//! \return
	static int GetNotificationEntries(out array<ref SCR_NewsEntry> entries = null)
	{
		if (entries)
			entries.Clear();

		m_aNotifications.Clear();

		//Print( "NOTIFICATIONS: Count - " + GetGame().GetBackendApi().GetNotifyCount() );
		for (int i, cnt = GetGame().GetBackendApi().GetNotifyCount(); i < cnt; i++)
		{
			NewsFeedItem item = GetGame().GetBackendApi().GetNotifyItem(i);
	//!
	//! \return
			if (item)
				m_aNews.Insert(new SCR_NewsEntry(item));
		}

		if (entries)
			entries = m_aNotifications;

		return m_aNotifications.Count();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//!
	//! \return
	//! \return
	static int GetUnreadNewsCount()
	{
		int count;
		foreach (SCR_NewsEntry entry : m_aNews)
		{
			if (!entry.m_bRead)
				count++;
		}
		return count;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static int GetUnreadNotificationCount()
	{
		int count;
		foreach (SCR_NewsEntry entry : m_aNotifications)
		{
			if (!entry.m_bRead)
				count++;
		}
		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Shows exit game dialog, or some other dialog if something should
	//! prevent user from exiting the game through the main menu
	protected static void TryExitGame()
	{
		int nCompleted, nTotal;
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr)
			mgr.GetDownloadQueueState(nCompleted, nTotal);

		if (nTotal > 0)
			new SCR_ExitGameWhileDownloadingDialog();
		else
			new SCR_ExitGameDialog();
	}

	//------------------------------------------------------------------------------------------------
	//! Check Services Status only if the main menu is still focused (given the check is delayed by SERVICES_STATUS_CHECK_DELAY)
	protected void CheckServicesStatusIfFocused()
	{
		if (!IsFocused())
			return;

		SCR_ServicesStatusDialogUI.OpenIfServicesAreNotOK();
	}
}
