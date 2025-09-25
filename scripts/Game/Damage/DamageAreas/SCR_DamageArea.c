//------------------------------------------------------------------------------------------------
class SCR_DamageArea : DamageArea
{
	
	/*!
	Callback when an entity enters this DamageArea this frame
	*/
	override void OnAreaEntered(notnull IEntity entity)
	{
	}
	
	/*!
	Callback when an entity was already inside of the area and it is still in the area this frame
	*/
	override void OnAreaPersisted(notnull IEntity entity)
	{
	}
	
	/*!
	Callback when an entity exits this DamageArea this frame
	*/
	override void OnAreaExit(IEntity entity)
	{
	}
	
	override void DisplayInfo()
	{
		DbgUI.Text("Area information");
	}
}