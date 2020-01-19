#include "StdAfx.h"
#include "SkillObject.h"
#include "Battle.h"
#include "UserTable.h"

// 090205 LUJ, ±¤¿ª ½ºÅ³ Çã¿ë ¿ÀÂ÷ÀÇ ±âº»°ª
float cSkillObject::mAllowRangeForWideSkill = 300.0f;

cSkillObject::cSkillObject(const SKILL_INFO& info) :
mSkillIndex(info.Index),
mSkillLevel(info.Level),
mSkillKind(info.Kind)
{
	// 080602 LUJ, ¿ÀºêÁ§Æ® Å¸ÀÔÀ» ÃÊ±âÈ­ÇÑ´Ù
	mType = cSkillObject::TypeNone;
// --- skr 16012020
  RelifeAllow = FALSE;
}

cSkillObject::~cSkillObject(void)
{}

CObject* cSkillObject::GetOperator()
{
	return g_pUserTable->FindUser( mSkillObjectInfo.operatorId );
//	return mSkillObjectInfo.pOperator;
}

void cSkillObject::Init( sSKILL_CREATE_INFO* pInfo )
{
	/// ½ºÅ³ »ý¼º Á¤º¸ ¼³Á¤
	memcpy(	&mSkillObjectInfo, pInfo, sizeof( sSKILL_CREATE_INFO ) );

	/// ±âº» Á¤º¸ ¼ÂÆÃ
	m_BaseObjectInfo.dwObjectID = pInfo->skillObjectId;
	strcpy(m_BaseObjectInfo.ObjectName,"SkillObject");
	m_BaseObjectInfo.ObjectState = eObjectState_None;
	m_SkillDir = pInfo->skillDir;

	/// »ç¿ëÀÚ Á¤º¸°¡ Àß¸øµÇ¾úÀ» °æ¿ì ½ÇÆÐ
	if( g_pUserTable->FindUser( pInfo->operatorId ) == NULL )
	{
		return;
	}
}

void cSkillObject::EndState()
{
	mState = SKILL_STATE_DESTROY;
	GetOperator()->CurCastingSkillID = 0;
}

DWORD cSkillObject::SetRemoveMsg(DWORD dwReceiverID, MSGBASE*& sendMessage)
{
	static MSG_DWORD message;
	ZeroMemory(&message, sizeof(message));
	message.Category = MP_SKILL;
	message.Protocol = MP_SKILL_SKILLOBJECT_REMOVE;
	message.dwObjectID = dwReceiverID;
	message.dwData = GetID();

	sendMessage = &message;
	return sizeof(message);
}