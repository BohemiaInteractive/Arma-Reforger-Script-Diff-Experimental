/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence_Config
\{
*/

class PersistenceConfig: ScriptAndConfig
{
	//! What collection this will be put in
	PersistenceCollection Collection;
	//! Chose on which save types this configuration will activate
	ESaveGameType m_eSaveMask;
	//! Delete the persistent record when the instance is destructed in playmode.
	bool m_bSelfDelete;
	//! Only constructed through the persistence system
	private void PersistenceConfig();

}

/*!
\}
*/
