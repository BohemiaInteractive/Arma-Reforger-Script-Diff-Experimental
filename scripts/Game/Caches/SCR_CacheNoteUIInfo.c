[BaseContainerProps()]
class SCR_CacheNoteUIInfo : SCR_InventoryUIInfo
{
	//------------------------------------------------------------------------------------------------
	override string GetInventoryItemDescription(InventoryItemComponent item)
	{
		IEntity owner = item.GetOwner();
		SCR_CacheNoteComponent noteComp = SCR_CacheNoteComponent.Cast(owner.FindComponent(SCR_CacheNoteComponent));
		if (!noteComp)
			return string.Empty;

		array<string> lines = noteComp.GetNoteLines();
		return SCR_StringHelper.Join(SCR_StringHelper.LINE_RETURN, lines);
	}
}
