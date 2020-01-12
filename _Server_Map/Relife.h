#ifndef _RELIFE_H
#define _RELIFE_H
#pragma once

#define RELIFEEMGR CRelife::GetInstance()

struct AllSkillRange {
	DWORD dwFirst;
	DWORD dwEnd;
};

class CRelife {
	DWORD dwmapnum;
	BOOL relife_mode;
  BOOL relife_icon;
	DWORD relife_timeup;
  DWORD relife_time_count;
public:
  CRelife();
  ~CRelife();
  void Init();
  static CRelife* GetInstance();
  void setmapnum( WORD anum );
  void LoadSetup();
	BOOL isRelifeMod();
  BOOL isRelifeIcon();
	void reset();
  BOOL isAllowSkill( DWORD skillnum );
private:
	std::vector<DWORD> Allowed_skill;
	std::vector<AllSkillRange> Allowed_skill_range;
};

 

#endif