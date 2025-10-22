/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence
\{
*/

sealed enum EPersistenceParentHandling
{
	//! No data if any parent is present, it must serialize us as child
	ACCEPT,
	//! Ignore parent if it is a loaded world entity (even if it is tracked)
	IGNORE_LOADED,
	//! Ignore parent if it is not tracked by persistence
	IGNORE_UNTRACKED,
	//! Ignore any parent and always create root record
	IGNORE,
}

/*!
\}
*/
