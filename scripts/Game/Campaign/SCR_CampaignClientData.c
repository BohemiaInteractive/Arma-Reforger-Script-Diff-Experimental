//------------------------------------------------------------------------------------------------
//! Used for storing client data to be reapplied for reconnecting clients
class SCR_CampaignClientData
{
	protected string m_sID;
	protected float m_fLastSuicideTimestamp;
	protected float m_fRespawnPenalty;
	protected float m_fLastPenaltyDeductionTimestamp;
	
	//------------------------------------------------------------------------------------------------
	//! Setter for client ID
	void SetID(string ID)
	{
		m_sID = ID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for client ID
	string GetID()
	{
		return m_sID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRespawnPenalty(float penalty)
	{
		m_fRespawnPenalty = Math.Max(0, penalty);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetRespawnPenalty()
	{
		return m_fRespawnPenalty;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLastSuicideTimestamp(float timestamp)
	{
		m_fLastSuicideTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetLastSuicideTimestamp()
	{
		return m_fLastSuicideTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLastPenaltyDeductionTimestamp(float timestamp)
	{
		m_fLastPenaltyDeductionTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetLastPenaltyDeductionTimestamp()
	{
		return m_fLastPenaltyDeductionTimestamp;
	}
}
