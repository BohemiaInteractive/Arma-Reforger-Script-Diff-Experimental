[EntityEditorProps(category: "GameScripted", description: "Handles the character's rank.", color: "0 0 255 255")]
class SCR_CharacterRankComponentClass: ScriptComponentClass
{
}

class SCR_CharacterRankComponent : ScriptComponent
{	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Rank", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_iRank;
	
	protected IEntity m_Owner;
	static ref ScriptInvoker s_OnRankChanged = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] newRank
	//! \param[in] prevRank
	//! \param[in] silent
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDoSetCharacterRank(SCR_ECharacterRank newRank, SCR_ECharacterRank prevRank, bool silent)
	{
		SCR_ECharacterRank oldRank = m_iRank;
		m_iRank = newRank;
		OnRankChanged(oldRank, newRank, silent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] unit
	//! \return
	static SCR_CharacterRankComponent GetCharacterRankComponent(IEntity unit)
	{
		return SCR_CharacterRankComponent.Cast(unit.FindComponent(SCR_CharacterRankComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] rank
	//! \param[in] silent
	void SetCharacterRank(SCR_ECharacterRank rank, bool silent = false)
	{
		if (rank != m_iRank)
		{
			Rpc(RpcDoSetCharacterRank, rank, m_iRank, silent);
			RpcDoSetCharacterRank(rank, m_iRank, silent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRankChanged(SCR_ECharacterRank prevRank, SCR_ECharacterRank newRank, bool silent)
	{
		s_OnRankChanged.Invoke(prevRank, newRank, m_Owner, silent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Helper method to easily read a character's rank by providing just the character parameter
	//! \param[in] unit
	//! \return
	static SCR_ECharacterRank GetCharacterRank(IEntity unit)
	{
		if (!unit)
			return SCR_ECharacterRank.INVALID;
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return SCR_ECharacterRank.INVALID;
		
		return comp.GetCharacterRank();
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_Faction GetCharacterFaction(IEntity unit)
	{
		if (!unit)
			return null;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(unit);
		if (!character)
			return null;
		
		Faction faction = character.GetFaction();
		if (!faction)
			return null;

		return SCR_Faction.Cast(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return name of passed rank.
	//! \param[in] unit
	//! \param[in] rank
	//! \return
	static string GetRankName(IEntity unit, SCR_ECharacterRank rank)
	{
		if (!unit)
			return string.Empty;
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		if (!comp)
			return string.Empty;

		SCR_Faction faction = comp.GetCharacterFaction(unit);
		if (!faction)
			return string.Empty;
		
		return faction.GetRanks().GetRankName(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] unit
	//! \return
	static string GetCharacterRankName(IEntity unit)
	{
		if (!unit)
			return "";
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return "";
		
		SCR_ECharacterRank rank = comp.GetCharacterRank();
		SCR_Faction faction = comp.GetCharacterFaction(unit);
		
		if (!faction)
			return "";
		
		return faction.GetRanks().GetRankName(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] unit
	//! \return
	static string GetCharacterRankNameUpperCase(IEntity unit)
	{
		if (!unit)
			return "";
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return "";
		
		SCR_ECharacterRank rank = comp.GetCharacterRank();
		SCR_Faction faction = comp.GetCharacterFaction(unit);
		
		if (!faction)
			return "";
		
		return faction.GetRanks().GetRankNameUpperCase(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] unit
	//! \return
	static string GetCharacterRankNameShort(IEntity unit)
	{
		if (!unit)
			return "";
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return "";
		
		SCR_ECharacterRank rank = comp.GetCharacterRank();
		SCR_Faction faction = comp.GetCharacterFaction(unit);
		
		if (!faction)
			return "";
		
		return faction.GetRanks().GetRankNameShort(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] unit
	//! \return
	static ResourceName GetCharacterRankInsignia(IEntity unit)
	{
		if (!unit)
			return "";
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return "";
		
		SCR_ECharacterRank rank = comp.GetCharacterRank();
		SCR_Faction faction = comp.GetCharacterFaction(unit);
		
		if (!faction)
			return "";

		return faction.GetRanks().GetRankInsignia(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ECharacterRank GetCharacterRank()
	{
		return m_iRank;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteIntRange(m_iRank, 0, SCR_ECharacterRank.INVALID-1);
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadIntRange(m_iRank, 0, SCR_ECharacterRank.INVALID-1);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!ChimeraCharacter.Cast(owner))
			Print("SCR_CharacterRankComponent must be attached to ChimeraCharacter!", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_CharacterRankComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Owner = ent;
	}
}