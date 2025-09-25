/*!
\addtogroup DamageEffects
\{
*/

class SCR_PersistentDamageEffect : PersistentDamageEffect
{
	//------------------------------------------------------------------------------------------------
	//ALWAYS OVERRIDE LIKE THIS. ALWAYS OVERRIDE THIS FUNCTION
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);

		evaluator.HandleEffectConsequences(this, dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	protected event override void OnDiag(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		super.OnDiag(dmgManager);

		if (IsProxy())
			return;
		
		if(!IsActive())
		{
			string text = text.Format("  Activeness: Paused");
			DbgUI.Text(text);
		}
		

	}

}

/*!
\}
*/
