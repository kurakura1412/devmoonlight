#include "stdafx.h"
#include ".\attackskillunit.h"

#include "../Target/Target.h"
#include "../Object/ActiveSkillObject.h"
#include "../Info/ActiveSkillInfo.h"

#include "AttackManager.h"
#include "PackedData.h"
// 080616 LUJ, �̺�Ʈ �߻� �ÿ� ��ų�� �׼��� ���ϱ� ���� ����
#include "Event.h"

// --- skr 20012020
#include "[Server]Map/Player.h"

cAttackSkillUnit::cAttackSkillUnit(cActiveSkillObject& skillObject) :
cSkillUnit(skillObject)
{}

cAttackSkillUnit::~cAttackSkillUnit(void)
{
}

BOOL cAttackSkillUnit::Excute()
{
	MSG_SKILL_RESULT msg;

	msg.InitMsg( mpParentSkill->GetID() );
	msg.SkillDamageKind = false;

	CAttackManager::AttackFunction attackFunction = 0;

	switch(mUnitType)
	{
	case UNITKIND_PHYSIC_ATTCK:
		{
			attackFunction = &CAttackManager::PhysicAttack;
			break;
		}
	case UNITKIND_MAGIC_ATTCK:
		{
			attackFunction = &CAttackManager::MagicAttack;
			break;
		}
	default:
		{
			return FALSE;
		}
	}

	CObject* const operatorObject = mpParentSkill->GetOperator();

	if(0 == operatorObject)
	{
		return FALSE;
	}

	BOOL isSucceed = FALSE;
	CTargetListIterator iter(&msg.TargetList);
	mpParentSkill->GetTarget().SetPositionHead();

	while(CObject* const pTarget = mpParentSkill->GetTarget().GetData())
	{
		if( pTarget->GetState() == eObjectState_Die )
		{
			continue;
		}
		else if(operatorObject == pTarget)
		{
			continue;
		}

// --- skr 20012020
    if( pTarget->GetObjectKind() == eObjectKind_Player ){
      CPlayer * pplayer = (CPlayer *)pTarget;
      if( pplayer->IsRelifeON() ){
        continue;
      }
    }

		RESULTINFO damage = { 0 };
		damage.mSkillIndex = mpParentSkill->GetSkillIdx();

		(ATTACKMGR->*attackFunction)(
			operatorObject,
			pTarget,
			&damage,
			mAccuracy,
			mAddDamage,
			mAddType);

		operatorObject->Execute( CGiveDamageEvent( pTarget, damage ) );
		pTarget->Execute( CTakeDamageEvent( operatorObject, damage ) );

		ApplyDamageResult( operatorObject, pTarget, damage );

		iter.AddTargetWithResultInfo( pTarget->GetID(), 1, &damage);
		iter.Release();

		PACKEDDATA_OBJ->QuickSend( pTarget, &msg, msg.GetMsgLength() );

		isSucceed = (0 < damage.ManaDamage || 0 < damage.RealDamage);
	}

	return isSucceed;
}

// 100219 ShinJS --- Damage (Life/Mana) �� �����Ѵ�.
BOOL cAttackSkillUnit::ApplyDamageResult( CObject* pAttacker, CObject* pTarget, RESULTINFO& damageInfo )
{
	// Mana Damage
	pTarget->ManaDamage( pAttacker, &damageInfo );

	DWORD newLife = pTarget->Damage( pAttacker, &damageInfo );
	if(newLife == 0)
	{
		// 080616 LUJ, ��� �� �̺�Ʈ�� ó���ϵ��� �Ѵ�
		// 080708 LUJ, ������/�����ڸ� ��� ���ڷ� �ѱ��
		pTarget->Execute( CDieEvent( pAttacker, pTarget ) );
		pAttacker->Execute( CKillEvent( pAttacker, pTarget ) );

		ATTACKMGR->sendDieMsg( pAttacker, pTarget );
		pTarget->Die( pAttacker );
	}

	return TRUE;
}