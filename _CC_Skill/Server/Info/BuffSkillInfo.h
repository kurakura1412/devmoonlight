#pragma once
#include "SkillInfo.h"

class cBuffSkillInfo : public cSkillInfo
{
	BUFF_SKILL_INFO mInfo;
	// 081203 LUJ, ½ºÅ©¸³Æ®·Î ¼öÄ¡ º¸³Ê½º¸¦ Àû¿ëÇÑ´Ù
	void SetStatus( CObject&, Status& status, const SkillScript::Buff&, SetSkillStatusType ) const;
	// 081203 LUJ, ½ºÅ©¸³Æ®·Î ¹öÇÁ »óÅÂ¸¦ Àû¿ëÇÑ´Ù
	void SetBuff( CObject&, const SkillScript&, SetSkillStatusType ) const;

public:
	cBuffSkillInfo(void);
	virtual ~cBuffSkillInfo(void);
	void Parse(LPCTSTR);
	virtual cSkillObject* GetSkillObject() const;
	const BUFF_SKILL_INFO& GetInfo() const { return mInfo; }
	void AddPassiveStatus( CPlayer* pPlayer ) const;
	void RemovePassiveStatus( CPlayer* pPlayer ) const;
	void AddBuffStatus( CObject* pTarget ) const;
	void RemoveBuffStatus( CObject* pTarget ) const;
	virtual SKILLKIND GetKind() const { return mInfo.Kind; }
	virtual DWORD GetIndex() const { return mInfo.Index; }
	virtual LEVELTYPE GetLevel() const { return mInfo.Level; }
// --- skr 12/01/2020
	void SetDelay( DWORD anum);
};