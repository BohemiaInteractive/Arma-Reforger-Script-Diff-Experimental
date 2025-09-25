/*!
This component is meant for cases when content (expecially text) is too long horisontally and doesn't fit into UI.
The component animates content left-right when activated.

!!! WARNING !!!
The content widget (the one which will be animated) must be in a Frame Slot!
In most basic case you should create a Frame, attach the component to it, and and put a text widget inside the Frame.
NB: this might cause the font to be cut on top and bottom if set to clip, in which case wrap the text into another widget and give it some padding
Frame
	Wrapper
		Text (with top & bottom padding)
*/

enum EScrollAnimationState
{
	NONE,
	MOVING_LEFT,
	MOVING_LEFT_DONE_WAITING,
	MOVING_RIGHT,
	MOVING_RIGHT_DONE_WAITING
}

class SCR_HorizontalScrollAnimationComponent : ScriptedWidgetComponent
{
	[Attribute("Content", UIWidgets.EditBox, "Name of widget which will be animated")]
	string m_sAnimationContentWidget;

	[Attribute("10", UIWidgets.EditBox, "Animation speed")]
	protected float m_fAnimationSpeedPxPerSec;

	[Attribute("0.5", UIWidgets.EditBox, "Wait time while animating when content is on the left")]
	protected float m_fWaitTimeLeftSec;

	[Attribute("0.5", UIWidgets.EditBox, "Wait time while animating when content is on the right")]
	protected float m_fWaitTimeRightSec;

	[Attribute("false", UIWidgets.CheckBox, "When true, content will animate to start after it has animated left. Otherwise it will jump to start instantly.")]
	protected bool m_bAnimateBack;

	[Attribute("0", UIWidgets.CheckBox, "Inwards offset from right edge of root widget to be used in calculations")]
	protected float m_fOffsetRight;

	[Attribute("0", UIWidgets.CheckBox, "Inwards offset from left edge of root widget to be used in calculations")]
	protected float m_fOffsetLeft;

	[Attribute(typename.EnumToString(EScrollAnimationState, EScrollAnimationState.NONE), UIWidgets.ComboBox, "Start state of animation", "", ParamEnumArray.FromEnum(EScrollAnimationState))]
	protected EScrollAnimationState m_eInitialState;

	protected EScrollAnimationState m_State;
	protected Widget m_wContent;
	protected Widget m_wRoot;
	protected float m_fStartPosX;
	protected float m_fTimer;

	//------------------------------------------------------------------------------------------------
	// P U B L I C

	//------------------------------------------------------------------------------------------------
	void AnimationStop()
	{
		m_State = EScrollAnimationState.NONE;
	}

	//------------------------------------------------------------------------------------------------
	void AnimationStart()
	{
		// If not animating, start animation
		if (m_State == EScrollAnimationState.NONE)
			m_State = EScrollAnimationState.MOVING_LEFT;

		// Otherwise it's already animating in some state, do nothing
	}

	//------------------------------------------------------------------------------------------------
	//! Sets X position to its initial value when component was initialized
	void ResetPosition()
	{
		if (!m_wContent)
			return;

		FrameSlot.SetPosX(m_wContent, m_fStartPosX);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true when content fits the parent widget
	bool GetContentFit()
	{
		if (!m_wContent)
			return true;

		float rootw, rooth;
		m_wRoot.GetScreenSize(rootw, rooth);

		float contentw, contenth;
		m_wContent.GetScreenSize(contentw, contenth);

		WorkspaceWidget workspace = GetGame().GetWorkspace();

		return rootw > contentw + workspace.DPIScale(m_fOffsetLeft + m_fOffsetRight);
	}

	//------------------------------------------------------------------------------------------------
	// P R O T E C T E D

	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		if (!GetGame().InPlayMode())
			return;

		m_wRoot = w;
		m_wContent = w.FindAnyWidget(m_sAnimationContentWidget);

		if (!m_wContent)
		{
			return;
			Print(string.Format("[SCR_HorizontalScrollAnimationComponent] Widget %1 was not found", m_sAnimationContentWidget), LogLevel.ERROR);
		}

		if (m_wContent)
		{
			m_fStartPosX = FrameSlot.GetPosX(m_wContent);
		}

		m_State = m_eInitialState;
		//use CallLater, it seems that Call() might not have a loop/repeat option
		GetGame().GetCallqueue().CallLater(OnFrame, 0, true);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		ArmaReforgerScripted game = GetGame();
		if (game && game.GetCallqueue())
			game.GetCallqueue().Remove(OnFrame);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFrame()
	{
		float tDelta = ftime * 0.001;

		switch (m_State)
		{
			case EScrollAnimationState.NONE:
				// Doing nothing
				break;

			case EScrollAnimationState.MOVING_LEFT:
			{
				// Moving left
				AnimatePosition(tDelta, -m_fAnimationSpeedPxPerSec);

				WorkspaceWidget workspace = GetGame().GetWorkspace();
				float rootw, rooth, rootx, rooty;
				float contentw, contenth, contentx, contenty;
				m_wRoot.GetScreenSize(rootw, rooth);
				m_wRoot.GetScreenPos(rootx, rooty);
				m_wContent.GetScreenSize(contentw, contenth);
				m_wContent.GetScreenPos(contentx, contenty);

				if (contentx + contentw < rootx + rootw - workspace.DPIScale(m_fOffsetRight))
				{
					m_State = EScrollAnimationState.MOVING_LEFT_DONE_WAITING;
					m_fTimer = m_fWaitTimeLeftSec;
				}

				break;
			}

			case EScrollAnimationState.MOVING_LEFT_DONE_WAITING:
			{
				m_fTimer -= tDelta;
				if (m_fTimer < 0)
				{
					if (m_bAnimateBack)
					{
						m_State = EScrollAnimationState.MOVING_RIGHT;
					}
					else
					{
						ResetPosition();
						m_fTimer = m_fWaitTimeRightSec;
						m_State = EScrollAnimationState.MOVING_RIGHT_DONE_WAITING;
					}
				}
				break;
			}

			case EScrollAnimationState.MOVING_RIGHT:
			{
				// Moving right
				AnimatePosition(tDelta, m_fAnimationSpeedPxPerSec);

				WorkspaceWidget workspace = GetGame().GetWorkspace();
				float rootx, rooty;
				float contentx, contenty;
				m_wRoot.GetScreenPos(rootx, rooty);
				m_wContent.GetScreenPos(contentx, contenty);

				if (contentx > rootx + workspace.DPIScale(m_fOffsetLeft))
				{
					m_State = EScrollAnimationState.MOVING_RIGHT_DONE_WAITING;
					m_fTimer = m_fWaitTimeRightSec;
				}

				break;
			}

			case EScrollAnimationState.MOVING_RIGHT_DONE_WAITING:
			{
				m_fTimer -= tDelta;
				if (m_fTimer < 0)
				{
					m_State = EScrollAnimationState.MOVING_LEFT;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AnimatePosition(float tDelta, float speed)
	{
		float currentPos = FrameSlot.GetPosX(m_wContent);
		float newPos = currentPos + tDelta * speed;
		FrameSlot.SetPosX(m_wContent, newPos);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_HorizontalScrollAnimationComponent FindComponent(Widget w)
	{
		return SCR_HorizontalScrollAnimationComponent.Cast(w.FindHandler(SCR_HorizontalScrollAnimationComponent));
	}
}
