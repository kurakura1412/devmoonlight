
#include <vector>
#include "MHFile.h"
#include "..\[CC]Header\GameResourceManager.h"
#include "Relife.h"

CRelife::CRelife()
{
	dwmapnum = 0;
  relife_time_count = 0;
	relife_mode = TRUE; // default yes
}
CRelife::~CRelife()
{}

void CRelife::LoadSetup()
{
	char tempBuf[256] = {0};
	DWORD mapnumme = 0;
	DWORD alloskillA = 0;
	DWORD alloskillF = 0;
	DWORD alloskillE = 0;
	CMHFile file;
	file.Init("System/Resource/Relife.txt", "r");
	if(file.IsInited() == FALSE)
	{
		//MessageBox(0, "System/Resource/Relife.txt is not found!", 0, MB_OK);
		return;
	}
	Allowed_skill.reserve(200);
	Allowed_skill_range.reserve(200);
	while(1)
	{
		file.GetString(tempBuf);
		if(file.IsEOF())
			break;
		if(tempBuf[0] == '@')
		{
			file.GetLine(tempBuf, 256);
			continue;
		}
		else if(0==strcmp(string, "#RELIFE_TIME"))
		{
			relife_timeup = file.GetDword();
		}
		else if(0==strcmp(string, "#EXCLUDE_MAP"))
		{
			mapnumme = file.GetDword();
			if( mapnumme == dwmapnum ){
				relife_mode = FALSE;
				break;
			}
		}
		else if(0==strcmp(string, "#ALLOWED_SKILL_A"))
		{
			alloskillA = file.GetDword();
			Allowed_skill.push_back( alloskillA );
		}
		else if(0==strcmp(string, "#ALLOWED_SKILL_RANGE"))
		{
			alloskillF = file.GetDword();
			alloskillE = file.GetDword();
			AllSkillRange stuff;
			stuff.dwFirst = alloskillF;
			stuff.dwEnd = alloskillE;
			Allowed_skill_range.push_back(stuff);
		}
	}
	file.Release();
}

BOOL CRelife::isRelifeMod()
{
	return relife_mode;
}

void CRelife::reset()
{
	relife_mode = TRUE;
	Allowed_skill.clear();
	Allowed_skill_range.clear();
}

BOOL CRelife::isAllowSkill(DWORD skillnum)
{
  BOOL ret = FALSE;
  DWORD ii, iii;
  AllSkillRange morestuff;
  DWORD ui = Allowed_skill.size();
  DWORD ai = Allowed_skill_range.size();
  for( ii = 0; ii < ui; ii++ )  {
    if( Allowed_skill[ii] == skillnum ){
      ret = TRUE;
      break;
    }
  }
  if( ret == FALSE ){
    for( iii = 0; iii < ai; iii++){
      if( skillnum >= Allowed_skill_range[iii].dwFirst &&
          skillnum <= Allowed_skill_range[iii].dwEnd ){
        ret = TRUE;
        break;
      }
    }
  }
  return ret;
}

void CRelife::setmapnum( WORD anum )
{
	dwmapnum = (DWORD)anum;
}
