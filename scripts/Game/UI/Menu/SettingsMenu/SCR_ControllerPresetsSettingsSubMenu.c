class SCR_ControllerPresetsSettingsSubmenu: SCR_SettingsSubMenuBase
{
	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();

		SCR_HeadTrackingSettings.SetHeadTrackingSettings();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		EPlatform platform = System.GetPlatform();
		if (platform == EPlatform.PS4 || platform == EPlatform.PS5 || platform == EPlatform.PS5_PRO || platform == EPlatform.UNKNOWN)
		{
			HideMenuItem("Joystick0");
			HideMenuItem("Joystick1");
		}
		
		m_aSettingsBindings.Clear();

		// HEAD TRACKING
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_bHeadTrackingFreelook", "HeadTrackingFreelook"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_bHeadTrackingADS", "HeadTrackingADS"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingSensitivity", "HeadTrackingSensitivity"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingADSDeadzone", "HeadTrackingADSDeadzone"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_eHeadTrackingLean", "HeadTrackingLean"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingLeanSensitivity", "HeadTrackingLeanSensitivity"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingLeanDeadzone", "HeadTrackingLeanDeadzone"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingLeanYawLimit", "HeadTrackingLeanYawLimit"));

		LoadSettings();

		// PRESETS
		array<ref SCR_ControllerPreset> controllerPresets = {};
		array<ref SCR_ControllerPreset> joystick0Presets = {};
		array<ref SCR_ControllerPreset> joystick1Presets = {};

		SCR_SettingsManagerKeybindModule keybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (keybindModule)
			keybindModule.GetControllerPresets(controllerPresets, joystick0Presets, joystick1Presets);

		SCR_ComboBoxComponent combo;
		combo = BindControllerPresets("Presets", controllerPresets);
		if (combo)
			combo.m_OnChanged.Insert(SelectControllerPreset);

		combo = BindControllerPresets("Joystick0", joystick0Presets);
		if (combo)
			combo.m_OnChanged.Insert(SelectJoystick0Preset);

		combo = BindControllerPresets("Joystick1", joystick1Presets);
		if (combo)
			combo.m_OnChanged.Insert(SelectJoystick1Preset);

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_UI_SHOW_ALL_SETTINGS, false))
			return;

		// Show only if there is TrackIR connected
		if (!GetGame().GetInputManager().IsTrackIRConnected())
		{
			HideMenuItem("TitleHeadTracking");

			HideMenuItem("HeadTrackingFreelook");
			HideMenuItem("HeadTrackingADS");
			HideMenuItem("HeadTrackingSensitivity");
			HideMenuItem("HeadTrackingADSDeadzone");

			HideMenuItem("HeadTrackingLean");
			HideMenuItem("HeadTrackingLeanSensitivity");
			HideMenuItem("HeadTrackingLeanDeadzone");
			HideMenuItem("HeadTrackingLeanYawLimit");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] widgetName
	//! \param[in] presets
	//! \return
	SCR_ComboBoxComponent BindControllerPresets(string widgetName, notnull array<ref SCR_ControllerPreset> presets)
	{
		Widget presetsWidget = m_wRoot.FindAnyWidget(widgetName);
		if (!presetsWidget)
			return null;

		SCR_SettingsManagerKeybindModule keybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (!keybindModule)
			return null;

		SCR_ComboBoxComponent comboComp = SCR_ComboBoxComponent.Cast(presetsWidget.FindHandler(SCR_ComboBoxComponent));
		if (!comboComp)
			return null;

		foreach (SCR_ControllerPreset preset : presets)
		{
			comboComp.AddItem(preset.GetDisplayName());
		}

		comboComp.SetCurrentItem(keybindModule.GetActivePresetIndex(presets));
		return comboComp;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] comp
	//! \param[in] index
	void SelectControllerPreset(SCR_ComboBoxComponent comp, int index)
	{
		SCR_SettingsManagerKeybindModule keybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (keybindModule)
			keybindModule.SelectControllerPresets(presetIndex: index);
		
		SCR_AnalyticsApplication.GetInstance().ChangeSetting("Presets", "Controller Preset");
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] comp
	//! \param[in] index
	void SelectJoystick0Preset(SCR_ComboBoxComponent comp, int index)
	{
		SCR_SettingsManagerKeybindModule keybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (keybindModule)
			keybindModule.SelectControllerPresets(joystick0Index: index);

		SCR_AnalyticsApplication.GetInstance().ChangeSetting("Presets", "Joystick #1 Preset");
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] comp
	//! \param[in] index
	void SelectJoystick1Preset(SCR_ComboBoxComponent comp, int index)
	{
		SCR_SettingsManagerKeybindModule keybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (keybindModule)
			keybindModule.SelectControllerPresets(joystick1Index: index);
		
		SCR_AnalyticsApplication.GetInstance().ChangeSetting("Presets", "Joystick #2 Preset");
	}
}
