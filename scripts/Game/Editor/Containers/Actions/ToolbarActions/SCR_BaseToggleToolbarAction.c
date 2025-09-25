//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_BaseToggleToolbarAction : SCR_EditorToolbarAction
{
	[Attribute(desc: "When defined, this info will be used when the action is toggled on.")]
	protected ref SCR_UIInfo m_InfoToggled;
	
	private ref ScriptInvoker m_OnToggleChanged = new ScriptInvoker();
	
	private int currentlyToggled = -1;
	private bool old_highlight = false;
	
	/*!
	Get ScriptInvoker called when the action is toggled by some external event.
	*/
	ScriptInvoker GetOnToggleChange()
	{
		return m_OnToggleChanged;
	}
	
	/*!
	Toggle action state.
	To be called by inherited classes.
	*/
	protected void Toggle(int value, bool highlight = false)
	{
		if(value == currentlyToggled && highlight == old_highlight)
			return;
		
		currentlyToggled = value;
		old_highlight = highlight;
		m_OnToggleChanged.Invoke(value, highlight);
	}
	
	/*!
	Track events which influence the action.
	Called when the action's GUI representation is created.
	To be overloaded by inherited classes.
	*/
	void Track()
	{
	}
	
	/*!
	Untrack events which influence the action.
	Called when the action's GUI representation is destroyed.
	To be overloaded by inherited classes.
	*/
	void Untrack()
	{
	}
	
	/*
	Get UI info representing toggled state of the action.
	When not defined, default UI info will be returned.
	\return UI info
	*/
	SCR_UIInfo GetInfoToggled()
	{
		if (m_InfoToggled)
			return m_InfoToggled;
		else
			return GetInfo();
	}
};