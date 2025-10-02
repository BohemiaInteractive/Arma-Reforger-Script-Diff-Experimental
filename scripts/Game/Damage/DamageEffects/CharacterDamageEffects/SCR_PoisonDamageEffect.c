class SCR_PoisonDamageEffect : SCR_DotDamageEffect
{
	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);

		evaluator.HandleEffectConsequences(this, dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	override bool HijackDamageEffect(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		// same as with bleeding, this should prevent us from regenerating
		dmgManager.TerminateDamageEffectsOfType(SCR_PhysicalHitZonesRegenDamageEffect);

		array<ref SCR_PersistentDamageEffect> damageEffects = {};
		dmgManager.FindAllDamageEffectsOfTypeOnHitZone(SCR_PoisonDamageEffect, GetAffectedHitZone(), damageEffects);

		SCR_PoisonDamageEffect poisonEffect;
		IEntity instigatorEnt = GetInstigator().GetInstigatorEntity();

		int otherMaxDuration;
		const int thisMaxDuration = GetMaxDuration();
		foreach (SCR_PersistentDamageEffect dmgEffect : damageEffects)
		{
			poisonEffect = SCR_PoisonDamageEffect.Cast(dmgEffect);
			if (!poisonEffect || poisonEffect == this)
				continue;

			if (poisonEffect.GetInstigator().GetInstigatorEntity() != instigatorEnt)
				continue;

			// combine same effects from the same entity
			poisonEffect.SetDPS(poisonEffect.GetDPS() + GetDPS());

			// is that effect supposed to end at some point?
			otherMaxDuration = poisonEffect.GetMaxDuration();
			if (otherMaxDuration > 0)
			{
				if (thisMaxDuration > 0)
					poisonEffect.SetMaxDuration(otherMaxDuration + thisMaxDuration); // if both of us are meant to expire, then lets combine that time
				else
					poisonEffect.SetMaxDuration(0); // if this new effect is not supposed to expire, then we make that old one not expire

				poisonEffect.ApplyEffect(dmgManager);
			}

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager)
	{
		timeSlice = GetAccurateTimeSlice(timeSlice);
		SCR_CharacterDamageManagerComponent characterDmgManager = SCR_CharacterDamageManagerComponent.Cast(dmgManager);

		int decayFactor = -1;
		if (GetMaxDuration() == 0)
			decayFactor = 1;//if it shouldnt expire then we are constantly exposed to the source of the poison

		float damageAmount = GetDPS() * (1 + decayFactor * characterDmgManager.GetPoisonBuildupFactor() * timeSlice);
		SetDPS(damageAmount);//poison buildup

		damageAmount *= timeSlice;

		DotDamageEffectTimerToken token = UpdateTimer(timeSlice, dmgManager);
		DealCustomDot(GetAffectedHitZone(), damageAmount, token, dmgManager);
	}
}
