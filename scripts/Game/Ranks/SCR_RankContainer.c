[BaseContainerProps(configRoot: true)]
class SCR_RankContainer
{
	[Attribute(desc : "List of ranks")]
	protected ref array<ref SCR_RankInfo> m_aRanks;

	//------------------------------------------------------------------------------------------------
	//! \param[in] rankID
	//! \return
	bool IsRankRenegade(SCR_ECharacterRank rankID)
	{
		if (!m_aRanks)
			return false;

		SCR_RankInfo foundRank = GetRankByID(rankID);
		if (!foundRank)
			return false;

		return foundRank.IsRankRenegade();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] rankID
	//! \return
	int GetRequiredRankXP(SCR_ECharacterRank rankID)
	{
		if (!m_aRanks)
			return int.MAX;

		SCR_RankInfo foundRank = GetRankByID(rankID);
		if (!foundRank)
			return int.MAX;

		return foundRank.GetRequiredRankXP();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ECharacterRank GetRenegadeRank()
	{
		foreach (SCR_RankInfo rank : m_aRanks)
		{
			if (rank && rank.IsRankRenegade())
				return rank.GetRankID();
		}

		return SCR_ECharacterRank.INVALID;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] XP
	//! \return
	SCR_ECharacterRank GetRankByXP(int XP)
	{
		if (!m_aRanks)
			return SCR_ECharacterRank.INVALID;

		int maxFoundXP = -100000;
		SCR_ECharacterRank rankFound = GetRenegadeRank();

		foreach (SCR_RankInfo rank : m_aRanks)
		{
			int reqXP = GetRequiredRankXP(rank.GetRankID());

			if (reqXP <= XP && reqXP > maxFoundXP)
			{
				maxFoundXP = reqXP;
				rankFound = rank.GetRankID();
			}
		}

		return rankFound;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] rank
	//! \return the next higher rank
	SCR_ECharacterRank GetRankNext(SCR_ECharacterRank rankID)
	{
		int rankXP = GetRequiredRankXP(rankID);
		int higherXP = int.MAX;
		SCR_ECharacterRank foundID = SCR_ECharacterRank.INVALID;

		if (!m_aRanks)
			return SCR_ECharacterRank.INVALID;

		foreach (SCR_RankInfo checkedRank : m_aRanks)
		{
			SCR_ECharacterRank ID = checkedRank.GetRankID();
			int thisXP = GetRequiredRankXP(ID);

			if (thisXP > rankXP && thisXP < higherXP)
			{
				higherXP = thisXP;
				foundID = ID;
			}
		}

		return foundID;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] rank
	//! \return the next lower rank
	SCR_ECharacterRank GetRankPrev(SCR_ECharacterRank rankID)
	{
		int rankXP = GetRequiredRankXP(rankID);
		int lowerXP = int.MIN;
		SCR_ECharacterRank foundID = SCR_ECharacterRank.INVALID;

		if (!m_aRanks)
			return SCR_ECharacterRank.INVALID;

		foreach (SCR_RankInfo checkedRank : m_aRanks)
		{
			SCR_ECharacterRank ID = checkedRank.GetRankID();
			int thisXP = GetRequiredRankXP(ID);

			if (thisXP < rankXP && thisXP > lowerXP)
			{
				lowerXP = thisXP;
				foundID = ID;
			}
		}

		return foundID;
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_RankInfo> GetAllRanks()
	{
		array<ref SCR_RankInfo> outArray = {};
		foreach (SCR_RankInfo rank : m_aRanks)
		{
			outArray.Insert(rank);
		}

		return outArray;
	}

	//------------------------------------------------------------------------------------------------
	SCR_RankInfo GetRankByID(SCR_ECharacterRank rankIdentifier)
	{
		foreach (SCR_RankInfo rank : m_aRanks)
		{
			if (rank.GetRankID() == rankIdentifier)
				return rank;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param rankID
	//! \return
	string GetRankName(SCR_ECharacterRank rankID)
	{
		SCR_RankInfo rank = GetRankByID(rankID);

		if (rank)
			return rank.GetRankName();
		else
			return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \param rankID
	//! \return
	string GetRankNameUpperCase(SCR_ECharacterRank rankID)
	{
		SCR_RankInfo rank = GetRankByID(rankID);

		if (rank)
			return rank.GetRankNameUpperCase();
		else
			return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \param rankID
	//! \return
	string GetRankNameShort(SCR_ECharacterRank rankID)
	{
		SCR_RankInfo rank = GetRankByID(rankID);

		if (rank)
			return rank.GetRankNameShort();
		else
			return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \param rankID
	//! \return
	string GetRankInsignia(SCR_ECharacterRank rankID)
	{
		SCR_RankInfo rank = GetRankByID(rankID);

		if (rank)
			return rank.GetRankInsignia();
		else
			return string.Empty;
	}
}
