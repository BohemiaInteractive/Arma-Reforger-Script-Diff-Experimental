/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Map
\{
*/

class MapDescriptorComponentClass: GameComponentClass
{
}

class MapDescriptorComponent: GameComponent
{
	/*!
	\brief Determine type of descriptor
	*/
	proto external int GetBaseType();
	/*!
	\brief Determine unit type of descriptor - may not be set if not unit
	*/
	proto external int GetUnitType();
	/*!
	\brief Determine group type of descriptor
	*/
	proto external int GetGroupType();
	/*!
	\brief raist todo: this is temporary - before we fix script
	*/
	proto external MapItem Item();
}

/*!
\}
*/
