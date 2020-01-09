#ifndef _RELIFE_H
#define _RELIFE_H
#pragma once

struct AllSkillRange {
	DWORD dwFirst;
	DWORD dwEnd;
};

class CRelife {
	DWORD dwmapnum;
	BOOL relife_mode;
	DWORD relife_timeup;
  DWORD relife_time_count;
public:
  CRelife();
  ~CRelife();
  void setmapnum( WORD anum );
  void LoadSetup();
	BOOL isRelifeMod();
	void reset();
  BOOL isAllowSkill( DWORD skillnum );
private:
	std::vector<DWORD> Allowed_skill;
	std::vector<AllSkillRange> Allowed_skill_range;
};

 

#endif