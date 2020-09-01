#include "stdafx.h"
#include "Player.h"
#include "PartyManager.h"
#include "Party.h"
#include "CharMove.h"
#include "PackedData.h"
#include "MapDBMsgParser.h"
#include "ItemManager.h"
#include "../[cc]skill/server/manager/skillmanager.h"
#include "ObjectStateManager.h"
#include "streetstallmanager.h"
#include "CharacterCalcManager.h"
#include "GuildManager.h"
#include "SkillTreeManager.h"
#include "GridSystem.h"
#include "GridTable.h"
#include "../[cc]skill/Server/info/BuffSkillInfo.h"
#include "../[cc]skill/Server/info/ActiveSkillInfo.h"
#include "Showdown/Battle_Showdown.h"
#include "StateMachinen.h"
#include "MHError.h"
#include "CQuestBase.h"
#include "Quest.h"
#include "TileManager.h"
#include "mhtimemanager.h"
#include "LootingManager.h"
#include "PKManager.h"
#include "QuestManager.h"
#include "StorageManager.h"
#include "UserTable.h"
#include "GuildFieldWarMgr.h"
#include "QuestMapMgr.h"
#include "GuildTournamentMgr.h"
#include "StreetStall.h"
#include "QuickManager.h"
#include "FishingManager.h"
#include "../[CC]BattleSystem/BattleSystem_Server.h"
#include "../[cc]skill/server/tree/SkillTree.h"
#include "../[CC]Skill\Server\Delay\Delay.h"
#include "AttackManager.h"
#include "../hseos/Common/SHMath.h"
#include "../hseos/Monstermeter/SHMonstermeterManager.h"
#include "../hseos/Farm/SHFarmManager.h"
#include "../hseos/Family/SHFamilyManager.h"
#include "../hseos/Date/SHDateManager.h"
#include "../[cc]skill/server/Object\BuffSkillObject.h"
#include "../[cc]skill/server/Object/ActiveSkillObject.h"
#include "Guild.h"
#include "Network.h"
#include "MapNetworkMsgParser.h"
#include "Pet.h"
#include "petmanager.h"
#include "LimitDungeonMgr.h"
#include "./SiegeWarfareMgr.h"
#include "../[CC]SiegeDungeon/SiegeDungeonMgr.h"
#include "Vehicle.h"
#include "VehicleManager.h"
#include "HousingMgr.h"
#include "Trigger\Manager.h"
#include "Trigger\Message.h"
#include "PCRoomManager.h"
#include "..\[CC]Header\GameResourceManager.h"
#include "Dungeon\DungeonMgr.h"
#include "..\hseos\ResidentRegist\SHResidentRegistManager.h"
#include "StreetTournamentMgr.h"
#include "ObjectStateManager.h"

#define INSERTLOG_TIME			600000 // 10 min
#define INSERTLOG_TIME_CHINA	1800000 // 30 min
#define INSERTLOG_TIME_HK		1800000 // 30 min

extern int	g_nHackCheckNum;
extern int	g_nHackCheckWriteNum;

CPlayer::CPlayer() :
m_SkillTree(new cSkillTree)
{
	m_SkillFailCount = 0;
	mpBattleStateDelay = new cDelay;
	mpBattleStateDelay->Init( 10000 );

	m_God = FALSE;

	mIsEmergency = FALSE;

	mTargetMap = 0;
	mTargetPosX = 0;
	mTargetPosZ = 0;

	// 071128 LYW --- Player : HPMP ����.
	m_byHP_Point = 0;
	m_byMP_Point = 0;

	m_bResetSkill	=	false;
	m_bResetStat	=	false;
	//aziz Reborn in Game 29 Sep
	m_bResetLevel	=	false;
	m_ItemArrayList.Initialize(10);
	m_FollowMonsterList.Initialize(5);
	m_QuestList.Initialize(30);
	m_dweFamilyRewardExp = 0;
	m_byCurFamilyMemCnt	 = 0;
	m_dwCurrentResurrectIndex = 0;
	m_registeredstreettournament = FALSE;
	m_STrank = 0;
	m_MovedOnSt = FALSE;
	m_AliasType = 0;
	m_MaxCountSpin = 0;
	m_VerifyCaptcha = 0;
	m_VerifyCount = 0;
	m_VerifyKillCount = 0;
	m_index = 0;
// --- skr : relife
 	RelifeON = FALSE;
	RelifeTimer = 0;
  RelifeStartTime = 0;
  currentwarehouseset = 0;
}

CPlayer::~CPlayer()
{
	delete mpBattleStateDelay;
	SAFE_DELETE(m_SkillTree);
}

void CPlayer::InitClearData()
{
	m_ItemContainer.Init();
	//m_ItemContainer.SetInit(eItemTable_Inventory,	TP_INVENTORY_START,		SLOT_INVENTORY_NUM,		&m_InventorySlot);
	m_ItemContainer.SetInit(eItemTable_Inventory,	TP_INVENTORY_START,		SLOT_MAX_INVENTORY_NUM,		&m_InventorySlot);

	m_ItemContainer.SetInit(eItemTable_Weared,		TP_WEAR_START,			SLOT_WEAR_NUM,			&m_WearSlot);
	m_ItemContainer.SetInit(eItemTable_Storage,		TP_STORAGE_START,		SLOT_STORAGE_NUM,		&m_StorageSlot);
	m_ItemContainer.SetInit(eItemTable_Shop,		TP_SHOPITEM_START,		SLOT_SHOPITEM_NUM,		&m_ShopItemSlot);	
	memset(&m_HeroCharacterInfo,0,sizeof(CHARACTER_TOTALINFO));
	memset(&m_HeroInfo,0,sizeof(HERO_TOTALINFO));
	// 090701 LUJ, �޸�Ǯ�� ������/�Ҹ��ڸ� ȣ������ �ʱ� ������ Init()���� �ʱ�ȭ���� ���� ���, �ʱ�ȭ��
	//		�̷����� �ʴ´�. purse�� �� ���� �ʱ�ȭ�� ���� �ʾ����Ƿ� �÷��̾ ������ �����ص� ����(�����Ⱚ)��
	//		���� �ִ�. �̸� �̿��ؼ� DB���� ���� �������� ���� ������ �� �̵��� �ݺ��� ���, �����Ⱚ�� DB�� �����ϰԵȴ�.
	//		�̸� ���� ���� ��ü �ʱ�ȭ ������ ���� �ʱ�ȭ��Ű���� �Ѵ�
	m_InventoryPurse.SetZeroMoney();
	m_StoragePurse.SetZeroMoney();

	m_QuestGroup.Initialize( this );
	m_wKillMonsterLevel = 0;
	m_bGotWarehouseItems = FALSE;
	m_SkillFailCount = 0;
	mGravity = 0;

	memset( &m_DateMatching, 0, sizeof(DATE_MATCHING_INFO));			// ����Ʈ ��Ī �ֹ���.
	// desc_hseos_�ֹε��01
	// S �ֹε�� �߰� added by hseos 2007.06.09
	m_DateMatching.nSerchTimeTick = gCurTime;
	m_DateMatching.nRequestChatTimeTick = gCurTime;
	// E �ֹε�� �߰� added by hseos 2007.06.09
	memset( &mPassiveStatus, 0, sizeof( Status ) );
	memset( &mBuffStatus, 0, sizeof( Status ) );
	memset( &mRatePassiveStatus, 0, sizeof( Status ) );
	memset( &mRateBuffStatus, 0, sizeof( Status ) );
	memset( &mAbnormalStatus, 0, sizeof( AbnormalStatus ) );

	// desc_hseos_���͹���01
	// S ���͹��� �߰� added by hseos 2007.05.23
	ZeroMemory(&m_stMonstermeterInfo, sizeof(m_stMonstermeterInfo));
	m_stMonstermeterInfo.nPlayTimeTick = gCurTime;
	m_pcFamilyEmblem = NULL;
	ZeroMemory(&m_stFarmInfo, sizeof(m_stFarmInfo));
	m_stFarmInfo.nCropPlantRetryTimeTick  = gCurTime;
	m_stFarmInfo.nCropManureRetryTimeTick = gCurTime;
	// 080519 KTH
	m_stFarmInfo.nAnimalFeedRetryTimeTick = gCurTime;
	m_stFarmInfo.nAnimalCleanRetryTimeTick = gCurTime;
	m_nChallengeZoneEnterFreq = 0;
	m_nChallengeZoneEnterBonusFreq = 0;
	m_nChallengeZonePartnerID = 0;
	m_nChallengeZoneSection = 0;
	m_nChallengeZoneStartState = 0;
	m_nChallengeZoneStartTimeTick = 0;
	m_nChallengeZoneMonsterNum = 0;
	m_nChallengeZoneKillMonsterNum = 0;
	m_bChallengeZoneNeedSaveEnterFreq = FALSE;
	m_bChallengeZoneCreateMonRightNow = FALSE;
	m_nChallengeZoneMonsterNumTillNow = 0;
	m_nChallengeZoneClearTime = 0;
	m_nChallengeZoneExpRate   = 0;

	m_God = FALSE;
	mIsEmergency = FALSE;

	mTargetMap = 0;
	mTargetPosX = 0;
	mTargetPosZ = 0;

	m_bResetSkill	=	false;
	m_bResetStat	=	false;
	//aziz Reborn in Game 29 Sep
	m_bResetLevel	=	false;
	m_dwReturnSkillMoney = 0;

//---KES AUTONOTE
	m_dwAutoNoteIdx = 0;
	m_dwLastActionTime = 0;
//---------------

	FishingInfoClear();
	m_dwFM_MissionCode = 1000000;

	int i;
	for(i=0; i<eFishItem_Max; i++)
	{
		m_fFishItemRate[i] = 0.0f;
	}
	m_lstGetFishList.clear();
	

	m_wFishingLevel = 1;
	m_dwFishingExp = 0;
	m_dwFishPoint = 0;
	mReviveFlag = ReviveFlagNone;
	m_dwKillCountFromGT = 0;
	m_dwKillPointFromGT = 0;
	m_dwRespawnTimeOnGTMAP = 0;
	m_dwImmortalTimeOnGTMAP = 0;
	m_wCookLevel = 1;
	m_wCookCount = 0;
	m_wEatCount = 0;
	m_wFireCount = 0;
	m_dwLastCookTime = 0;
	memset(m_MasterRecipe, 0, sizeof(m_MasterRecipe));
	// 090316 LUJ, Ż �� �ʱ�ȭ
	SetSummonedVehicle( 0 );
	SetMountedVehicle( 0 );
	m_initState = 0;

	mWeaponEnchantLevel = 0;
	mPhysicDefenseEnchantLevel = 0;
	mMagicDefenseEnchantLevel = 0;

	strcpy(m_szHouseName, "");

	m_dwCurrentResurrectIndex = 0;
	mPetIndex = 0;
	m_bDungeonObserver = FALSE;

	m_dwConsignmentTick = 0;
	ForbidChatTime = 0;
	m_registeredstreettournament = FALSE;
	m_STrank = 0;
	m_MovedOnSt = FALSE;
	m_lastspinslot = 0;
	m_pSpinslothasil1 = 0;
	m_pSpinslothasil2 = 0;
	m_pSpinslothasil3 = 0;
	m_pSpinslothasil4 = 0;
	m_wincodehasil = 0;
	SlotGetmoneyhasil = 0;
	m_additemachievment = 0;
	m_MaxCountSpin = 0;
	m_VerifyCaptcha = 0;
	m_VerifyCount = 0;
	m_VerifyKillCount = 0;
	m_index = 0;
// --- skr 22012020
	RelifeON = FALSE;
	RelifeStartTime = 0;
	currentwarehouseset = 0;
}

BOOL CPlayer::Init(EObjectKind kind,DWORD AgentNum, BASEOBJECT_INFO* pBaseObjectInfo)
{
	m_StreetStallTitle[0] = 0;
	m_MurdererIDX = 0;
	m_MurdererKind = 0;
	m_bPenaltyByDie = FALSE;

	m_bReadyToRevive = FALSE;
	m_bShowdown	= FALSE;
	m_bExit = FALSE;
	m_bNormalExit = FALSE;
//
	CObject::Init(kind, AgentNum, pBaseObjectInfo); //���ϡ�a������A eObjectState_NoneA����I ������c��U.

//KES 040827
	OBJECTSTATEMGR_OBJ->StartObjectState( this, eObjectState_Immortal, 0 );
	// 06.08.29. RaMa.
	OBJECTSTATEMGR_OBJ->EndObjectState( this, eObjectState_Immortal, 30000 );
//

	m_CurComboNum = 0;
	m_pGuetStall = 0;
	m_bNeedRevive = TRUE;
	ZeroMemory(
		&m_GameOption,
		sizeof(m_GameOption));
	m_ContinuePlayTime = 0;

//HACK CHECK INIT
	m_nHackCount = 0;
	m_dwHackStartTime = gCurTime;
	m_wKillMonsterLevel = 0;
	m_bDieForGFW = FALSE;
	m_bWaitExitPlayer = TRUE;
	m_bWaitExitPlayer = TRUE;

//	m_SkillList.RemoveAll();
	m_SkillFailCount = 0;

	for( BYTE i = 0; i < 8; i++ )
	{
		m_QuickSlot[i].Init( GetID(), i );
	}

	m_SkillTree->Init( this );
	m_stMonstermeterInfo.nPlayTimeTick = gCurTime;
	mIsEmergency = FALSE;

	mTargetMap = 0;
	mTargetPosX = 0;
	mTargetPosZ = 0;

	m_dwLastTimeCheckItemDBUpdate = gCurTime;
	m_dwLastTimeCheckItem = gCurTime;

	m_bNoExpPenaltyByPK = FALSE;

	m_pvpscore = 0;

	// 080515 LUJ, ��Ÿ�� üũ�� ����ü �ʱ�ȭ
	ZeroMemory( &mCheckCoolTime, sizeof( mCheckCoolTime ) );

	m_dwSkillCancelCount = 0;
	m_dwSkillCancelLastTime = 0;

	m_dwAutoNoteLastExecuteTime = 0;
	
	mWeaponEnchantLevel = 0;
	mPhysicDefenseEnchantLevel = 0;
	mMagicDefenseEnchantLevel = 0;

	m_dwCurrentResurrectIndex = 0;

	ZeroMemory( &m_YYManaRecoverTime, sizeof(m_YYManaRecoverTime) );
	m_ManaRecoverDirectlyAmount = 0;

// --- skr 22012020
	RelifeON = FALSE;
	RelifeStartTime = 0;
	currentwarehouseset = 0;

	return TRUE;
}
void CPlayer::Release()
{
	RemoveAllAggroed();
//-------------------

	FishingData_Update(GetID(), GetFishingLevel(), GetFishingExp(), GetFishPoint());
	//aziz MallShop in Game Method 1
	VipData_Update(GetID(), GetVipPoint());
	//aziz Reborn 24 Sep
	RebornData_Update(GetID(), GetRebornData());
	//aziz Kill Shop 30 Sep
	KillData_Update(GetID(), GetKillPoint(), 3);
	//aziz Reborn Point 13 Oct
	RebornPoint_Update(GetID(), GetRebornPoint());

	Cooking_Update(this);

	if(FISHINGMGR->GetActive())		// ���ý�ũ��Ʈ�� Ȱ���Ǿ� ������(==2�� �����)�� ���.
	{
		// 080808 LUJ, ���� ������ �α׷� ����
		Log_Fishing(
			GetID(),
			eFishingLog_Regular,
			0,
			GetFishPoint(),
			0,
			0,
			GetFishingExp(),
			GetFishingLevel() );
	}

	LogOnRelease();
	GUILDMGR->RemovePlayer( this );
	ITEMMGR->RemoveCoolTime( GetID() );
	mCoolTimeMap.clear();
	mSkillCoolTimeMap.clear();
	mSkillAnimTimeMap.clear();

	{
		for( POSTYPE position = 0; position < m_ItemContainer.GetSize(); ++position )
		{
			const ITEMBASE* item = m_ItemContainer.GetItemInfoAbs( position );

			if( ! item )
			{
				continue;
			}

			ITEMMGR->RemoveOption( item->dwDBIdx );
		}
	}

	{
		m_ItemArrayList.SetPositionHead();

		for(
			ITEMOBTAINARRAYINFO* pInfo;
			(pInfo = m_ItemArrayList.GetData())!= NULL; )
		{
			ITEMMGR->Free(this, pInfo);
		}

		m_ItemArrayList.RemoveAll();
	}

	m_FollowMonsterList.RemoveAll();
	
	{		
		m_QuestList.SetPositionHead();

		for(
			CQuestBase* pQuest;
			(pQuest = m_QuestList.GetData())!= NULL; )
		{
			delete pQuest;
		}

		m_QuestList.RemoveAll();
	}
	

	m_InventoryPurse.Release();
	m_StoragePurse.Release();
	m_QuestGroup.Release();
	CObject::Release();

	mSpecialSkillList.clear();
	m_SkillFailCount = 0;
	m_SkillTree->Release();

	SAFE_DELETE_ARRAY(m_pcFamilyEmblem);	

	{
		CPet* const petObject = PETMGR->GetPet(
			GetPetItemDbIndex());

		g_pServerSystem->RemovePet(
			petObject ? petObject->GetID() : 0,
			FALSE);
	}

	// 090316 LUJ, ���� ó���Ѵ�
	{
		CVehicle* const vehicleObject = ( CVehicle* )g_pUserTable->FindUser( GetMountedVehicle() );

		if( vehicleObject &&
			vehicleObject->GetObjectKind() == eObjectKind_Vehicle )
		{
			vehicleObject->Dismount( GetID(), TRUE );
		}
	}

	mWeaponEnchantLevel = 0;
	mPhysicDefenseEnchantLevel = 0;
	mMagicDefenseEnchantLevel = 0;
}

void CPlayer::UpdateGravity()
{
	mGravity = 0;
	m_FollowMonsterList.SetPositionHead();

	while(CObject* const pObject = m_FollowMonsterList.GetData())
	{
		mGravity += pObject->GetGravity();
	}
}

BOOL CPlayer::AddFollowList(CMonster * pMob)
{
	if( m_FollowMonsterList.GetDataNum() < 50 )		//max 50����
	{
		m_FollowMonsterList.Add(pMob, pMob->GetID());
		UpdateGravity();
		return TRUE;
	}

	return FALSE;
}
BOOL CPlayer::RemoveFollowAsFarAs(DWORD GAmount, CObject* pMe )
{	
	VECTOR3 * ObjectPos	= CCharMove::GetPosition(this);
	BOOL bMe = FALSE;

	while(GAmount > 100)
	{	
		CMonster * MaxObject = NULL;
		float	MaxDistance	= -1;
		float	Distance	= 0;

		m_FollowMonsterList.SetPositionHead();
		while(CMonster* pObject = m_FollowMonsterList.GetData())
		{
			VECTOR3 * TObjectPos = CCharMove::GetPosition(pObject);
			if((Distance = CalcDistanceXZ( ObjectPos, TObjectPos )) > MaxDistance)
			{
				MaxDistance = Distance;
				MaxObject = pObject;
			}
		}

		if(MaxObject)
		{
			if(GAmount > MaxObject->GetGravity())
				GAmount -= MaxObject->GetGravity();
			else
				GAmount = 0;

			MaxObject->SetTObject(NULL);

			GSTATEMACHINE.SetState(MaxObject, eMA_WALKAROUND);

			if( pMe == MaxObject )
				bMe = TRUE;
		}
		else
		{
			MHERROR->OutputFile("Debug.txt", MHERROR->GetStringArg("amount != 0"));
			GAmount = 0;
		}
	}

	return bMe;	
}

void CPlayer::RemoveFollowList(DWORD ID)
{
	m_FollowMonsterList.Remove(ID);
	UpdateGravity();
}

ITEMOBTAINARRAYINFO * CPlayer::GetArray(WORD id)
{
	return m_ItemArrayList.GetData(id);
}

void CPlayer::AddArray(ITEMOBTAINARRAYINFO * pInfo)
{
	m_ItemArrayList.Add(pInfo, pInfo->wObtainArrayID);
}
void CPlayer::RemoveArray(DWORD id)
{
	m_ItemArrayList.Remove(id);
}
void CPlayer::InitCharacterTotalInfo(CHARACTER_TOTALINFO* pTotalInfo)
{
	memcpy(&m_HeroCharacterInfo,pTotalInfo,sizeof(CHARACTER_TOTALINFO));
	if( GetUserLevel() == eUSERLEVEL_GM )
		m_HeroCharacterInfo.bVisible = FALSE;
	else
		m_HeroCharacterInfo.bVisible = TRUE;

	// 071226 KTH -- �κ��丮�� ũ�⸦ �����Ͽ� �ش�.
	m_InventorySlot.SetSlotNum( POSTYPE( SLOT_INVENTORY_NUM + GetInventoryExpansionSize() ) );

}

void CPlayer::InitHeroTotalInfo(HERO_TOTALINFO* pHeroInfo)
{
	memcpy(&m_HeroInfo,pHeroInfo,sizeof(HERO_TOTALINFO));
	m_ItemContainer.GetSlot(eItemTable_Inventory)->CreatePurse(&m_InventoryPurse, this, m_HeroInfo.Money, MAX_INVENTORY_MONEY);
}

void CPlayer::InitItemTotalInfo(ITEM_TOTALINFO* pItemInfo)
{
	m_ItemContainer.GetSlot(eItemTable_Inventory)->SetItemInfoAll(pItemInfo->Inventory);	
	m_ItemContainer.GetSlot(eItemTable_Weared)->SetItemInfoAll(pItemInfo->WearedItem);
}

void CPlayer::AddStorageItem(ITEMBASE * pStorageItem)
{
	CStorageSlot * pSlot = (CStorageSlot *)m_ItemContainer.GetSlot(eItemTable_Storage);
	pSlot->InsertItemAbs(this, pStorageItem->Position, pStorageItem);
}

void CPlayer::InitStorageInfo(BYTE Storagenum,MONEYTYPE money)
{
	CStorageSlot * pSlot = (CStorageSlot *)m_ItemContainer.GetSlot(eItemTable_Storage);
	pSlot->SetStorageNum(Storagenum);

	MONEYTYPE maxmoney = 0;
	if(Storagenum)
	{		
		STORAGELISTINFO* pInfo = STORAGEMGR->GetStorageInfo(Storagenum);
		ASSERT(pInfo);
		maxmoney = pInfo ? pInfo->MaxMoney : 10;
	}
	else
	{
		ASSERT(money == 0);
		maxmoney = 0;
	}
	pSlot->CreatePurse(&m_StoragePurse, this, money, maxmoney);	
}

void CPlayer::InitShopItemInfo(SEND_SHOPITEM_INFO& message)
{
	CItemSlot* const pSlot = m_ItemContainer.GetSlot(
		eItemTable_Shop);

	for(DWORD i = 0; i < _countof(message.Item); ++i)
	{
		pSlot->ClearItemBaseAndSlotInfo(
			POSTYPE(TP_SHOPITEM_START + i));

		ITEMBASE& itemBase = message.Item[i];

		if(0 == itemBase.dwDBIdx)
		{
			continue;
		}

		pSlot->InsertItemAbs(
			this,
			itemBase.Position,
			&itemBase);
	}
}

void CPlayer::SetStorageNum(BYTE n)
{
	CStorageSlot * pSlot = (CStorageSlot *)m_ItemContainer.GetSlot(eItemTable_Storage);
	pSlot->SetStorageNum(n);
}

BYTE CPlayer::GetStorageNum()
{
	CStorageSlot * pSlot = (CStorageSlot *)m_ItemContainer.GetSlot(eItemTable_Storage);
	return pSlot->GetStorageNum();
}

MONEYTYPE CPlayer::GetMoney(eITEMTABLE tableIdx)
{
	CItemSlot* pSlot = m_ItemContainer.GetSlot(tableIdx);

	return pSlot ? pSlot->GetMoney() : 0;
}

void CPlayer::GetItemtotalInfo(ITEM_TOTALINFO& pRtInfo,DWORD dwFlag)
{
	if(dwFlag & GETITEM_FLAG_INVENTORY)
	{
		m_ItemContainer.GetSlot(eItemTable_Inventory)->GetItemInfoAll(
			pRtInfo.Inventory,
			_countof(pRtInfo.Inventory));
	}

	if(dwFlag & GETITEM_FLAG_WEAR)
	{
		m_ItemContainer.GetSlot(eItemTable_Weared)->GetItemInfoAll(
			pRtInfo.WearedItem,
			_countof(pRtInfo.WearedItem));
	}
}

// 091026 LUJ, ���� ũ�⿡ �����ϰ� ������ �� �ֵ��� ����
DWORD CPlayer::SetAddMsg(DWORD dwReceiverID, BOOL isLogin, MSGBASE*& sendMessage)
{
	if(eUSERLEVEL_GM >= GetUserLevel() &&
		FALSE == IsVisible())
	{
		return 0;
	}

	static SEND_CHARACTER_TOTALINFO message;
	ZeroMemory( &message, sizeof( message ) );
	message.Category = MP_USERCONN;
	message.Protocol = MP_USERCONN_CHARACTER_ADD;
	message.dwObjectID = dwReceiverID;
	GetSendMoveInfo( &message.MoveInfo,&message.AddableInfo );
	GetBaseObjectInfo( &message.BaseObjectInfo);
	GetCharacterTotalInfo( &message.TotalInfo );
	message.TotalInfo.DateMatching = m_DateMatching;
	message.TotalInfo.AliasType = m_AliasType;
	message.bLogin = BYTE(isLogin);
	message.mMountedVehicle.mVehicleIndex = GetSummonedVehicle();

	// 090316 LUJ, ž�� ���� ����
	{
		CVehicle* const vehicle = ( CVehicle* )g_pUserTable->FindUser( GetMountedVehicle() );

		if( vehicle && vehicle->GetObjectKind() == eObjectKind_Vehicle )
		{
			message.mMountedVehicle.mVehicleIndex = vehicle->GetID();
			message.mMountedVehicle.mSeatIndex = vehicle->GetMountedSeat( GetID() );
		}
	}

	// �Ͽ�¡ ž�� ���� ����
	{
		const stHouseRiderInfo* const houseRiderInfo = HOUSINGMGR->GetRiderInfo(GetID());

		if(houseRiderInfo)
		{
			message.mRiderInfo.dwFurnitureObjectIndex = houseRiderInfo->dwFurnitureObjectIndex;
			message.mRiderInfo.wSlot = houseRiderInfo->wSlot;
		}
	}

	cStreetStall* pStall = STREETSTALLMGR->FindStreetStall(this);

	if( pStall != NULL)
	{
		char StallTitle[MAX_STREETSTALL_TITLELEN+1] = {0};
		GetStreetStallTitle(StallTitle);
		CAddableInfoList::AddableInfoKind kind = CAddableInfoList::None;

		switch(pStall->GetStallKind())
		{
		case eSK_SELL:
			{

				kind = CAddableInfoList::StreetStall;
				break;
			}
		case eSK_BUY:
			{
				kind = CAddableInfoList::StreetBuyStall;
				break;
			}
		}
		
		message.AddableInfo.AddInfo(
			BYTE(kind),
			WORD(strlen(StallTitle)+1),
			StallTitle,
			__FUNCTION__);
	}

	switch(GetBattle()->GetBattleKind())
	{
	case eBATTLE_KIND_SHOWDOWN:
		{
			COMPRESSEDPOS ShowdownPos;
			BATTLE_INFO_SHOWDOWN info;
			WORD wSize = 0;
			GetBattle()->GetBattleInfo( (char*)&info, &wSize );
			ShowdownPos.Compress(&info.vStgPos);
			message.AddableInfo.AddInfo(CAddableInfoList::ShowdownInfo,sizeof(COMPRESSEDPOS),&ShowdownPos, __FUNCTION__ );
			break;
		}
	case eBATTLE_KIND_GTOURNAMENT:
		{
			int nTeam = GetBattle()->GetBattleTeamID(this);
			message.AddableInfo.AddInfo(CAddableInfoList::GTournament, sizeof(nTeam), &nTeam, __FUNCTION__ );
			break;
		}
	}
	//for street tournament //johan cek
	/*if(STREETTOURNAMENTMGR->GetState() != 0)
	{
		MSG_WORD3 msg2 ;
		ZeroMemory(&msg2, sizeof(msg2));
		msg2.Category	= MP_USERCONN ;
		msg2.Protocol	= MP_USERCONN_STREETTOURNAMENT_STATE;
		msg2.wData1		= 0 ;
		msg2.wData2		= STREETTOURNAMENTMGR->GetState();
		msg2.wData3		= STREETTOURNAMENTMGR->GetStage();
		SendMsg( &msg2, sizeof(msg2) );
	} */

	sendMessage = &message;
	return message.GetMsgLength();
}

void CPlayer::SetStreetStallTitle(char* title)
{
	SafeStrCpy( m_StreetStallTitle, title, MAX_STREETSTALL_TITLELEN+1 );
}

void CPlayer::GetStreetStallTitle(char* title)
{
	SafeStrCpy( title, m_StreetStallTitle, MAX_STREETSTALL_TITLELEN+1);
}

void CPlayer::CalcState()
{
	//CHARCALCMGR->CalcItemStats(this);
	CHARCALCMGR->CalcCharStats( this );

	SetLife(m_HeroCharacterInfo.Life);
	SetMana(m_HeroInfo.Mana);
}

void CPlayer::MoneyUpdate( MONEYTYPE money )
{	
	m_HeroInfo.Money = money;
}

void CPlayer::SetStrength(DWORD val)
{
	m_HeroInfo.Str = val;

	CHARCALCMGR->CalcCharStats(this);

	// DB��?�ˢ� ��u��A��i��IAI������c
	CharacterHeroInfoUpdate(this);

	MSG_DWORD msg;
	msg.Category = MP_CHAR;
	msg.Protocol = MP_CHAR_STR_NOTIFY;
	msg.dwObjectID = GetID();
	msg.dwData = val;
	SendMsg(&msg,sizeof(msg));
}

void CPlayer::SetDexterity(DWORD val)
{
	m_HeroInfo.Dex = val;

	CHARCALCMGR->CalcCharStats(this);

	// DB��?�ˢ� ��u��A��i��IAI������c
	CharacterHeroInfoUpdate(this);

	MSG_DWORD msg;
	msg.Category = MP_CHAR;
	msg.Protocol = MP_CHAR_DEX_NOTIFY;
	msg.dwObjectID = GetID();
	msg.dwData = val;
	SendMsg(&msg,sizeof(msg));
}
void CPlayer::SetVitality(DWORD val)
{
	m_HeroInfo.Vit = val;

	// ��iy�ˡ�i����A, ��o�Ϣ���ui����A; �ˡ�U��oA ����e��ie
	CHARCALCMGR->CalcCharStats(this);

	// DB��?�ˢ� ��u��A��i��IAI������c
	CharacterHeroInfoUpdate(this);

	MSG_DWORD msg;
	msg.Category = MP_CHAR;
	msg.Protocol = MP_CHAR_VIT_NOTIFY;
	msg.dwObjectID = GetID();
	msg.dwData = val;
	SendMsg(&msg,sizeof(msg));
}
void CPlayer::SetWisdom(DWORD val)
{
	m_HeroInfo.Wis = val;

	// �ϩ���i����A; �ˡ�U��oA ����e��ie
	CHARCALCMGR->CalcCharStats(this);

	// DB��?�ˢ� ��u��A��i��IAI������c
	CharacterHeroInfoUpdate(this);

	MSG_DWORD msg;
	msg.Category = MP_CHAR;
	msg.Protocol = MP_CHAR_WIS_NOTIFY;
	msg.dwObjectID = GetID();
	msg.dwData = val;
	SendMsg(&msg,sizeof(msg));
}

// 070411 LYW --- Player : Add function to setting intelligence.
void CPlayer::SetIntelligence( DWORD val )
{
	m_HeroInfo.Int = val ;

	CHARCALCMGR->CalcCharStats(this);

	CharacterHeroInfoUpdate(this);

	MSG_DWORD msg;
	msg.Category = MP_CHAR;
	msg.Protocol = MP_CHAR_INT_NOTIFY;
	msg.dwObjectID = GetID();
	msg.dwData = val;
	SendMsg(&msg,sizeof(msg));
}

CItemSlot * CPlayer::GetSlot(POSTYPE absPos)
{
	if( TP_INVENTORY_START <= absPos && absPos < TP_SHOPITEM_END )
	{
		return m_ItemContainer.GetSlot(absPos);
	}
	else if( TP_GUILDWAREHOUSE_START <= absPos && absPos < TP_GUILDWAREHOUSE_END )
	{
		return (CItemSlot*)GUILDMGR->GetSlot( GetGuildIdx());
	}
	else if( TP_SHOPITEM_START <= absPos && absPos < TP_SHOPITEM_END )
	{
		return m_ItemContainer.GetSlot(absPos);
	}

	return NULL;
}

CItemSlot * CPlayer::GetSlot(eITEMTABLE tableIdx)
{
	if(tableIdx < eItemTable_TableMax)
		return m_ItemContainer.GetSlot(tableIdx);
	else if( tableIdx == eItemTable_MunpaWarehouse)
	{
		return (CItemSlot*)GUILDMGR->GetSlot(GetGuildIdx());
	}
	return NULL;
}

//-------------------------------------------------------------------------------------------------
//	NAME : SetLifeForce
//	DESC : 080625 LYW 
//		   ĳ���Ͱ� ���� ���¿��� �����̳� ƨ�� ������ �߻� �� ��, 
//		   ĳ������ ����� 50%�� ������ �־�� �Ѵ�. �׷��� ���°� ���� ���¸�, 
//		   ���� �Լ��� return ó���� �ϹǷ�, ������ ���� ���θ� ������ �� �ִ� �Լ��� �߰��Ѵ�.
//-------------------------------------------------------------------------------------------------
void CPlayer::SetLifeForce(DWORD Life, BYTE byForce, BOOL bSendMsg) 
{
	// ���� ���� ���θ� Ȯ���Ѵ�.
	if(byForce == FALSE)
	{
		// ĳ���Ͱ� ���� ���¶��, return ó���� �Ѵ�.
		if(GetState() == eObjectState_Die) return ;
	}


	// ���� ĳ���� ������ �´� �ִ� ������� �޴´�.
	DWORD maxlife = 0 ;
	maxlife = GetMaxLife() ;


	// ����� ��ġ ��ȿ üũ.
	if(Life > maxlife) Life = maxlife ;


	// ���ڷ� �Ѿ�� ������� ���� ����� ���� ������, return ó���� �Ѵ�.
	if( m_HeroCharacterInfo.Life >= Life ) return ;

	
	// ���� ����� / ���ڷ� �Ѿ�� ������� ���� ������,
	if(m_HeroCharacterInfo.Life != Life)
	{
		// �޽��� ���� �ϴ� ��Ȳ�̶��,
		if(bSendMsg == TRUE)
		{
			//// ���ο� ������� ����Ѵ�.
			//DWORD dwNewLife = 0 ;
			//dwNewLife = Life - GetLife() ;

			// �޽����� �����Ѵ�.
			MSG_INT msg ;
			msg.Category = MP_CHAR ;
			msg.Protocol = MP_CHAR_LIFE_ACK ;
			msg.dwObjectID = GetID() ;
			//msg.nData = dwNewLife ;
			msg.nData = Life ;

			SendMsg(&msg,sizeof(msg)) ;
		}
		
		SendLifeToParty(
			Life);
	}
		

	// ĳ������ ������� �����Ѵ�.
	m_HeroCharacterInfo.Life = Life ;
}





void CPlayer::SetLife(DWORD val,BOOL bSendMsg) 
{
	if(GetState() == eObjectState_Die)
		return;

	DWORD maxlife = GetMaxLife();
	if(val > maxlife)
		val = maxlife;
	
	if(m_HeroCharacterInfo.Life != val)	// ��i��I��oIAo AI�����Ϣ���?i��?�ˢ�ˡ�A ��?����Aa��?�ˢ硧u���� ��i��IA�����ˡ�U.
	{
		if(bSendMsg == TRUE)
		{
			// To A������OoAI��u�ϡ̡�����c -------------------------------
			MSG_INT msg;
			msg.Category = MP_CHAR;
			msg.Protocol = MP_CHAR_LIFE_BROADCAST;
			msg.dwObjectID = GetID();
			// 070419 LYW --- Player : Modified function SetLife.
			msg.nData = val - GetLife();
			PACKEDDATA_OBJ->QuickSend( this, &msg, sizeof( msg ) );
		}
		
		SendLifeToParty(
			val);
	}
	m_HeroCharacterInfo.Life = val;
	
}

void CPlayer::SendLifeToParty(DWORD val)
{	
	if(CParty* pParty = PARTYMGR->GetParty(GetPartyIdx()))
	{
		MSG_DWORD2 message;
		ZeroMemory(
			&message,
			sizeof(message));
		message.Category = MP_PARTY;
		message.Protocol = MP_PARTY_MEMBERLIFE;
		message.dwData1 = GetID();
		message.dwData2 = val * 100 / GetMaxLife();

		pParty->SendMsgExceptOneinChannel(
			&message,
			sizeof(message),
			GetID(),
			GetGridID());
	}
}





//-------------------------------------------------------------------------------------------------
//	NAME : SetLifeForce
//	DESC : 080625 LYW 
//		   ĳ���Ͱ� ���� ���¿��� �����̳� ƨ�� ������ �߻� �� ��, 
//		   ĳ������ ���� 30%�� ������ �־�� �Ѵ�. �׷��� ���°� ���� ���¸�, 
//		   ���� �Լ��� return ó���� �ϹǷ�, ������ ���� ���θ� ������ �� �ִ� �Լ��� �߰��Ѵ�.
//-------------------------------------------------------------------------------------------------
void CPlayer::SetManaForce(DWORD Mana, BYTE byForce, BOOL bSendMsg) 
{
	// ���� ���� ���θ� Ȯ���Ѵ�.
	if(byForce == FALSE)
	{
		// ĳ���Ͱ� ���� ���¶��, return ó���� �Ѵ�.
		if(GetState() == eObjectState_Die) return ;
	}


	// ĳ������ ���� ������ �ִ� ���� ��ġ�� �޴´�.
	DWORD MaxMana = 0 ;
	MaxMana = GetMaxMana() ;


	// ���ڷ� �Ѿ�� ������ ��ȿ ������ üũ.
	if(Mana > MaxMana) Mana = MaxMana ;


	// ���ڷ� �Ѿ�� �������� ���� ������ ���� ������, return ó���� �Ѵ�.
	if( m_HeroInfo.Mana >= Mana ) return ;


	// �������� / ���ڷ� �Ѿ�� ������ ���� ������,
	if( m_HeroInfo.Mana != Mana)
	{
		// �޽��� ���� ���ΰ� TRUE �̸�,
		if(bSendMsg)
		{
			// ������ �����Ѵ�.
			MSG_DWORD msg ;
			msg.Category = MP_CHAR ;
			msg.Protocol = MP_CHAR_MANA_GET_ACK;
			msg.dwObjectID = GetID() ;
			msg.dwData = Mana ;
			SendMsg(&msg,sizeof(msg)) ;
		}
		
		SendManaToParty(
			Mana);
	}
	

	// ĳ������ ������ �����Ѵ�.
	m_HeroInfo.Mana = Mana ; 
}





void CPlayer::SetMana(DWORD val,BOOL bSendMsg)
{ 
	if(GetState() == eObjectState_Die)
		return;

	DWORD MaxMana = GetMaxMana();
	if(val > MaxMana)
		val = MaxMana;

	if( m_HeroInfo.Mana != val)
	{
		// 100223 ShinJS --- ���� ������ �߰��� ���� ���� ������ ��ȭ������ ����.
		if(bSendMsg)
		{
			MSG_INT msg;
			msg.Category = MP_CHAR;
			msg.Protocol = MP_CHAR_MANA_ACK;
			msg.dwObjectID = GetID();
			msg.nData = val - GetMana();
			SendMsg(&msg,sizeof(msg));
		}
		
		SendManaToParty(
			val);
	}
	m_HeroInfo.Mana = val; 
	
}

void CPlayer::SendManaToParty(DWORD mana)
{
	if(CParty* pParty = PARTYMGR->GetParty(GetPartyIdx()))
	{
		MSG_DWORD2 message;
		ZeroMemory(
			&message,
			sizeof(message));
		message.Category = MP_PARTY;
		message.Protocol = MP_PARTY_MEMBERMANA;
		message.dwData1 = GetID();
		message.dwData2 = mana * 100 / GetMaxMana();

		pParty->SendMsgExceptOneinChannel(
			&message,
			sizeof(message),
			GetID(),
			GetGridID());
	}
}

void CPlayer::SetMaxLife(DWORD val)
{
	m_HeroCharacterInfo.MaxLife = val;

	// To A������OoAI��u�ϡ̡�����c -------------------------------
	MSG_DWORD msg;
	msg.Category = MP_CHAR;
	msg.Protocol = MP_CHAR_MAXLIFE_NOTIFY;
	msg.dwObjectID = GetID();
	msg.dwData = val;
	PACKEDDATA_OBJ->QuickSend( this, &msg, sizeof( msg ) );
}

void CPlayer::SetMaxMana(DWORD val)
{
	m_HeroInfo.MaxMana= val;

	// To A������OoAI��u�ϡ̡�����c -------------------------------
	MSG_DWORD msg;
	msg.Category = MP_CHAR;
	msg.Protocol = MP_CHAR_MAXMANA_NOTIFY;
	msg.dwObjectID = GetID();
	msg.dwData = val;
	SendMsg(&msg,sizeof(msg));
}
void CPlayer::AddStrength(int val)
{
	SetStrength(m_HeroInfo.Str+val);
}
void CPlayer::AddWisdom(int val)
{
	SetWisdom(m_HeroInfo.Wis+val);
}
void CPlayer::AddDexterity(int val)
{
	SetDexterity(m_HeroInfo.Dex+val);
}
void CPlayer::AddVitality(int val)
{
	SetVitality(m_HeroInfo.Vit+val);
}
// 070411 LYW --- Player : Add function to setting intelligence.
void CPlayer::AddIntelligence( int val )
{
	SetIntelligence( m_HeroInfo.Int+ val ) ;
}
void CPlayer::SetPlayerLevelUpPoint(LEVELTYPE val) 
{ 
	m_HeroInfo.LevelUpPoint=val;
	
	MSG_DWORD msg;
	msg.Category = MP_CHAR;
	msg.Protocol = MP_CHAR_LEVELUPPOINT_NOTIFY;
	msg.dwObjectID = GetID();
	msg.dwData = val;
	SendMsg(&msg,sizeof(msg));
}
/*****************************************************************/
/* 1. AE����a��?�ˢ� A�ϩ��ˡ�?AI�����ˢ� ��iy��u������iC��uu;��O���� from DBResultQuery
/* 2. SetPlayerExpPoint()��?�ˢ硧u���� AO�ˡ�e �����Ϣ�CeA�ˢ�ˡ�| �ϩ�N��ui ��u��o; ��O���� call��iE
/*
/*
/*****************************************************************/

void CPlayer::SetLevel(LEVELTYPE level)
{
	if(level >= MAX_PLAYERLEVEL_NUM)
	{
		ASSERT(0);
		return;
	}

	if(m_HeroCharacterInfo.Level == level) return;

	LEVELTYPE AddPoint = 0;
	if(level > m_HeroCharacterInfo.Level)
	{
		if(m_HeroInfo.MaxLevel < level)
		{
			m_HeroInfo.MaxLevel = level;
			AddPoint += 1; //johan menambahkan 1 stat point setiap naik level

			DWORD skillpoint = (DWORD)( ceil( level / 4 ) + 3 );

			if(m_HeroCharacterInfo.Race == RaceType_Devil)
			{
				skillpoint = (DWORD)( ceil( level / 4 ) + 1 );
			}

			SetSkillPoint( skillpoint, FALSE );

			// ���̺�Ʈ
			if( level == 10 )
			{
				WebEvent( GetUserID(), 1 );
			}
		}
	}

	SetLife(
		GetMaxLife());
	SetMana(
		GetMaxMana());

	m_HeroCharacterInfo.Level = level;
	CHARCALCMGR->CalcCharStats( this );

	MSG_LEVEL message;
	ZeroMemory(
		&message,
		sizeof(message));
	message.Category = MP_CHAR;
	message.Protocol = MP_CHAR_LEVEL_NOTIFY;
	message.dwObjectID = GetID();
	message.Level = GetLevel();
	message.MaxExpPoint = GAMERESRCMNGR->GetMaxExpPoint(level);
	message.CurExpPoint = GetPlayerExpPoint();
	PACKEDDATA_OBJ->QuickSend(
		this,
		&message,
		sizeof(message));

	SetPlayerLevelUpPoint(GetPlayerLevelUpPoint() + AddPoint);

	CQuestEvent QEvent(
		eQuestEvent_Level,
		level,
		1);
	QUESTMGR->AddQuestEvent(
		this,
		&QEvent);

	PARTYMGR->MemberLevelUp(
		GetPartyIdx(),
		GetID(),
		GetLevel());
	GUILDMGR->MemberLevelUp(
		GetGuildIdx(),
		GetID(),
		GetLevel());
	g_csFamilyManager.SRV_UpdateMapPlayerInfo(
		this);
}

// 080611 LYW --- Player : ��ų����Ʈ ������Ʈ ó���� ������.
// (���������� ��ų ����Ʈ�� �߰��ϴ� ����� ����� ����.)
//void CPlayer::SetSkillPoint( DWORD point )
void CPlayer::SetSkillPoint( DWORD point, BYTE byForced )
{
	// ���� ������Ʈ ���� üũ.
	ASSERT(byForced <= TRUE) ;
	if(byForced > TRUE) return ;


	// ��ų ����Ʈ ������Ʈ.
	GetHeroTotalInfo()->SkillPoint += point;


	// Ŭ���̾�Ʈ ����
	MSG_DWORD msg;

	msg.Category = MP_SKILLTREE;
	msg.Protocol = MP_SKILLTREE_POINT_NOTIFY;
	msg.dwObjectID = GetID();
	msg.dwData = point;

	SendMsg(&msg, sizeof(msg));


	// DB ������Ʈ
	SkillPointUpdate( GetID(), GetSkillPoint() );


	// 071129 LYW --- Player : ���� ��ų ����Ʈ ������Ʈ.
	//DB_UpdateAccumulateSkillPoint(GetID(), FALSE, point) ;
	DB_UpdateAccumulateSkillPoint(GetID(), byForced, point) ;


	// 071114 ����. �α�
	{
		const SKILL_BASE emptyData = { 0 };

		InsertLogSkill( this, &emptyData, eLog_SkillGetPoint );
	}
}

DWORD CPlayer::GetSkillPoint()
{
	return GetHeroTotalInfo()->SkillPoint;
}

void CPlayer::SetPlayerExpPoint(EXPTYPE point)
{
	// 071119 ����, �ѹ��� ���� �ܰ踦 �������� �� �ֵ��� �����ϰ� �ڵ带 �ܼ�ȭ��

	const LEVELTYPE& level = m_HeroCharacterInfo.Level;
	
	ASSERT( level <= MAX_CHARACTER_LEVEL_NUM );

	if( level == MAX_CHARACTER_LEVEL_NUM )
	{
		const EXPTYPE& BeforeExp = m_HeroInfo.ExpPoint;
		// Max�����϶� ����ġ �϶��� ���� �ʴ� ���� ����
		// ����ġ ����϶��� ���Ͻ�Ų��.
		if( point > BeforeExp )
		{
			return;
		}
	}	
	
	// ����ġ�� ���� �ܰ迡�� �䱸�ϴ� �ͺ��� �ξ� ���� �� �����Ƿ�,
	// ��� üũ�ؼ� ����������
	{
		EXPTYPE nextPoint = GAMERESRCMNGR->GetMaxExpPoint( level );

		while( point >= nextPoint )
		{
			SetLevel( level + 1 );

			CharacterHeroInfoUpdate( this );
			CharacterTotalInfoUpdate( this );

			InsertLogCharacter( GetID(), m_HeroCharacterInfo.Level, &m_HeroInfo );
			InsertLogExp( eExpLog_LevelUp, GetID(), level, 0, GetPlayerExpPoint(), 0, 0, 0 );

			point		-=	nextPoint;
			nextPoint	=	GAMERESRCMNGR->GetMaxExpPoint( level );
		}

		m_HeroInfo.ExpPoint = point;
	}

	{
		MSG_EXPPOINT message;
		message.Category	= MP_CHAR;
		message.Protocol	= MP_CHAR_EXPPOINT_ACK;
		message.ExpPoint	= point;

		SendMsg( &message, sizeof( message ) );
	}
}
void CPlayer::AddPlayerExpPoint(EXPTYPE Exp)
{
	if( Exp == 0 ) return;

	EXPTYPE NewExp = 0 ;

	// 090213 LYW --- Player : �йи� ��� ���ӿ� ���� �߰� ����ġ�� �����Ѵ�. ( ���� ���� ����ġ ���� )
	if( GetFamilyIdx() )
	{
		NewExp = GetPlayerExpPoint() + Exp + m_dweFamilyRewardExp ;
	}
	else
	{
		NewExp = GetPlayerExpPoint() + Exp ;
	}

	SetPlayerExpPoint(NewExp);

	MSG_DWORDEX2 msg ;

	msg.Category	= MP_USERCONN ;
	msg.Protocol	= MP_USERCONN_CHARACTER_APPLYEXP_NOTICE ;
	msg.dwObjectID	= GetID() ;
	msg.dweData1	= GetPlayerExpPoint() ;

	// 090213 LYW --- Player : �йи� ��� ���ӿ� ���� �߰� ����ġ�� �����Ѵ�. ( ���ۿ� ����ġ ���� )
	if( GetFamilyIdx() )
	{
		msg.dweData2	= Exp + m_dweFamilyRewardExp ;
	}
	else
	{
		msg.dweData2	= Exp ;
	}

	SendMsg(&msg, sizeof(msg)) ;
}

void CPlayer::ReduceExpPoint(EXPTYPE minusExp, BYTE bLogType)
{
	LEVELTYPE minuslevel = 0;

	// 080602 LYW --- Player : ����ġ ��ġ (__int32) ���� (__int64) ������� ���� ó��.
	//DWORD CurExp = GetPlayerExpPoint();
	EXPTYPE CurExp = GetPlayerExpPoint();

	if(GetLevel() <= 1 && CurExp < minusExp)	//��������1A�� 0��iAo���� ��iAI��U.
		minusExp = CurExp;

	InsertLogExp( bLogType, GetID(), GetLevel(), minusExp, GetPlayerExpPoint(), m_MurdererKind, m_MurdererIDX, 0/*�����Ƽ ���� - GetPlayerAbilityExpPoint()*/);
	
	while(1)
	{
		if(CurExp < minusExp)
		{
			minusExp -= CurExp;
			++minuslevel;
			CurExp = GAMERESRCMNGR->GetMaxExpPoint(GetLevel()-minuslevel) - 1;
			ASSERT(minuslevel<2);	//E����A���� C������
			if(minuslevel > 3)		//E����A���� CI��A... ����CN��cCA ����Ao��e
				break;
		}
		else
		{
			CurExp -= minusExp;
			break;
		}
	}

	if(minuslevel > 0)
	{
		SetLevel(m_HeroCharacterInfo.Level-minuslevel);

		// character info log
		InsertLogCharacter( GetID(), m_HeroCharacterInfo.Level, &m_HeroInfo );
	}

	SetPlayerExpPoint(CurExp);
}

BYTE CPlayer::GetLifePercent()
{
	return BYTE(GetLife() / (float)GetMaxLife() * 100);
}

BYTE CPlayer::GetManaPercent()
{
	return BYTE(GetMana() / (float)GetMaxMana() * 100);
}

void CPlayer::OnEndObjectState(EObjectState State)
{
	switch(State)
	{
	case eObjectState_Die:
		GetBattle()->OnTeamMemberRevive(GetBattleTeam(),this);
		m_bNeedRevive = TRUE;
		break;
	}

}

// 090204 LUJ, Ÿ���� ��Ȯ�� ��
eWeaponType CPlayer::GetWeaponEquipType()
{
	const ITEM_INFO* const pItemInfo = ITEMMGR->GetItemInfo( GetWearedWeapon() );

	return pItemInfo ? eWeaponType( pItemInfo->WeaponType ) : eWeaponType_None;
}

// 080703 LUJ, ��ȯ Ÿ���� enum���� ����
eWeaponAnimationType CPlayer::GetWeaponAniType()
{
	const ITEM_INFO* leftInfo	= ITEMMGR->GetItemInfo( GetWearedItemIdx( eWearedItem_Weapon ) );
	const ITEM_INFO* rightInfo	= ITEMMGR->GetItemInfo( GetWearedItemIdx( eWearedItem_Shield ) );

	const eWeaponAnimationType	leftType	= eWeaponAnimationType( leftInfo ? leftInfo->WeaponAnimation : eWeaponAnimationType_None );
	const eWeaponAnimationType	rightType	= eWeaponAnimationType( rightInfo ? rightInfo->WeaponAnimation : eWeaponAnimationType_None );

	// 080703 LUJ, ����� ���Ⱑ �ٸ��� �̵����� �ƴϴ�. �޼տ� ���⸦ �� ����� ��쵵 ���������̴�.
	if( leftType != rightType ||
		leftType == eWeaponAnimationType_None )
	{
		return leftType;
	}

	return eWeaponAnimationType_TwoBlade;
}

void CPlayer::ReviveAfterShowdown( BOOL bSendMsg )
{
	ClearMurderIdx();
	m_bNeedRevive = TRUE;

	if( bSendMsg )
	{
		MOVE_POS msg;
		msg.Category = MP_USERCONN;
		msg.Protocol = MP_USERCONN_CHARACTER_REVIVE;
		msg.dwObjectID = GetID();
		msg.dwMoverID = GetID();
	
		msg.cpos.Compress(CCharMove::GetPosition(this));
		
		PACKEDDATA_OBJ->QuickSend(this,&msg,sizeof(msg));
	}

	OBJECTSTATEMGR_OBJ->EndObjectState(this,eObjectState_Die);

	m_YYLifeRecoverTime.bStart = FALSE;
	m_YYManaRecoverTime.bStart = FALSE;
	//aziz revive 100% HP after Battle Showdown
	SetLife( GetMaxLife() * 100 / 100 );	//��uoA�ˢ� ��ui��O��i����O CO����i.
}
void CPlayer::ReviveAfterShowdownHPFULL( BOOL bSendMsg )
{
	ClearMurderIdx();
	m_bNeedRevive = TRUE;

	if( bSendMsg )
	{
		MOVE_POS msg;
		msg.Category = MP_USERCONN;
		msg.Protocol = MP_USERCONN_CHARACTER_REVIVE;
		msg.dwObjectID = GetID();
		msg.dwMoverID = GetID();
	
		msg.cpos.Compress(CCharMove::GetPosition(this));
		
		PACKEDDATA_OBJ->QuickSend(this,&msg,sizeof(msg));
	}

	OBJECTSTATEMGR_OBJ->EndObjectState(this,eObjectState_Die);

	m_YYLifeRecoverTime.bStart = FALSE;
	m_YYManaRecoverTime.bStart = FALSE;
	//ICE01 FULLHP BATTLE
	SetLife(GetMaxLife());	//��uoA�ˢ� ��ui��O��i����O CO����i.
	SetMana(GetMaxMana());
}
	
// 080602 LYW --- Player : ����ġ ��ġ (__int32) ���� (__int64) ������� ���� ó��.
//DWORD CPlayer::RevivePenalty(BOOL bAdditionPenalty)								// ���ڸ� ��Ȱ�� �߰� ����ġ �϶��� ó���ϴ� �Լ�.
EXPTYPE CPlayer::RevivePenalty(BOOL bAdditionPenalty)								// ���ڸ� ��Ȱ�� �߰� ����ġ �϶��� ó���ϴ� �Լ�.
{
	// desc_hseos_����Ʈ ��_01
	// S ����Ʈ �� �߰� added by hseos 2007.11.30
	// ..ç���� ������ �׾ �ʾƿ��� ���� ���Ƽ ����
	if (g_csDateManager.IsChallengeZoneHere())
	{
		return FALSE;
	}
	// E ����Ʈ �� �߰� added by hseos 2007.11.30

	// ��Ʈ ��ʸ�Ʈ�� ������� �г�Ƽ ����.
	//aziz remove pvp penalty
	if(g_pServerSystem->GetMapNum()==PVP)
	{
		return FALSE;
	}
	if( g_pServerSystem->GetMapNum() == GTMAPNUM)
	{
		return FALSE;
	}

	DWORD PenaltyNum = 0 ;														// �߰� ����ġ �϶��� 2%�� �����Ѵ�.

	if( bAdditionPenalty )
	{
		PenaltyNum = 1 ;
	}
	else
	{
		PenaltyNum = random(1, 3) ;
	}
	
	// 071217 KTH --- Status�� ProtectExp�� ȿ���� �����ϸ� ����ġ�� ���� ��Ű�� �ʴ´�.//
	Status* pStatus;
	pStatus = this->GetBuffStatus();

	if( pStatus->IsProtectExp )
	{
		return 0;
	}
	//////////////////////////////////////////////////////////////////////////////////////

	LEVELTYPE CurLevel = GetLevel() ;											// �÷��̾��� ���� ������ ���Ѵ�.

	EXPTYPE CurExp	= GetPlayerExpPoint() ;										// �÷��̾��� ���� ����ġ�� ���Ѵ�.
	
	EXPTYPE GoalExp	= GAMERESRCMNGR->GetMaxExpPoint(CurLevel) ;					// �÷��̾��� ������ ����ġ�� ���Ѵ�. 

	//---KES CHECK : GoalExp�� �ſ� ū ���̴�. * PeanltyNum�� ���� ��� DWORD�� �Ѿ �� �ִ�.
	// 080602 LYW --- Player : ����ġ ��ġ (__int32) ���� (__int64) ������� ���� ó��.
	//DWORD dwExpA = GoalExp * PenaltyNum ;										// �г�Ƽ ��ġ�� ���Ѵ�.
	EXPTYPE dwExpA = GoalExp * PenaltyNum ;										// �г�Ƽ ��ġ�� ���Ѵ�.
	
	EXPTYPE PenaltyExp = (EXPTYPE)(dwExpA / 100) ;								// �г�Ƽ ����ġ�� ���Ѵ�.

	// 080602 LYW --- Player : ����ġ ��ġ (__int32) ���� (__int64) ������� ���� ó��.
	//DWORD dwExp = 0 ;															// ������ ����ġ ������ �����ϰ� 0���� �����Ѵ�.
	EXPTYPE dwExp = 0 ;															// ������ ����ġ ������ �����ϰ� 0���� �����Ѵ�.

	BOOL bLevelDown = FALSE ;													// ���� �ٿ�� ������ �����ϰ� FALSE ������ �Ѵ�.

	if( CurExp >= PenaltyExp )													// ���� ����ġ�� �г�Ƽ ����ġ �̻��� ���.
	{
		dwExp = CurExp - PenaltyExp ;											// ������ ����ġ�� �����Ѵ�.

		ASSERT( dwExp >= 0 ) ;													// ����ġ�� 0�̻��̾�߸� �Ѵ�.

		SetPlayerExpPoint( dwExp ) ;

		MSG_DWORDEX3 msg ;

		msg.Category	= MP_USERCONN ;											// ī�װ��� MP_USERCONN�� �����Ѵ�.
		msg.Protocol	= MP_USERCONN_CHARACTER_DOWNEXP_NOTICE ;				// ���������� ����ġ �϶����� �����Ѵ�.
		msg.dwObjectID	= GetID() ;												// �÷��̾��� ���̵� �����Ѵ�.
		msg.dweData1	= (DWORD)PenaltyNum ;									// �г�Ƽ ��ġ�� �����Ѵ�.
		msg.dweData2	= dwExp ;												// ������ ����ġ�� �����Ѵ�.

		if( bAdditionPenalty )
		{
			msg.dweData3		= TRUE ;												// �߰� ����ġ �϶� ���θ� TRUE�� �����Ѵ�.
		}
		else
		{
			msg.dweData3		= FALSE ;												// �߰� ����ġ �϶� ���θ� FALSE�� �����Ѵ�.
		}

		SendMsg(&msg, sizeof(msg)) ;											// �÷��̾�� �޽����� �����Ѵ�.
	}
	else																		// ���� �÷��̾��� ����ġ�� �г�Ƽ ����ġ���� �������.
	{
		bLevelDown = TRUE ;														// ���� �ٿ� ���θ� TRUE�� �����Ѵ�.

		dwExp = PenaltyExp - CurExp ;											// ������ ����ġ�� �����Ѵ�.

		ASSERT( dwExp >= 0 ) ;													// ����ġ�� 0�̻��̾�߸� �Ѵ�.

		GoalExp = GAMERESRCMNGR->GetMaxExpPoint(CurLevel-1) ;					// �Ѵܰ� ���� ������ ������ ��ǥ ����ġ�� ���Ѵ�.
		SetLevel( CurLevel -1 ) ;												// �÷��̾��� ������ �ٿ��� ������ �����Ѵ�.
		SetPlayerExpPoint(GoalExp-dwExp) ;										// �÷��̾��� ����ġ�� �����Ѵ�.

		MSG_DWORDEX4 msg ;

		msg.Category	= MP_USERCONN ;											// ī�װ��� MP_USERCONN�� �����Ѵ�.
		msg.Protocol	= MP_USERCONN_CHARACTER_DOWNLEVEL_NOTICE ;				// ���������� ����ġ �϶����� �����Ѵ�.
		msg.dwObjectID	= GetID() ;												// �÷��̾��� ���̵� �����Ѵ�.
		msg.dweData1	= (DWORDEX)GetLevel() ;									// �÷��̾��� ������ �����Ѵ�.
		msg.dweData2	= GoalExp-dwExp ;										// �÷��̾��� ����ġ�� �����Ѵ�.
		msg.dweData3	= (DWORDEX)PenaltyNum ;									// �г�Ƽ ��ġ�� �����Ѵ�.


		if( bAdditionPenalty )
		{
			msg.dweData4		= TRUE ;												// �߰� ����ġ �϶� ���θ� TRUE�� �����Ѵ�.
		}
		else
		{
			msg.dweData4		= FALSE ;												// �߰� ����ġ �϶� ���θ� FALSE�� �����Ѵ�.
		}

		SendMsg(&msg, sizeof(msg)) ;											// �÷��̾�� �޽����� �����Ѵ�.
	}

	// 080414 LUJ, ����ġ �ս� �� �սǵ� ����ġ ��ŭ �α׸� �����
	InsertLogExp(
		eExpLog_LosebyRevivePresent,
		GetID(),
		GetLevel(),
		PenaltyExp,
		dwExp,
		0,
		0,
		0 );

	return PenaltyExp;
}


void CPlayer::RevivePresentSpot()
{	
	//aziz revive Quick in PVP//Beyond SEA2
	if(g_pServerSystem->GetMapNum()==PVP && GetState() == eObjectState_Die)
	{
		ClearMurderIdx();
		m_bNeedRevive = TRUE;

		MOVE_POS msg;
		msg.Category = MP_USERCONN;
		msg.Protocol = MP_USERCONN_CHARACTER_REVIVE;
		msg.dwObjectID = GetID();
		msg.dwMoverID = GetID();

		msg.cpos.Compress(CCharMove::GetPosition(this));

		PACKEDDATA_OBJ->QuickSend(this,&msg,sizeof(msg));

		OBJECTSTATEMGR_OBJ->EndObjectState(this,eObjectState_Die);

		m_YYLifeRecoverTime.bStart = FALSE;
		m_YYManaRecoverTime.bStart = FALSE;
		SetLife( GetMaxLife() * 100 / 100 );//test
		SetMana( GetMaxMana() * 100 / 100 );
	}
	if(GetState() != eObjectState_Die)
	{
		ASSERT(0);
		MSG_BYTE msg;
		msg.Category	= MP_USERCONN;
		msg.Protocol	= MP_USERCONN_CHARACTER_REVIVE_NACK;
		msg.bData		= 1; //errorcode
		SendMsg( &msg, sizeof(msg) );

		return;
	}

	if( LOOTINGMGR->IsLootedPlayer( GetID() ) )	//����c����AA�ϡ���?��I
	{
		MSG_BYTE msg;
		msg.Category	= MP_USERCONN;
		msg.Protocol	= MP_USERCONN_CHARACTER_REVIVE_NACK;
		msg.bData		= 2; //errorcode
		SendMsg( &msg, sizeof(msg) );

		return;
	}

	if( IsExitStart() )
	{
		MSG_BYTE msg;
		msg.Category	= MP_USERCONN;
		msg.Protocol	= MP_USERCONN_CHARACTER_REVIVE_NACK;
		msg.bData		= 4; //errorcode
		SendMsg( &msg, sizeof(msg) );

		return;
	}
	// 100111 LUJ, ��Ȱ �÷��׿� ���� ��Ȱ �Ұ����� ���� �ִ�
	else if(ReviveFlagTown == mReviveFlag)
	{
		MSG_BYTE message;
		ZeroMemory(
			&message,
			sizeof(message));
		message.Category = MP_USERCONN;
		message.Protocol = MP_USERCONN_CHARACTER_REVIVE_NACK;
		message.bData = 3;
		SendMsg(
			&message,
			sizeof(message));
		return;
	}

	m_bNeedRevive = TRUE;
	MOVE_POS msg;
	msg.Category = MP_USERCONN;
	msg.Protocol = MP_USERCONN_CHARACTER_REVIVE;
	msg.dwObjectID = GetID();
	msg.dwMoverID = GetID();
	
	msg.cpos.Compress(CCharMove::GetPosition(this));
		
	PACKEDDATA_OBJ->QuickSend(this,&msg,sizeof(msg));

	LEVELTYPE curLevel = GetLevel() ;

	if( curLevel >= 10 && m_bNoExpPenaltyByPK == FALSE )
	{
		RevivePenalty(FALSE) ;
		RevivePenalty(TRUE) ;
	
		if( !g_csDateManager.IsChallengeZoneHere() )
		{
			// 090204 LUJ, ���� ȸ���� ������
			RemoveBuffCount( eBuffSkillCountType_Dead, 1 );
		}
	}

	OBJECTSTATEMGR_OBJ->EndObjectState(this,eObjectState_Die);
	
	DWORD MaxLife = GetMaxLife();
	DWORD MaxMana = GetMaxMana();

	int nReviveVal = (int)(MaxLife*0.3) ;

	MSG_INT ReviveLife;
	ReviveLife.Category = MP_CHAR;
	ReviveLife.Protocol = MP_CHAR_LIFE_ACK;
	ReviveLife.dwObjectID = GetID();
	ReviveLife.nData = max(1, nReviveVal);
	SendMsg(&ReviveLife,sizeof(ReviveLife));			
		
	SendLifeToParty(
		nReviveVal);
		
	m_HeroCharacterInfo.Life = nReviveVal;

	// 070417 LYW --- Player : Modified setting mana when the character revived.
	DWORD dwManaRate = (DWORD)(MaxMana*0.3) ;
	if( GetMana() < dwManaRate )
	{
		SetMana(dwManaRate);
	}
	
	m_YYLifeRecoverTime.bStart = FALSE;
	m_YYManaRecoverTime.bStart = FALSE;
	ClearMurderIdx();

	m_bDieForGFW = FALSE;
// --- skr  12/01/2020
   SetRelifeStart();
 
}

// 080602 LYW --- Player : ����ġ ��ġ (__int32) ���� (__int64) ������� ���� ó��.
//DWORD CPlayer::ReviveBySkill()
void CPlayer::ReviveBySkill( cSkillObject* pSkillObject )
{	
	if( !pSkillObject )
		return;

	if(GetState() != eObjectState_Die)
	{
		ASSERT(0);
		MSG_BYTE msg;
		msg.Category	= MP_USERCONN;
		msg.Protocol	= MP_USERCONN_CHARACTER_REVIVE_NACK;
		msg.bData		= 1; //errorcode
		SendMsg( &msg, sizeof(msg) );

		return;
	}

	if( LOOTINGMGR->IsLootedPlayer( GetID() ) )	//����c����AA�ϡ���?��I
	{
		MSG_BYTE msg;
		msg.Category	= MP_USERCONN;
		msg.Protocol	= MP_USERCONN_CHARACTER_REVIVE_NACK;
		msg.bData		= 2; //errorcode
		SendMsg( &msg, sizeof(msg) );

		return;
	}

	if( IsExitStart() )
	{
		MSG_BYTE msg;
		msg.Category	= MP_USERCONN;
		msg.Protocol	= MP_USERCONN_CHARACTER_REVIVE_NACK;
		msg.bData		= 4; //errorcode
		SendMsg( &msg, sizeof(msg) );

		return;
	}

	// 100211 ONS ��Ȱ��󿡰� ��Ȱ���θ� ���´�.
	// ��Ȱ��ų�� �����Ѵ�.
	SetCurResurrectIndex( pSkillObject->GetSkillIdx() );
	
	// ��ųOperator�̸��� �����Ѵ�.
	CObject* pOperator = pSkillObject->GetOperator();
	if( !pOperator || 
		pOperator->GetObjectKind() != eObjectKind_Player )
	{
		return;
	}

	MSG_NAME msg;
	ZeroMemory(&msg, sizeof(MSG_NAME));
	msg.Category	= MP_SKILL;
	msg.Protocol	= MP_SKILL_RESURRECT_SYN;
	msg.dwObjectID	= GetID();
	SafeStrCpy(msg.Name, pOperator->GetObjectName(), MAX_NAME_LENGTH+1);
	SendMsg( &msg, sizeof(msg) );
}

// 100211 ONS ��Ȱ����ڰ� ������ ��� ��Ȱó���� �����Ѵ�.
EXPTYPE CPlayer::OnResurrect()
{
	EXPTYPE exp = 0;
	m_bNeedRevive = TRUE;

	MOVE_POS msg;
	msg.Category = MP_USERCONN;
	msg.Protocol = MP_USERCONN_CHARACTER_REVIVE;
	msg.dwObjectID = GetID();
	msg.dwMoverID = GetID();
	
	msg.cpos.Compress(CCharMove::GetPosition(this));
		
	PACKEDDATA_OBJ->QuickSend(this,&msg,sizeof(msg));

	LEVELTYPE curLevel = GetLevel() ;

	if( curLevel >= 10 && m_bNoExpPenaltyByPK == FALSE )
	{
		exp = RevivePenalty(FALSE) ;

		if( !g_csDateManager.IsChallengeZoneHere() && g_pServerSystem->GetMapNum()!=GTMAPNUM)
		{
			// 090204 LUJ, ���� ȸ���� ������
			RemoveBuffCount( eBuffSkillCountType_Dead, 1 );
		}
	}

	OBJECTSTATEMGR_OBJ->EndObjectState(this,eObjectState_Die);
	
	DWORD MaxLife = GetMaxLife();
	DWORD MaxMana = GetMaxMana();

	int nReviveVal = (int)(MaxLife*0.3) ;

	MSG_INT ReviveLife;
	ReviveLife.Category = MP_CHAR;
	ReviveLife.Protocol = MP_CHAR_LIFE_ACK;
	ReviveLife.dwObjectID = GetID();
	ReviveLife.nData = max(1, nReviveVal);
	SendMsg(&ReviveLife,sizeof(ReviveLife));			
		
	SendLifeToParty(
		nReviveVal);
		
	m_HeroCharacterInfo.Life = nReviveVal;

	// 070417 LYW --- Player : Modified setting mana when the character revived.
	DWORD dwManaRate = (DWORD)(MaxMana*0.3) ;
	if( GetMana() < dwManaRate )
	{
		SetMana(dwManaRate);
	}
	
	m_YYLifeRecoverTime.bStart = FALSE;
	m_YYManaRecoverTime.bStart = FALSE;
	ClearMurderIdx();

	m_bDieForGFW = FALSE;
// --- skr  12/01/2020
  SetRelifeStart();
	return exp;
}

void CPlayer::ReviveLogIn()
{	
	// ĳ���Ͱ� ���� ���°� �ƴϸ�, ��Ȱ ���� ó���� �Ѵ�.
	if(GetState() != eObjectState_Die)
	{
		ASSERT(0) ;
		MSG_BYTE msg ;
		msg.Category	= MP_USERCONN ;
		msg.Protocol	= MP_USERCONN_CHARACTER_REVIVE_NACK ;
		msg.bData		= 1 ; //errorcode
		SendMsg( &msg, sizeof(msg) ) ;

		return ;
	}
	

	// ���� ���¶��, ��Ȱ ����ó���� �Ѵ�.
	if( LOOTINGMGR->IsLootedPlayer( GetID() ) )
	{
		MSG_BYTE msg ;
		msg.Category	= MP_USERCONN ;
		msg.Protocol	= MP_USERCONN_CHARACTER_REVIVE_NACK ;
		msg.bData		= 2 ; //errorcode
		SendMsg( &msg, sizeof(msg) ) ;

		return ;
	}


	// �ƿ�ó���� ���� �Ǿ�����, ���� ó���� �Ѵ�.
	if( IsExitStart() )
	{
		MSG_BYTE msg ;
		msg.Category	= MP_USERCONN ;
		msg.Protocol	= MP_USERCONN_CHARACTER_REVIVE_NACK ;
		msg.bData		= 4 ; //errorcode
		SendMsg( &msg, sizeof(msg) ) ;

		return ;
	}
	// 100111 LUJ, ��Ȱ �÷��׿� ���� ��Ȱ �Ұ����� ���� �ִ�
	else if(ReviveFlagHere == mReviveFlag)
	{
		MSG_BYTE message;
		ZeroMemory(
			&message,
			sizeof(message));
		message.Category = MP_USERCONN;
		message.Protocol = MP_USERCONN_CHARACTER_REVIVE_NACK;
		message.bData = 3;
		SendMsg(
			&message,
			sizeof(message));
		return;
	}

	// ���� ��� �������� Ȯ���Ѵ�.
	if( SIEGEDUNGEONMGR->IsSiegeDungeon(g_pServerSystem->GetMapNum()) )
	{
		ReviveLogIn_GuildDungeon() ;
	}
	else
	{
		ReviveLogIn_Normal() ;
	}
}





// 081210 LYW --- Player : ������ ��� ���������� ��Ȱ ������ ���� �ΰ��� �Լ��� �߰��Ѵ�.
//-------------------------------------------------------------------------------------------------
//	NAME		: ReviveLogIn_Normal
//	DESC		: �Ϲ����� �������� ��Ȱ �Լ�.
//	PROGRAMMER	: Yongs Lee
//	DATE		: December 10, 2008
//-------------------------------------------------------------------------------------------------
void CPlayer::ReviveLogIn_Normal()
{
	//aziz revive safe location pvp
	if(g_pServerSystem->GetMapNum()==PVP)
	{
		ClearMurderIdx();
		m_bNeedRevive = TRUE;

		MOVE_POS msg;
		msg.Category = MP_USERCONN;
		msg.Protocol = MP_USERCONN_CHARACTER_REVIVE;
		msg.dwObjectID = GetID();
		msg.dwMoverID = GetID();

		msg.cpos.Compress(CCharMove::GetPosition(this));

		PACKEDDATA_OBJ->QuickSend(this,&msg,sizeof(msg));

		OBJECTSTATEMGR_OBJ->EndObjectState(this,eObjectState_Die);

		m_YYLifeRecoverTime.bStart = FALSE;
		m_YYManaRecoverTime.bStart = FALSE;
		SetLife( GetMaxLife() * 100 / 100 );//test
		SetMana( GetMaxMana() * 100 / 100 );
	}
	// ��Ȱâ�� �ʿ��ϴٰ� �����Ѵ�.
	m_bNeedRevive = TRUE ;


	// ��Ȱ �޽����� �����Ѵ�.
	MOVE_POS msg ;

	msg.Category	= MP_USERCONN ;
	msg.Protocol	= MP_USERCONN_CHARACTER_REVIVE ;

	msg.dwObjectID	= GetID() ;
	msg.dwMoverID	= GetID() ;


	// ��Ȱ ��ġ�� �����Ѵ�.
	VECTOR3* ppos ;
	VECTOR3 pos ;

	if(g_pServerSystem->GetMapNum() == GTMAPNUM)
	{
		const DWORD dwTeam = GetBattle()->GetBattleTeamID( this );
		ppos = GetBattle()->GetRevivePoint(dwTeam);
	}
	else
	{
		ppos = GAMERESRCMNGR->GetRevivePoint() ;
	}

	int temp ;
	temp	= rand() % 500 - 250 ;
	pos.x	= ppos->x + temp ;
	temp	= rand() % 500 - 250 ;
	pos.z	= ppos->z + temp ;
	pos.y	= 0 ;

	msg.cpos.Compress(&pos) ;
	
	CCharMove::SetPosition(this,&pos) ;

	PACKEDDATA_OBJ->QuickSend(this,&msg,sizeof(msg)) ;
		

	// Player�� ���� ���¸� �����Ѵ�.
	OBJECTSTATEMGR_OBJ->EndObjectState(this,eObjectState_Die) ;


	// ��Ȱ �г�Ƽ�� �����Ѵ�.
	const LEVELTYPE curLevel = GetLevel() ;
	
	if(	curLevel >= 10 && !m_bDieForGFW && m_bNoExpPenaltyByPK == FALSE )
	{
		RevivePenalty(FALSE) ;
		
		if( !g_csDateManager.IsChallengeZoneHere() && g_pServerSystem->GetMapNum()!=GTMAPNUM )
		{
			// 090204 LUJ, ���� ȸ���� ������
            RemoveBuffCount( eBuffSkillCountType_Dead, 1 );
		}
	}


	// ��� ��ʸ�Ʈ ���� ó���� �Ѵ�.
	m_bDieForGFW = FALSE ;
	m_dwRespawnTimeOnGTMAP = 0 ;
	m_dwImmortalTimeOnGTMAP = 0 ;
	

	// ��Ȳ�� ���� ������� �����Ѵ�.
	DWORD CurLife = GetMaxLife() ;

	int nReviveVal = 0 ;
	if(g_pServerSystem->GetMapNum() == GTMAPNUM)
		nReviveVal = (int)(CurLife*1.0) ;
	else
		nReviveVal = (int)(CurLife*0.3) ;

	MSG_INT ReviveLife ;
	ReviveLife.Category = MP_CHAR ;
	ReviveLife.Protocol = MP_CHAR_LIFE_ACK ;
	ReviveLife.dwObjectID = GetID() ;
	ReviveLife.nData = max(1, nReviveVal);
	SendMsg(&ReviveLife,sizeof(ReviveLife)) ;

	m_HeroCharacterInfo.Life = nReviveVal ;


	// ��Ȳ�� ���� �������� �����Ѵ�.
	DWORD MaxMana = GetMaxMana() ;

	DWORD dwManaRate = 0 ;
	if(g_pServerSystem->GetMapNum() == GTMAPNUM)
		SetMana(MaxMana) ;
	else
	{
		dwManaRate = (DWORD)(MaxMana*0.3) ;
		if( GetMana() < dwManaRate )
		{
			SetMana(dwManaRate) ;
		}
	}


	// ��� ��ʸ�Ʈ ���ܸ� ó���Ѵ�.
	if(g_pServerSystem->GetMapNum() == GTMAPNUM)
	{
		WORD wCode = GetJobCodeForGT() ;
		m_dwImmortalTimeOnGTMAP = GTMGR->GetImmortalTimeByClass(wCode) ;
	}


	// �������� ó���� �Ѵ�.
	OBJECTSTATEMGR_OBJ->StartObjectState(this,eObjectState_Immortal,0) ;
	// 06.08.29. RaMa.
	OBJECTSTATEMGR_OBJ->EndObjectState( this, eObjectState_Immortal, 30000 ) ;
	
	m_YYLifeRecoverTime.bStart = FALSE ;
	m_YYManaRecoverTime.bStart = FALSE ;

	if(CParty* const pParty = PARTYMGR->GetParty(m_HeroInfo.PartyID))
	{
		SEND_PARTYICON_REVIVE msg;
		ZeroMemory(
			&msg,
			sizeof(msg));
		msg.Category = MP_PARTY;
		msg.Protocol = MP_PARTY_REVIVEPOS;
		msg.dwMoverID = m_BaseObjectInfo.dwObjectID;
		msg.Pos.posX = (WORD)pos.x;
		msg.Pos.posZ = (WORD)pos.z;

		pParty->SendMsgToAll(
			&msg,
			sizeof(msg));
		SendLifeToParty(
			nReviveVal);
	}

	ClearMurderIdx();

// --- skr  12/01/2020
  SetRelifeStart();

}





//-------------------------------------------------------------------------------------------------
//	NAME		: ReviveLogIn_GuildDungeon
//	DESC		: ���� ������������ �������� ��Ȱ �Լ�.
//	PROGRAMMER	: Yongs Lee
//	DATE		: December 10, 2008
//-------------------------------------------------------------------------------------------------
void CPlayer::ReviveLogIn_GuildDungeon()
{
	BYTE byCheckRevivePoint = TRUE ;

	// ��Ȱâ�� �ʿ��ϴٰ� �����Ѵ�.
	m_bNeedRevive = TRUE ;


	// ��Ȱ �޽����� �����Ѵ�.
	MOVE_POS msg ;

	msg.Category	= MP_USERCONN ;
	msg.Protocol	= MP_USERCONN_CHARACTER_REVIVE ;

	msg.dwObjectID	= GetID() ;
	msg.dwMoverID	= GetID() ;

	
	// Player�� �罨�� ��� �Ҽ����� ���Ἲ ��� �Ҽ����� Ȯ���Ѵ�.
	VillageWarp* pRevivePoint	= NULL ;

	DWORD dwGuildID				= GetGuildIdx() ;
	if( dwGuildID == 0 )
	{
		char szMsg[512] = {0, } ;
		sprintf( szMsg, "This player is not in guild! - %s,%d", GetObjectName(), GetID() ) ;
		SIEGEWARFAREMGR->WriteLog(szMsg) ;

		// 081217 LYW --- Player : ���� ��� �������� ��� �Ҽ� ���� �����, ĳ���� ���� ȭ������ �̵��ϴ� ó�� �߰�.
		MSGBASE msg ;

		msg.Category	= MP_SIEGEWARFARE ;
		msg.Protocol	= MP_SIEGEWARFARE_DIEINGUILDDUNGEON_NOTICE ;

		msg.dwObjectID	= GetID() ;

		SendMsg( &msg, sizeof(MSGBASE) ) ;

		byCheckRevivePoint = FALSE ;
	}

	DWORD dwGuildID_Rushen		= SIEGEWARFAREMGR->GetCastleGuildIdx(eNeraCastle_Lusen) ;
	DWORD dwGuildID_Zevyn		= SIEGEWARFAREMGR->GetCastleGuildIdx(eNeraCastle_Zebin) ;

	// �罨�� �����,
	if( dwGuildID_Rushen == dwGuildID )
	{
		pRevivePoint = SIEGEWARFAREMGR->GetDGRP_Rushen() ;
	}
	// ���Ἲ �����,
	else
	{
		if( dwGuildID_Zevyn == dwGuildID )
		{
			pRevivePoint = SIEGEWARFAREMGR->GetDGRP_Zevyn() ;
		}
		else
		{
			char szMsg[512] = {0, } ;
			sprintf( szMsg, "Invalid guild idx! \n PLAYER_GUILD:%d / RUSHEN_GUILD:%d / ZEVYN_GUILD:%d", 
			dwGuildID, dwGuildID_Rushen, dwGuildID_Zevyn ) ;
			SIEGEWARFAREMGR->WriteLog(szMsg) ;

			// 081217 LYW --- Player : ���� ��� �������� ��� �Ҽ� ���� �����, ĳ���� ���� ȭ������ �̵��ϴ� ó�� �߰�.
			MSGBASE msg ;

			msg.Category	= MP_SIEGEWARFARE ;
			msg.Protocol	= MP_SIEGEWARFARE_DIEINGUILDDUNGEON_NOTICE ;

			msg.dwObjectID	= GetID() ;

			SendMsg( &msg, sizeof(MSGBASE) ) ;

			byCheckRevivePoint = FALSE ;
		}
	}


	// ��Ȱ ��ġ�� �����Ѵ�.
	VECTOR3 pos ;
	int temp ;

	if( byCheckRevivePoint )
	{
		if( !pRevivePoint )
		{
			char szMsg[512] = {0, } ;
			sprintf( szMsg, "Failed to receive dungeon revive point! \n PLAYER_GUILD:%d / RUSHEN_GUILD:%d / ZEVYN_GUILD:%d", 
				dwGuildID, dwGuildID_Rushen, dwGuildID_Zevyn ) ;
			SIEGEWARFAREMGR->WriteLog(szMsg) ;
			return ;
		}

		temp	= rand() % 100 ;
		pos.x	= (pRevivePoint->PosX*100) + temp ;
		temp	= rand() % 100 ;
		pos.z	= (pRevivePoint->PosZ*100) + temp ;
		pos.y	= 0 ;
	}
	else
	{
		pos = *CCharMove::GetPosition(this) ;
	}

	msg.cpos.Compress(&pos) ;
	
	CCharMove::SetPosition(this,&pos) ;

	PACKEDDATA_OBJ->QuickSend(this,&msg,sizeof(msg)) ;
		

	// Player�� ���� ���¸� �����Ѵ�.
	OBJECTSTATEMGR_OBJ->EndObjectState(this,eObjectState_Die) ;


	// ��Ȱ �г�Ƽ�� �����Ѵ�.
	const LEVELTYPE curLevel = GetLevel() ;
	
	if(	curLevel >= 10 && !m_bDieForGFW && m_bNoExpPenaltyByPK == FALSE )
	{
		RevivePenalty(FALSE) ;
		
		if( !g_csDateManager.IsChallengeZoneHere() && g_pServerSystem->GetMapNum()!=GTMAPNUM )
		{
			// 090204 LUJ, ���� ȸ���� ������
            RemoveBuffCount( eBuffSkillCountType_Dead, 1 );
		}
	}


	// ��Ȳ�� ���� ������� �����Ѵ�.
	DWORD CurLife = GetMaxLife() ;
	int nReviveVal = (int)(CurLife*0.3) ;

	MSG_INT ReviveLife ;
	ReviveLife.Category = MP_CHAR ;
	ReviveLife.Protocol = MP_CHAR_LIFE_ACK ;
	ReviveLife.dwObjectID = GetID() ;
	ReviveLife.nData = max(1, nReviveVal);
	SendMsg(&ReviveLife,sizeof(ReviveLife)) ;

	m_HeroCharacterInfo.Life = nReviveVal ;


	// ��Ȳ�� ���� �������� �����Ѵ�.
	DWORD MaxMana = GetMaxMana() ;

	DWORD dwManaRate = (DWORD)(MaxMana*0.3) ;
	if( GetMana() < dwManaRate )
	{
		SetMana(dwManaRate) ;
	}


	// �������� ó���� �Ѵ�.
	OBJECTSTATEMGR_OBJ->StartObjectState(this,eObjectState_Immortal,0) ;
	OBJECTSTATEMGR_OBJ->EndObjectState( this, eObjectState_Immortal, 30000 ) ;
	
	m_YYLifeRecoverTime.bStart = FALSE ;
	m_YYManaRecoverTime.bStart = FALSE ;

	if(CParty* const pParty = PARTYMGR->GetParty(m_HeroInfo.PartyID))
	{
		SEND_PARTYICON_REVIVE msg;
		ZeroMemory(
			&msg,
			sizeof(msg));
		msg.Category = MP_PARTY;
		msg.Protocol = MP_PARTY_REVIVEPOS;
		msg.dwMoverID = m_BaseObjectInfo.dwObjectID;
		msg.Pos.posX = (WORD)pos.x;
		msg.Pos.posZ = (WORD)pos.z;

		pParty->SendMsgToAll(
			&msg,
			sizeof(msg));
		SendLifeToParty(
			nReviveVal);
	}

	ClearMurderIdx();

// --- skr  12/01/2020
  SetRelifeStart();
}





void CPlayer::ReviveLogInPenelty()
{
	if( GetLevel() >= 10 && m_bNoExpPenaltyByPK == FALSE )
	{
		RevivePenalty(FALSE) ;
		RevivePenalty(TRUE) ;
		
		if( !g_csDateManager.IsChallengeZoneHere() )
		{
			// 090204 LUJ, ���� ȸ���� ����?			RemoveBuffCount( eBuffSkillCountType_Dead, 1 );
		}
	}

	DWORD CurLife = GetMaxLife();
	DWORD CurMana = GetMaxMana();

	// 080625 LYW --- Player : ����� ������ �϶�� �ϳ�, ĳ���Ͱ� ���� �����̱� ������, 
	// ����� ���� �Լ��� ����� ����� ���� �ʴ´�. ������, ������ ������� �����ϴ� 
	// �Լ��� �����Ѵ�.
	//SetLife((DWORD)(CurLife*0.3));
	//SetMana(0);

	// ����� ����.
	DWORD dwNewLife = 0 ;
	// 080710 LYW --- Player : ����� ȸ�� 50%�� ����.
	//dwNewLife = (DWORD)(CurLife * 0.3f) ;
	dwNewLife = (DWORD)(CurLife * 0.5f) ;

	SetLifeForce(dwNewLife, TRUE) ;

	// ���� ����.
	DWORD dwNewMana = 0 ;
	// 080710 LYW --- Player : ���� ȸ�� 50%�� ����.
	//dwNewMana = (DWORD)(CurMana* 0.3f) ;
	dwNewMana = (DWORD)(CurMana* 0.5f) ;

	SetManaForce(dwNewMana, TRUE) ;

	m_bDieForGFW = FALSE;
}


void CPlayer::DoDie(CObject* pAttacker)
{
	OBJECTSTATEMGR_OBJ->StartObjectState(this,eObjectState_Die,pAttacker->GetID());

	m_bNoExpPenaltyByPK = FALSE;

	if( pAttacker->GetObjectKind() == eObjectKind_Pet )
	{
		CObject* const object = g_pUserTable->FindUser(
			pAttacker->GetOwnerIndex());

		if(0 == object)
		{
			return;
		}

		pAttacker = object;
	}

	if(pAttacker->GetObjectKind() == eObjectKind_Player)
	{
		CPlayer* pAttackPlayer = (CPlayer*)pAttacker;
		CBattle* pBattle = pAttacker->GetBattle();
		if(pBattle->GetBattleKind() == eBATTLE_KIND_NONE || pAttacker->GetBattleID() != GetBattleID() )
		{
			// for pk
			if(STREETTOURNAMENTMGR->isStreetTournament() && STREETTOURNAMENTMGR->GetState() == 3)
			{
				//update rank on db
				//STUpdateRank(GetID(), m_STrank);
				SetRegisterStreetTournament(FALSE);
				m_bNoExpPenaltyByPK = TRUE;
				SetReadyToRevive(TRUE);
				SetDieForGFW( TRUE );

			}
			if(g_pServerSystem->GetMapNum() == PVP)
			{
				SetReadyToRevive(TRUE);
			}
			int randomLootPercent = 10;
			int randomNum = rand() % 100;
			if (randomNum < randomLootPercent && g_pServerSystem->GetMapNum()!=PVP)
			{
				if( LOOTINGMGR->IsLootingSituation( this, pAttackPlayer ) )
				{
					LOOTINGMGR->CreateLootingRoom( this, (CPlayer*)pAttacker );
				}
			}

			if(pAttackPlayer->IsPKMode() == TRUE)
			{
				m_MurdererIDX = pAttacker->GetID();
				m_MurdererKind = WORD(pAttacker->GetObjectKind());
				m_bNoExpPenaltyByPK = TRUE;
			}
			else if( GUILDWARMGR->JudgeGuildWar( this, (CPlayer*)pAttacker ) &&
				! m_bDieForGFW )
			{
				SetDieForGFW( TRUE );

				const CGuildManager::ScoreSetting& setting = GUILDMGR->GetScoreSetting();

				GuildUpdateScore( pAttackPlayer, this, setting.mKillerScore, setting.mCorpseScore );
			}

			SetReadyToRevive(TRUE);

		}
	}
	else if(pAttacker->GetObjectKind() & eObjectKind_Monster )
	{
		if(STREETTOURNAMENTMGR->isStreetTournament() && STREETTOURNAMENTMGR->GetState() == 3)
		{
				//update rank on db
				//STUpdateRank(GetID(), m_STrank);
				SetRegisterStreetTournament(FALSE);
				m_bNoExpPenaltyByPK = TRUE;
				SetReadyToRevive(TRUE);
				SetDieForGFW( TRUE );
		}
		m_MurdererKind = ((CMonster*)pAttacker)->GetMonsterKind();
			
		// 080616 LUJ, ������ ��쿡�� ���Ƽ�� ���� �ʵ��� �Ѵ�
		if( pAttacker->GetObjectKind() != eObjectKind_Trap )
		{
			//SW060831 ���� �� �� //���� ����� üũ ����
			SetPenaltyByDie(TRUE);
		}

		SetReadyToRevive(TRUE);

		// add quest event
		CQuestEvent QEvent( eQuestEvent_Die, g_pServerSystem->GetMapNum(), 0 );
		QUESTMGR->AddQuestEvent( this, &QEvent, 0 );
	}

	//KES EXIT
	ExitCancel();
	//KES EXCHANGE 031002
	EXCHANGEMGR->CancelExchange( this );
	STREETSTALLMGR->UserLogOut( this );
	//KES PKLOOTING
	LOOTINGMGR->LootingCancel( this );
	//KES PKPlayerPanelty
	PKMGR->DiePanelty( this, pAttacker );

//---KES Aggro 070918
//---��׷� ����Ʈ ������
	RemoveAllAggroed();	//*����: �Ʒ� FollowMonsterList�����ϱ� ������ ���־��, ���� ��׷θ� Ÿ������ ���� �� �ִ�.
//-------------------

	CMonster * pObject = NULL;
	m_FollowMonsterList.SetPositionHead();
	while((pObject = (CMonster *)m_FollowMonsterList.GetData())!= NULL)
	{
		pObject->SetTObject(NULL);
	}
	m_FollowMonsterList.RemoveAll();

	//---KES ������ �̵��� �����־�� �Ѵ�.
	if( CCharMove::IsMoving(this) )
	{
		VECTOR3 pos;
		GetPosition( &pos );
		CCharMove::EndMove( this, gCurTime, &pos );
	}

	QUESTMAPMGR->DiePlayer( this );

	// desc_hseos_����Ʈ ��_01
	// S ����Ʈ �� �߰� added by hseos 2007.11.29
	g_csDateManager.SRV_EndChallengeZone(this, CSHDateManager::CHALLENGEZONE_END_ALL_DIE);
	// E ����Ʈ �� �߰� added by hseos 2007.11.29

	// 080725 KTH
	SIEGEWARFAREMGR->CancelWaterSeedUsing(this);

	{
		CPet* const petObject = PETMGR->GetPet(
			GetPetItemDbIndex());

		if(petObject)
		{
			petObject->DoDie(
				pAttacker);
		}
	}

	// 081020 LYW --- Player : ������ ĳ���� �����, A�� B�� �׿��ٴ� �� ���� ��ε�ĳ���� ó�� �߰�. - �۰���.
	if( SIEGEWARFAREMGR->IsSiegeWarfareZone(g_pServerSystem->GetMapNum()) )
	{
		MSG_DWORD2 msg ;

		msg.Category	= MP_SIEGEWARFARE ;
		msg.Protocol	= MP_SIEGEWARFARE_NOTICE_KILLANDKILLER_SYN ;

		msg.dwObjectID	= 0 ;

		msg.dwData1		= pAttacker->GetID() ;
		msg.dwData2		= this->GetID() ;

		g_Network.Broadcast2AgentServer((char*)&msg, sizeof(MSG_DWORD2)) ;
	}

	SetSummonedVehicle( 0 );
	SetMountedVehicle( 0 );
	
	CObject* forfindsumon = (CObject*)this;
	forfindsumon->RemoveSummonedAll();

	// 100621 ShinJS ����� ���� �������� ��ų�� ��ҽ�Ų��.
	CancelCurrentCastingSkill( FALSE );
}

float CPlayer::DoGetMoveSpeed()
{
	if( GetAbnormalStatus()->IsMoveStop )
	{
		return 0;
	}

	// 090422 ShinJS --- Ż���� Master�̰� ž������ ��� Ż���� �̵��ӵ� �̿�
	{
		CObject* const vehicleObject = g_pUserTable->FindUser( GetSummonedVehicle() );

		if( vehicleObject &&
			GetSummonedVehicle() == GetMountedVehicle() )
		{
			return vehicleObject->GetMoveSpeed();
		}
	}

	float speed = float( m_MoveInfo.MoveMode == eMoveMode_Run ? RUNSPEED : WALKSPEED );

	// 080630 LUJ, ��Ʈ ������ ��ġ�� ����ǵ��� ��
	float addrateval = ( GetRateBuffStatus()->MoveSpeed + GetRatePassiveStatus()->MoveSpeed ) / 100.f + m_itemBaseStats.mMoveSpeed.mPercent + m_itemOptionStats.mMoveSpeed.mPercent + m_SetItemStats.mMoveSpeed.mPercent;
	float addval = GetBuffStatus()->MoveSpeed + GetPassiveStatus()->MoveSpeed + m_itemBaseStats.mMoveSpeed.mPlus + m_itemOptionStats.mMoveSpeed.mPlus + m_SetItemStats.mMoveSpeed.mPlus;

	m_MoveInfo.AddedMoveSpeed = speed * addrateval + addval;

	speed += m_MoveInfo.AddedMoveSpeed;

	if(g_pServerSystem->GetMapNum() == GTMAPNUM)
		speed *= 0.8f;
	
	return max( 0, speed );
}


void CPlayer::SetInitedGrid()
{
	MSGBASE msg;
	msg.Category = MP_USERCONN;
	msg.Protocol = MP_USERCONN_GRIDINIT;
	SendMsg(&msg,sizeof(msg));

	CGridUnit::SetInitedGrid();
	
	CBattle* pBattle = BATTLESYSTEM->GetBattle(this->GetBattleID());
	if(pBattle && pBattle->GetBattleKind() != eBATTLE_KIND_NONE)
		BATTLESYSTEM->AddObjectToBattle(pBattle, this);

	{
		CQuestEvent QEvent( eQuestEvent_GameEnter, 0, 1 );
		QUESTMGR->AddQuestEvent( this, &QEvent );
	}

	{
		CQuestEvent QEvent( eQuestEvent_MapChange, 0, g_pServerSystem->GetMapNum() );
		QUESTMGR->AddQuestEvent( this, &QEvent );
	}

	// 090316 LUJ, Ż�Ϳ� ž���� ä�� �� �̵��� ��� �ڵ����� �¿�� ���� ������ �����´�
	LoadVehicleFromDb( GetID(), g_pServerSystem->GetMapNum() );
}
// RaMa - 04.11.10    -> ShopItemOption �߰�   AvatarOption�߰�(05.02.16)
DWORD CPlayer::DoGetCritical()
{	
	return (DWORD)mCriticalRate;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 06. 07 ���� ����(�ϰ�) - �̿���
DWORD CPlayer::DoGetDecisive()
{	
	return (DWORD)mCriticalRate;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CPlayer::DoGetPhyAttackPowerMin()
{
	return (DWORD)mPhysicAttackMin;
}

DWORD CPlayer::DoGetPhyAttackPowerMax()
{
	return (DWORD)mPhysicAttackMax;
}

void CPlayer::DoDamage(CObject* pAttacker,RESULTINFO* pDamageInfo,DWORD beforeLife)
{
	SetObjectBattleState( eObjectBattleState_Battle );
	if ( pAttacker )
	{
		pAttacker->SetObjectBattleState( eObjectBattleState_Battle );
		AddToAggroed(
			pAttacker->GetID());
	}

	if( GetAbnormalStatus()->IsSlip )
	{
		EndBuffSkillByStatus( eStatusKind_Slip );
	}

	// 090109 LUJ, �ǰ� �� ĳ���� ���� ��ų�� ��ҵ� �� �ִ�
	CancelCurrentCastingSkill( TRUE );
}

void CPlayer::DoManaDamage( CObject* pAttacker, RESULTINFO* pDamageInfo, DWORD beforeMana )
{
	SetObjectBattleState( eObjectBattleState_Battle );
	if ( pAttacker )
	{
		pAttacker->SetObjectBattleState( eObjectBattleState_Battle );
	}

	if( GetAbnormalStatus()->IsSlip )
	{
		EndBuffSkillByStatus( eStatusKind_Slip );
	}

	// 090109 LUJ, �ǰ� �� ĳ���� ���� ��ų�� ��ҵ� �� �ִ�
	CancelCurrentCastingSkill( TRUE );
}

void CPlayer::InitBaseObjectInfo(BASEOBJECT_INFO* pBaseInfo)
{
	ASSERT(GetInitState() == PLAYERINITSTATE_ONLY_ADDED);
	memcpy(&m_BaseObjectInfo,pBaseInfo,sizeof(BASEOBJECT_INFO));
}

/* ��oC����?E���Ϣ�����c Return */
MONEYTYPE CPlayer::SetMoney( MONEYTYPE ChangeValue, BYTE bOper, BYTE MsgFlag, eITEMTABLE tableIdx, BYTE LogType, DWORD TargetIdx )
{
	CPurse* pPurse = m_ItemContainer.GetPurse(tableIdx);
	if( !pPurse) return 0;

	MONEYTYPE Real = 0;
	if( bOper == MONEY_SUBTRACTION )
	{
		Real = pPurse->Subtraction( ChangeValue, MsgFlag );
	}
	else
	{
		Real = pPurse->Addition( ChangeValue, MsgFlag );
	}

	if(tableIdx == eItemTable_Inventory)
	{
		if( Real >= 10000 )
			InsertLogWorngMoney( LogType, GetID(), Real, GetMoney(), GetMoney(eItemTable_Storage), TargetIdx );
	}

	return Real;
}

/* ��i����; �ϩ�O; A�Ϣ���������CN �����Ϩ�������IAI AO�ˡ�AAo�ˡ�| ��o?�ˡ�A�ˡ�U.*/
BOOL CPlayer::IsEnoughAdditionMoney(MONEYTYPE money, eITEMTABLE tableIdx )
{
	CPurse* pPurse = m_ItemContainer.GetPurse(tableIdx);
	if(!pPurse) return FALSE;

	return pPurse->IsAdditionEnough( money );
}


MONEYTYPE CPlayer::GetMaxPurseMoney(eITEMTABLE TableIdx)
{
	CPurse* pPurse = m_ItemContainer.GetPurse(TableIdx);
	if( !pPurse) return FALSE;
	return pPurse->GetMaxMoney();
}

void CPlayer::SetMaxPurseMoney(eITEMTABLE TableIdx, MONEYTYPE MaxMoney)
{
	//C��I������o; ��ic�ˡ�e ��i���� �����ˡ�����u �ˡ�������o����A�ˢ碮���ˢ� �ˡ�A��ui�ϩ����ϡˡ�U.
	if(TableIdx != eItemTable_Storage)
	{
		ASSERT(0);
		return;
	}
	CPurse* pPurse = m_ItemContainer.GetPurse(TableIdx);
	if( !pPurse) return;
	pPurse->SetMaxMoney(MaxMoney);
}

void CPlayer::AddQuest(CQuestBase* pQuest)
{
	if(CQuestBase* questBase = m_QuestList.GetData(pQuest->GetQuestIdx()))
	{
		m_QuestList.Remove(
			pQuest->GetQuestIdx());
		SAFE_DELETE(
			questBase);
	}
	m_QuestList.Add( pQuest, pQuest->GetQuestIdx() );
}

BOOL CPlayer::SetQuestState(DWORD QuestIdx, QSTATETYPE value)
{
	CQuestBase* pQuest;
	pQuest = m_QuestList.GetData(QuestIdx);

	if( !pQuest ) 
	{
//		char buff[256] = {0,};
//		sprintf(buff, "��?a����a�ˡ�A Au��o����������c�����ˢ� xAc ��uECN�ˡ�U�ϩ��ϡ� CI��?��I������I ��oy��u��c��?�ˢ碮��O ��uE����AAa��?���� [QUEST ID : %d]", QuestIdx);
//		ASSERTMSG(0, buff);
		return FALSE;
	}

	pQuest->SetValue(value);
	BOOL bEnd = pQuest->IsComplete();

	// DB��?�ˢ� ��uA����ACN�ˡ�U.
	QuestUpdateToDB( GetID(), QuestIdx, value, (BYTE)bEnd );

	if( bEnd )
	{
		m_QuestList.Remove(QuestIdx);

		MSG_DWORD msg;
		msg.Category = MP_QUEST;
		msg.Protocol = MP_QUEST_REMOVE_NOTIFY;
		msg.dwObjectID = GetID();
		msg.dwData = QuestIdx;
		SendMsg(&msg, sizeof(msg));

		SAFE_DELETE(pQuest);
	}

	return TRUE;
}

void CPlayer::SetInitState(int initstate,DWORD protocol)
{
	m_initState |= initstate;


	// 091106 LUJ, ������ if�� ó���� ����ȭ
	if(FALSE == (m_initState & PLAYERINITSTATE_ONLY_ADDED))
	{
		return;
	}
	else if(FALSE == (m_initState & PLAYERINITSTATE_SKILL_INFO))
	{
		return;
	}
	else if(FALSE == (m_initState & PLAYERINITSTATE_ITEM_INFO))
	{
		return;
	}
	else if(FALSE == (m_initState & PLAYERINITSTATE_QUICK_INFO))
	{
		return;
	}
	else if(FALSE == (m_initState & PLAYERINITSTATE_HERO_INFO))
	{
		return;
	}
	else if(FALSE == (m_initState & PLAYERINITSTATE_QEUST_INFO))
	{
		return;
	}
	else if(FALSE == (m_initState & PLAYERINITSTATE_FARM_INFO))
	{
		return;
	}
	else if(FALSE == (m_initState & PLAYERINITSTATE_CHALLENGEZONE_INFO))
	{
		return;
	}
	else if(FALSE == (m_initState & PLAYERINITSTATE_STORAGE_INFO))
	{
		CharacterStorageInfo(GetUserID(), GetID());
		return;
	}
	else if(FALSE == (m_initState & PLAYERINITSTATE_OPTION_INFO))
	{
		return;
	}

	SetPeaceMode(g_pServerSystem->GetMap()->IsVillage());

	for(POSTYPE part = TP_WEAR_START ; part < TP_WEAR_END ; ++part )
	{
		const ITEMBASE * pTargetItemBase = ITEMMGR->GetItemInfoAbsIn(this, part);

		if( pTargetItemBase && 
			pTargetItemBase->dwDBIdx )
		{
			m_HeroCharacterInfo.WearedItemIdx[part-TP_WEAR_START] = pTargetItemBase->wIconIdx;
		}
		else
		{
			m_HeroCharacterInfo.WearedItemIdx[part-TP_WEAR_START] = 0;
		}
	}

	const ITEMBASE * pWeaponItemBase = ITEMMGR->GetItemInfoAbsIn(this, TP_WEAR_START + eWearedItem_Weapon );

	if( pWeaponItemBase && pWeaponItemBase->dwDBIdx )
	{
		const ITEM_OPTION& option = ITEMMGR->GetOption( *pWeaponItemBase );

		if( option.mItemDbIndex )
		{
			m_HeroCharacterInfo.WeaponEnchant = option.mEnchant.mLevel;
		}
	}

	m_dwProgressTime = gCurTime;

	// LUJ, Ŭ���̾�Ʈ�� �����ϱ� ���� ����ؾ��Ѵ�
	CHARCALCMGR->Initialize(
		this);

	SEND_HERO_TOTALINFO msg;
	memset( &msg, 0, sizeof(msg) );
	GetBaseObjectInfo(&msg.BaseObjectInfo);
	GetCharacterTotalInfo(&msg.ChrTotalInfo);
	GetHeroTotalInfo(&msg.HeroTotalInfo);
	GetItemtotalInfo(
		msg.ItemTotalInfo,
		GETITEM_FLAG_INVENTORY | GETITEM_FLAG_WEAR);
	GetSendMoveInfo(&msg.SendMoveInfo,NULL);


	msg.ChrTotalInfo.CurMapNum = GAMERESRCMNGR->GetLoadMapNum();
	msg.UniqueIDinAgent = GetUniqueIDinAgent();

	SKILL_BASE SkillTreeInfo[MAX_SKILL_TREE] = {0};	
	m_SkillTree->SetPositionHead();

	for(SKILL_BASE* skill = m_SkillTree->GetData();
		0 < skill;
		skill = m_SkillTree->GetData())
	{
		if(msg.SkillNum > sizeof(SkillTreeInfo) / sizeof(*SkillTreeInfo))
		{
			break;
		}

		SkillTreeInfo[msg.SkillNum] = *skill;
		++msg.SkillNum;
	}

	msg.AddableInfo.AddInfo(
		CAddableInfoList::SkillTree,
		sizeof(*SkillTreeInfo) * msg.SkillNum,
		SkillTreeInfo,
		__FUNCTION__);
	msg.ChrTotalInfo.DateMatching = m_DateMatching;
	msg.ChrTotalInfo.AliasType = m_AliasType;

	srand( GetTickCount());
	GetLocalTime(&msg.ServerTime);

	// 080827 LYW --- Player : ���� ���¸� ��������(Client)�� �����Ѵ�.
	msg.Category	= MP_USERCONN;
	msg.Protocol	= MP_USERCONN_GAMEIN_ACK;

	// 071227 LUJ
	msg.StorageSize	= GetStorageNum();
	SendMsg( &msg, msg.GetMsgLength() );

	MSG_DWORDEX2 msgFishingExp;
	msgFishingExp.Category = MP_FISHING;
	msgFishingExp.Protocol = MP_FISHING_EXP_ACK;
	msgFishingExp.dweData1 = (DWORDEX)m_wFishingLevel;
	msgFishingExp.dweData2 = m_dwFishingExp;
	SendMsg( &msgFishingExp, sizeof(msgFishingExp) );

	// 080424 NYJ --- ���������Ʈ
	MSG_DWORD msgFishPoint;
	msgFishPoint.Category = MP_FISHING;
	msgFishPoint.Protocol = MP_FISHING_POINT_ACK;
	msgFishPoint.dwData   = m_dwFishPoint;
	SendMsg( &msgFishPoint, sizeof(msgFishPoint) );

	//aziz MallShop 24
	MSG_DWORD msgVipPoint;
	msgVipPoint.Category = MP_VIP;
	msgVipPoint.Protocol = MP_VIP_POINT_ACK;
	msgVipPoint.dwData   = m_dwVipPoint;
	SendMsg( &msgVipPoint, sizeof(msgVipPoint) );

	//aziz Reborn 24 Sep
	MSG_DWORD msgRebornData;
	msgRebornData.Category = MP_REBORN;
	msgRebornData.Protocol = MP_REBORN_DATA_ACK;
	msgRebornData.dwData   = m_dwRebornData;
	SendMsg( &msgRebornData, sizeof(msgRebornData) );

	//aziz Kill Shop 30 Sep
	MSG_DWORD msgKillPoint;
	msgKillPoint.Category = MP_KILL;
	msgKillPoint.Protocol = MP_KILL_POINT_ACK;
	msgKillPoint.dwData   = m_dwKillPoint;
	SendMsg( &msgKillPoint, sizeof(msgKillPoint) );

	//aziz Reborn Point 13 Oct
	MSG_DWORD msgRebornPoint;
	msgRebornPoint.Category = MP_UTILITY;
	msgRebornPoint.Protocol = MP_REBORNPOINT_ACK;
	msgRebornPoint.dwData   = m_dwRebornPoint;
	SendMsg( &msgRebornPoint, sizeof(msgRebornPoint) );
	
	// �丮���õ�
	MSG_DWORD4 msgCookState;
	msgCookState.Category = MP_COOK;
	msgCookState.Protocol = MP_COOK_STATE;
	msgCookState.dwData1 = GetCookLevel();
	msgCookState.dwData2 = GetCookCount();
	msgCookState.dwData3 = GetEatCount();
	msgCookState.dwData4 = GetFireCount();
	SendMsg( &msgCookState, sizeof(msgCookState) );

	// �丮���η�����
	int i;
	for(i=0; i<MAX_RECIPE_LV4_LIST; i++)
	{
		MSG_DWORD4	msgRecipe;
		msgRecipe.Category = MP_COOK;
		msgRecipe.Protocol = MP_COOK_UPDATERECIPE;
		msgRecipe.dwData1 = eCOOKRECIPE_ADD;
		msgRecipe.dwData2 = m_MasterRecipe[i].dwRecipeIdx;
		msgRecipe.dwData3 = i;
		msgRecipe.dwData4 = m_MasterRecipe[i].dwRemainTime;
		SendMsg( &msgRecipe, sizeof(msgRecipe) );
	}

	VECTOR3 pos;
	msg.SendMoveInfo.CurPos.Decompress(&pos);

	MSG_WORD3 msgMapDesc;
	msgMapDesc.Category		= MP_USERCONN;
	msgMapDesc.Protocol		= MP_USERCONN_MAPDESC;
	msgMapDesc.wData1		= (WORD)g_pServerSystem->GetMap()->IsVillage();
	// 090824 ONS GM������ PK����� ������ PK��밪.
	msgMapDesc.wData2		= (WORD)PKMGR->IsPKAllow();
	msgMapDesc.wData3		= (WORD)GetCurChannel();
	SendMsg( &msgMapDesc, sizeof(msgMapDesc) );

	const MAPTYPE MapNum = GAMERESRCMNGR->GetLoadMapNum();

	RegistLoginMapInfo(
		GetID(),
		GetObjectName(),
		BYTE(GTMAPNUM == MapNum ? GTRETURNMAPNUM : MapNum),
		g_pServerTable->GetSelfServer()->wPortForServer);
	SetInited();
	if(PARTYMGR->CanUseInstantPartyMap(g_pServerSystem->GetMapNum()))
		PARTYMGR->ProcessReservationList(GetID());

	PARTYMGR->UserLogIn(
		this,
		MP_USERCONN_GAMEIN_SYN == protocol);
	QUICKMNGR->SendQuickInfo(
		this);

	// S ���͹��� �߰� added by hseos 2007.05.29
	{
		MSG_DWORD2 msg;
		msg.Category	= MP_CHAR;
		msg.Protocol	= MP_CHAR_MONSTERMETER_PLAYTIME;
		msg.dwData1		= m_stMonstermeterInfo.nPlayTime;
		msg.dwData2		= m_stMonstermeterInfo.nPlayTimeTotal;
		SendMsg(&msg, sizeof(msg));

		msg.Category	= MP_CHAR;
		msg.Protocol	= MP_CHAR_MONSTERMETER_KILLMONSTER;
		msg.dwData1		= m_stMonstermeterInfo.nKillMonsterNum;
		msg.dwData2		= m_stMonstermeterInfo.nKillMonsterNumTotal;
		SendMsg(&msg, sizeof(msg));
		// Ʈ���Ÿ� �о���δ�
		TRIGGERMGR->LoadTrigger(*this);
	}

	g_csFarmManager.SRV_SendPlayerFarmInfo(this);
	g_csDateManager.SRV_SendChallengeZoneEnterFreq(this);
	// 091106 LUJ, ����Ʈ ������ ��Ͻ�Ų��
	LIMITDUNGEONMGR->AddPlayer(*this);
	GUILDMGR->AddPlayer( this );
	GUILDWARMGR->AddPlayer( this );
	PassiveSkillCheckForWeareItem();
	PetInfoLoad(
		GetID(),
		GetUserID());
	CharacterBuffLoad(
		GetID());

	CCharMove::InitMove(
		this,
		&m_MoveInfo.CurPosition);

	if(CGridTable* const gridTable = g_pServerSystem->GetGridSystem()->GetGridTable(this))
	{
		gridTable->AddObject(
			this,
			CCharMove::GetPosition(this));
	}

	// 100408 ShinJS --- ���� �ð� ����
	stTime64t serverTimeMsg;
	ZeroMemory( &serverTimeMsg, sizeof(serverTimeMsg) );
	serverTimeMsg.Category = MP_USERCONN;
	serverTimeMsg.Protocol = MP_USERCONN_GETSERVERTIME_ACK;
	serverTimeMsg.dwObjectID = GetID();
	_time64( &serverTimeMsg.time64t );

	SendMsg( &serverTimeMsg, sizeof(serverTimeMsg) );

	if(const DWORD gameRoomIndex = PCROOMMGR->GetGameRoomIndex(GetID()))
	{
		g_DB.FreeMiddleQuery(
			RLoadPCRoomPoint,
			GetID(),
			"EXEC dbo.MP_GAMEROOM_LOAD_POINT %d, %d, %d",
			GetID(),
			gameRoomIndex,
			g_pServerSystem->GetMapNum());
	}

	// 100525 NYJ - �ǸŴ��� ��ϻ�ǰ�� ���� �ð����üũ ����
	Consignment_CheckDate(GetID());
	Note_CheckDate(GetID());

	// 100611 ONS �α��ν� ä�ñ��� ������ �ε��Ѵ�.
	ForbidChatLoad(GetID());
	if(g_pServerSystem->GetMapNum() == PVP_MAP)
	{
		if(CParty* pParty = PARTYMGR->GetParty(GetPartyIdx()))
		{
			if(pParty->GetMasterID() == GetID())
			{
					PARTYMGR->BreakupParty(pParty->GetPartyIdx(), GetID());
			}
			else
			{
				PARTYMGR->DelMember(GetID(), pParty->GetPartyIdx());
			}
		}

		if(CPet* const petObject = PETMGR->GetPet(GetPetItemDbIndex()))
		{
			PETMGR->SealPet(petObject);
		}

		CVehicle* const vehicle = ( CVehicle* )g_pUserTable->FindUser( GetSummonedVehicle() );

		if( vehicle && vehicle->GetObjectKind() == eObjectKind_Vehicle )
		{
			VEHICLEMGR->Unsummon( GetID(), TRUE );
		} 

		if( GetState() == eObjectState_Immortal )
		{
			OBJECTSTATEMGR_OBJ->EndObjectState( this, eObjectState_Immortal );
		} else {
			OBJECTSTATEMGR_OBJ->StartObjectState(this, eObjectState_BattleReady, 0);
		}
	}

	if(STREETTOURNAMENTMGR->isStreetTournament())
	{
		if(STREETTOURNAMENTMGR->GetState() == 0 || !IsRegisterStreetTournament() )//|| !allowedstbyrebirth)
		{
			STREETTOURNAMENTMGR->MoveToAlker(this, TRUE);
		} else 
		{
			if(CParty* pParty = PARTYMGR->GetParty(GetPartyIdx()))
			{
				if(pParty->GetMasterID() == GetID())
				{
						PARTYMGR->BreakupParty(pParty->GetPartyIdx(), GetID());
				}
				else
				{
					PARTYMGR->DelMember(GetID(), pParty->GetPartyIdx());
				}
			}

			if(CPet* const petObject = PETMGR->GetPet(GetPetItemDbIndex()))
			{
				PETMGR->SealPet(petObject);
			}

			CVehicle* const vehicle = ( CVehicle* )g_pUserTable->FindUser( GetSummonedVehicle() );

			if( vehicle && vehicle->GetObjectKind() == eObjectKind_Vehicle )
			{
				VEHICLEMGR->Unsummon( GetID(), TRUE );
			} 
		}
	}

}

int CPlayer::CanExitStart()	//~����av����C
{
//	if( GetState() != eObjectState_None && GetState() != eObjectState_Move )
//		return FALSE;
//	if( IsPKMode() )
	if( IsPKMode() && g_pServerSystem->GetMapNum()!=PVP)
		return eEXITCODE_PKMODE;
	if( LOOTINGMGR->IsLootedPlayer(GetID()) )	//PK����c����A; �ˡ�cCI�ˡ�A A�ϡ�AI�����ˢ�?
		return eEXITCODE_LOOTING;

	if( GetState() == eObjectState_Exchange )	//����E?A������ A����aCO ��o ������U.
		return eEXITCODE_NOT_ALLOW_STATE;

	if( GetState() == eObjectState_StreetStall_Owner ||
		GetState() == eObjectState_StreetStall_Guest )	//��eA��A������ A����aCO ��o ������U.
		return eEXITCODE_NOT_ALLOW_STATE;

	if( GetState() == eObjectState_Deal )	//��oA�� AI��eA������ A����aCO ��o ������U.
		return eEXITCODE_NOT_ALLOW_STATE;

	if(g_pServerSystem->GetMapNum() == StreetTournament)
		return eEXITCODE_NOT_ALLOW_STATE;
	
	return eEXITCODE_OK;
}

void CPlayer::SetExitStart( BOOL bExit )
{
	m_bExit				= bExit;
	m_dwExitStartTime	= gCurTime;
}

int CPlayer::CanExit()
{
	DWORD lCurTime = MHTIMEMGR_OBJ->GetNewCalcCurTime();
	if( lCurTime - m_dwExitStartTime < EXIT_COUNT*1000 - 2000 )	//8.0	//���۸� ����� ����
		return eEXITCODE_SPEEDHACK;

//	if( IsPKMode() )							//PK�ˡ��ϡ̡�iaAI�����ˢ�?
	if( IsPKMode() && g_pServerSystem->GetMapNum()!=PVP)							//PK�ˡ��ϡ̡�iaAI�����ˢ�?
		return eEXITCODE_PKMODE;
	if( LOOTINGMGR->IsLootedPlayer(GetID()) )	//PK����c����A; �ˡ�cCI�ˡ�A A�ϡ�AI�����ˢ�?
		return eEXITCODE_LOOTING;

//---KES AUTONOTE
	if( GetAutoNoteIdx() )
		return eEXITCODE_NOT_ALLOW_STATE;
//---------------
	
	return eEXITCODE_OK;
}

void CPlayer::ExitCancel()
{
	if( IsExitStart() )
	{
		SetExitStart( FALSE );
		MSG_BYTE msgNack;
		msgNack.Category	= MP_CHAR;
		msgNack.Protocol	= MP_CHAR_EXIT_NACK;
		msgNack.bData		= eEXITCODE_DIEONEXIT;
		SendMsg(&msgNack, sizeof(msgNack));		
	}
}

int CPlayer::PKModeOn()
{
	if( IsPKMode() ) return ePKCODE_ALREADYPKMODEON;		//AI��oI PK�ˡ��ϡ̡�ia
	if( IsShowdown() ) return ePKCODE_SHOWDOWN;		//����n��o��iA�ϡ�AI�ˡ�e ��uE��iE�ˡ�U

	//pk������a A�Ƣ�e ����Au��oAA C��A|
	if( GetState() == eObjectState_Immortal )
		OBJECTSTATEMGR_OBJ->EndObjectState( this, eObjectState_Immortal );

	if( GetState() == eObjectState_Die )
		return ePKCODE_STATECONFLICT;	//�ˡ�U�ˡ���I��ioAAAI��O�����ˡ�A ��uE��iE�ˡ�U.
	
	m_HeroCharacterInfo.bPKMode = TRUE;
	m_dwPKModeStartTime			= gCurTime;

//---KES PK 071124
	m_dwPKContinueTime			= 20*60*1000 + ( GetBadFame() / 75 ) * 5*60*1000;	//�⺻ 30�� + �Ǹ�ġ 75���� 5��
//----------------

	return ePKCODE_OK;
}

BOOL CPlayer::PKModeOff()
{
	if (g_pServerSystem->GetMapNum() == PVP)
		return FALSE;

	if( !IsPKMode() ) return FALSE;

	if( gCurTime - m_dwPKModeStartTime >= m_dwPKContinueTime || ( GetUserLevel() <= eUSERLEVEL_GM && PKMGR->IsPKEvent() ) )
	{
		m_HeroCharacterInfo.bPKMode = FALSE;
		SetPKModeEndtime();
		PKCharacterUpdate( GetID(), m_HeroInfo.LastPKModeEndTime );
		return TRUE;
	}

	return FALSE;
}

void CPlayer::PKModeOffForce()
{
	m_HeroCharacterInfo.bPKMode = FALSE;
}

void CPlayer::StateProcess()
{
	switch( GetState() )		
	{
	case eObjectState_None:
		{
			if( m_BaseObjectInfo.ObjectBattleState )
			{
				if( !mpBattleStateDelay->Check() )
				{
					SetObjectBattleState( eObjectBattleState_Peace );
				}
			}

			//in Korea : per 10min
			//in China : per 30min
			DWORD dwInsertLogTime = INSERTLOG_TIME;

			if( gCurTime - m_dwProgressTime >= dwInsertLogTime )
			{
				{
					CPet* const petObject = (CPet*)g_pUserTable->FindUser(
						GetPetItemDbIndex());

					if(petObject &&
						eObjectKind_Pet == petObject->GetObjectKind())
					{
						LogPet(
							petObject->GetObjectInfo(),
							ePetLogRegular);
					}
				}

				InsertLogExp(
					eExpLog_Time,
					GetID(),
					GetLevel(),
					0,
					GetPlayerExpPoint(),
					0,
					0,
					0);
				InsertLogMoney(
					0,
					GetID(),
					GetMoney(),
					0,
					0,
					0);

				if( IsPKMode())
				{
					m_HeroInfo.LastPKModeEndTime = 0;
				}
				
				UpdateCharacterInfoByTime(
					GetID(),
					GetPlayerExpPoint(),
					GetMoney(),
					m_HeroInfo.Playtime,
					m_HeroInfo.LastPKModeEndTime);
				MonsterMeter_Save(
					GetID(),
					m_stMonstermeterInfo.nPlayTime,
					m_stMonstermeterInfo.nKillMonsterNum,
					m_stMonstermeterInfo.nPlayTimeTotal,
					m_stMonstermeterInfo.nKillMonsterNumTotal);
				FishingData_Update(
					GetID(),
					GetFishingLevel(),
					GetFishingExp(),
					GetFishPoint());
				//aziz MallShop in Game Method 1
				VipData_Update(GetID(), GetVipPoint());
				//aziz Reborn 24 Sep
				RebornData_Update(GetID(), GetRebornData());
				//aziz Kill Shop 30 Sep
				KillData_Update(GetID(), GetKillPoint(), 3);
				//aziz Reborn Point 13 Oct
				RebornPoint_Update(GetID(), GetRebornPoint());

				m_dwProgressTime = gCurTime;
			}

			m_ContinuePlayTime += gTickTime;
			DWORD dwPT = m_ContinuePlayTime/1000;
			if(dwPT)
			{
				m_HeroInfo.Playtime += dwPT;
				m_ContinuePlayTime -= dwPT*1000;
			}
		}
		break;
	case eObjectState_Die:
		{
			//---KES PK 071202	���� ��� �ð��� ��� ���� (�ð��� �Ȱ�����)
			SetPKStartTimeReset();
			
			if(FALSE == m_bNeedRevive )
			{
				break;
			}
			else if(g_pServerSystem->GetMapNum() == GTMAPNUM)
			{
				if(gTickTime < m_dwRespawnTimeOnGTMAP)
					m_dwRespawnTimeOnGTMAP -= gTickTime;
				else
				{
					MSGBASE message;
					message.Category	= MP_USERCONN;
					message.Protocol	= MP_USERCONN_READY_TO_REVIVE_BY_GFW;
					SendMsg( &message, sizeof( message ) );
					m_bNeedRevive = FALSE;
				}

				break;
			}

			DWORD dwElapsed = gCurTime - m_ObjectState.State_Start_Time;

			if( dwElapsed > PLAYERREVIVE_TIME && g_pServerSystem->GetMapNum() != StreetTournament)
			{
				if(TRUE == m_bDieForGFW)
				{
					MSGBASE message;
					message.Category	= MP_USERCONN;
					message.Protocol	= MP_USERCONN_READY_TO_REVIVE_BY_GFW;					
					SendMsg( &message, sizeof( message ) );

					m_bNeedRevive = FALSE;

					break;
				}

				if( LOOTINGMGR->IsLootedPlayer( GetID() ) )
					break;
				if( IsReadyToRevive() != TRUE )
					break;
				
				MSGBASE msg;
				msg.Category = MP_USERCONN;
				msg.Protocol = MP_USERCONN_READY_TO_REVIVE;
				SendMsg( &msg, sizeof(msg) );

				m_bNeedRevive = FALSE;
			} else if (dwElapsed > PLAYERREVIVE_TIME && g_pServerSystem->GetMapNum() == StreetTournament && m_bDieForGFW == TRUE)
			{
				STREETTOURNAMENTMGR->MoveToAlker(this, FALSE);
			}
		}
		break;
	}
	
	if(m_lastspinslot != 0)
	{
		if(gCurTime > m_lastspinslot)
		{
			m_lastspinslot = 0;
			SendSlotHasil();
		}
	}

	AddBadFameReduceTime();
	ProcMonstermeterPlayTime();
	g_csResidentRegistManager.SRV_ProcDateMatchingChatTimeOut(this);
	ProcFarmTime();
	g_csDateManager.SRV_Process(this);
	ProcCoolTime();
	ProcSpecialSkill();
}

void CPlayer::SetWearedItemIdx(DWORD WearedPosition,DWORD ItemIdx)
{	
	const DWORD size = sizeof( m_HeroCharacterInfo.WearedItemIdx ) / sizeof( DWORD );

	if( size > WearedPosition )
	{
		m_HeroCharacterInfo.WearedItemIdx[WearedPosition] = ItemIdx;
	}
	else
	{
		ASSERT( 0 );
	}	
}

void CPlayer::QuestProcess()
{
	m_QuestGroup.Process();
}


void CPlayer::SetPKModeEndtime()
{
	m_HeroInfo.LastPKModeEndTime = 0;
}


void CPlayer::AddBadFameReduceTime()
{
	if( g_pServerSystem->GetMap()->IsVillage() )	return;
	if( GetState() == eObjectState_Immortal )		return;
	if( IsPKMode() && g_pServerSystem->GetMapNum() != PVP)								return;

	m_HeroInfo.LastPKModeEndTime += gTickTime;

	//---KES PK 071202
	if( m_HeroInfo.LastPKModeEndTime > 90*60*1000 )
	{
		m_HeroInfo.LastPKModeEndTime -= 30*60*1000;	//
		PKCharacterUpdate( GetID(), m_HeroInfo.LastPKModeEndTime );

        if( GetBadFame() > 5 )
		{
			SetBadFame( GetBadFame()-5 );
		}
		else
		{
			SetBadFame( 0 );
			m_HeroInfo.LastPKModeEndTime = 0;
		}

		BadFameCharacterUpdate( GetID(), GetBadFame() );
	}
}



void CPlayer::SpeedHackCheck()
{
	++m_nHackCount;

	if( gCurTime - m_dwHackStartTime >= 60*1000 )	//1��
	{
		if( m_nHackCount >= g_nHackCheckWriteNum )
		{
			InsertSpeedHackCheck( GetID(), GetObjectName(), m_nHackCount, 1 );
		}
		if( m_nHackCount >= g_nHackCheckNum )
		{
			MSGBASE Msg;
			Msg.Category = MP_HACKCHECK;
			Msg.Protocol = MP_HACKCHECK_BAN_USER_TOAGENT;
			SendMsg( &Msg, sizeof(Msg) );
		}
		m_nHackCount		= 1;
		m_dwHackStartTime	= gCurTime;
	}
}

void CPlayer::ClearMurderIdx()
{
	SetReadyToRevive(FALSE);
	m_MurdererKind = 0;
	m_MurdererIDX = 0;
	m_bPenaltyByDie = FALSE;
}


DWORD CPlayer::Damage(CObject* pAttacker,RESULTINFO* pDamageInfo)
{
	DWORD life = GetLife();
	DWORD beforelife = life;

	if(life > pDamageInfo->RealDamage)
	{
		life -= pDamageInfo->RealDamage;
	}
	else
	{
		if( GetUserLevel() == eUSERLEVEL_GM || m_God ) // && g_pServerSystem->GetNation() == eNATION_KOREA )
		{
			life = 1;		//gm�� �������� �޾Ƶ� ���� �ʵ���
		}
		else
		{
			life = 0;
		}
	}
	
	SetLife(life,FALSE);
 
	DoDamage(pAttacker,pDamageInfo,beforelife);

	return life;
}

DWORD CPlayer::ManaDamage( CObject* pAttacker, RESULTINFO* pDamageInfo )
{
	DWORD mana = GetMana();
	DWORD beforemana = mana;
	pDamageInfo->ManaDamage = (pDamageInfo->ManaDamage / 2) * 3;
	mana = (mana > pDamageInfo->ManaDamage ? mana - pDamageInfo->ManaDamage : 0);
	
	SetMana( mana, FALSE );
 
	// ���� �������� �ִ� ���
	if( pDamageInfo->RealDamage == 0 )
		DoManaDamage( pAttacker, pDamageInfo, beforemana );

	return mana;
}

void CPlayer::SetGuildInfo(DWORD GuildIdx, BYTE Grade, const char* GuildName, MARKNAMETYPE MarkName)
{
	m_HeroCharacterInfo.MunpaID = GuildIdx;
	m_HeroCharacterInfo.PositionInMunpa = Grade;
	SafeStrCpy(m_HeroCharacterInfo.GuildName, GuildName, sizeof( m_HeroCharacterInfo.GuildName ) );
	m_HeroCharacterInfo.MarkName = MarkName;
}

void CPlayer::RSetMoney( MONEYTYPE money, BYTE flag )
{
	CPurse* pPurse = m_ItemContainer.GetPurse(eItemTable_Inventory);
	if( !pPurse ) return;

	pPurse->RSetMoney( money, flag );
	m_HeroInfo.Money = money;
}

void CPlayer::SetGuildMarkName(MARKNAMETYPE MarkName)
{
	m_HeroCharacterInfo.MarkName = MarkName;
}

char* CPlayer::GetGuildCanEntryDate()
{ 
	return m_HeroInfo.MunpaCanEntryDate;
}

void CPlayer::SetGuildCanEntryDate(char* date)
{
	SafeStrCpy(m_HeroInfo.MunpaCanEntryDate, date, 11);
}

void CPlayer::ClrGuildCanEntryDate()
{
	SafeStrCpy(m_HeroInfo.MunpaCanEntryDate, "2004.01.01", 11);
}

void CPlayer::UpdateLogoutToDB(BOOL val)
{
	if(CPet* const petObject = PETMGR->GetPet(GetPetItemDbIndex()))
	{
		if(eObjectState_Die != petObject->GetState())
		{
			petObject->SetPetObjectState(
				val ? ePetState_None : ePetState_Summon);
		}
	}

	for(int i = 0; i < _countof(m_QuickSlot); ++i)
	{
		m_QuickSlot[i].DBUpdate();
	}
}

void CPlayer::SetPVPScoreLogin(DWORD pvpscore)
{
	m_pvpscore = pvpscore;
	
	if(m_pvpscore != 0)
	{
		MSG_DWORD msg ;
		msg.Category	= MP_USERCONN ;
		msg.Protocol	= MP_PVP_CEK_LOGIN ;
		msg.dwObjectID	= GetID();
		msg.dwData		= pvpscore;
		SendMsg( &msg, sizeof(msg) );
	}
	
	if(STREETTOURNAMENTMGR->GetState() != 0)
	{
		MSG_WORD3 msg2 ;
		ZeroMemory(&msg2, sizeof(msg2));
		msg2.Category	= MP_USERCONN ;
		msg2.Protocol	= MP_USERCONN_STREETTOURNAMENT_STATE;
		msg2.wData1		= 0 ;
		msg2.wData2		= STREETTOURNAMENTMGR->GetState();
		msg2.wData3		= STREETTOURNAMENTMGR->GetStage();
		SendMsg( &msg2, sizeof(msg2) );
	}
	
}

DWORD CPlayer::GetPVPScore()
{
	return m_pvpscore;
}

void CPlayer::UpdatePVPScore(DWORD pvpscore)
{
	m_pvpscore = pvpscore;
}
void CPlayer::DoSpinSlot(DWORD SpinBet, DWORD SpinTime)
{
	if(g_pServerSystem->GetMapNum() == GTMAPNUM || g_pServerSystem->GetMapNum() == PVP)
		return;

	if(m_lastspinslot != 0 || m_pSpinslothasil1 != 0 || m_pSpinslothasil2 != 0 || m_pSpinslothasil3 != 0 || m_pSpinslothasil4 != 0)
		return;
	
	if(m_MaxCountSpin > 50) 
	{
		m_VerifyCaptcha = random(1000, 9999);
		//m_VerifyCount += 1;

		MSG_DWORD2 msg;
		msg.Category = MP_USERCONN;
		msg.Protocol = MP_USERCONN_CAPTCHA_SYN;
		msg.dwObjectID = GetID();
		msg.dwData1 = m_VerifyCaptcha;
		msg.dwData2 = 1;
		SendMsg( &msg, sizeof(msg) );

		return;
	}

	m_MaxCountSpin += 1;

	WORD cheatspincheck = 0;
	if (SpinBet == 200000)
	{
		cheatspincheck += 1;
	} else if (SpinBet == 400000)
	{
		cheatspincheck += 1;
	} else if (SpinBet == 600000)
	{
		cheatspincheck += 1;
	} else if (SpinBet == 1000000)
	{
		cheatspincheck += 1;
	} else {
		cheatspincheck = 0;
	}

	if (cheatspincheck == 0 )
	{
		MSG_WORD msg;
		msg.Category = MP_USERCONN;
		msg.Protocol = MP_SLOT_NACK;
		msg.dwObjectID = GetID();
		msg.wData = 0;
		SendMsg( &msg, sizeof(msg) );
		return;
	}
		
	if ((GetMoney() - SpinBet) < 10000000 )
	{
		MSG_WORD msg;
		msg.Category = MP_USERCONN;
		msg.Protocol = MP_SLOT_NACK;
		msg.dwObjectID = GetID();
		msg.wData = 1;
		SendMsg( &msg, sizeof(msg) );
		return;
	}

	if ((GetMoney()+4000000000) > MAX_INVENTORY_MONEY )
	{
		MSG_WORD msg;
		msg.Category = MP_USERCONN;
		msg.Protocol = MP_SLOT_NACK;
		msg.dwObjectID = GetID();
		msg.wData = 2;
		SendMsg( &msg, sizeof(msg) );
		return;
	}

	SetMoney(SpinBet, MONEY_SUBTRACTION ) ;

	if (SpinBet > 0)
	{
		MSG_DWORDEX2 msg2;
		msg2.Category	= MP_USERCONN;
		msg2.Protocol	= MP_ITEM_ADDMONEY_ICE;
		msg2.dwObjectID = GetID();
		msg2.dweData1	= 4;
		msg2.dweData2	= SpinBet;
		SendMsg( &msg2, sizeof(msg2) );
	}

	m_pLastSpinSlot = gCurTime;

	DWORD random_hasil71 = random(5, 7);
	DWORD random_hasil72 = random(4, 7);
	DWORD random_hasil73 = random(6, 7);
	DWORD random_hasil74 = random(4, 7);
	DWORD m_pSpinslot1	= random(1, random_hasil71);
	DWORD m_pSpinslot2	= random(1, random_hasil72);
	DWORD m_pSpinslot3	= random(1, random_hasil73);
	DWORD m_pSpinslot4	= random(1, random_hasil74);
	MONEYTYPE SlotGetmoney = 0;
	MONEYTYPE jackpotbefore = g_pServerSystem->GetTotjackpot();
	WORD m_wincode	= 0;
	//for starter
	using namespace std;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<unsigned long long> dis(1, 1999999999);
	if(jackpotbefore <= 0)
	{
randomize:
		unsigned long long randomX = dis(gen);
		if(randomX < 9999999)
			goto randomize;

		jackpotbefore = randomX;
	}

	SYSTEMTIME sysTime;
	GetLocalTime( &sysTime );

	if (m_pSpinslot1 == 7 && m_pSpinslot2 == 7 && m_pSpinslot3 == 7 && m_pSpinslot4 == 7)
	{
		//80% jackpot + 5x
		m_wincode = 1;
		SlotGetmoney = MONEYTYPE(jackpotbefore * 80) / 100;
		SlotGetmoney += SpinBet * 5;
		SlotGetmoney += SpinBet;

		//MONEYTYPE	getmoneynya = MONEYTYPE(jackpotbefore * 80) / 100;
		//MONEYTYPE	totjackpot = jackpotbefore - SlotGetmoney;
		//DailyJackpotUpdate(g_pServerSystem->GetMapNum(), totjackpot );
		//g_pServerSystem->SetMapTotaljackpot(totjackpot);
		g_pServerSystem->MinusMapjackpot(SlotGetmoney);

		m_additemachievment = 1;

	} else if (((m_pSpinslot1 == 7 && m_pSpinslot2 == 7 && m_pSpinslot3 == 7 && m_pSpinslot4 != 7) ||
		(m_pSpinslot1 != 7 && m_pSpinslot2 == 7 && m_pSpinslot3 == 7 && m_pSpinslot4 == 7) ||
		(m_pSpinslot1 == 7 && m_pSpinslot2 != 7 && m_pSpinslot3 == 7 && m_pSpinslot4 == 7) ||
		(m_pSpinslot1 == 7 && m_pSpinslot2 == 7 && m_pSpinslot3 != 7 && m_pSpinslot4 == 7)) && m_wincode == 0)
	{
		//50% jackpot + 5x
		m_wincode = 2;
		SlotGetmoney = MONEYTYPE(jackpotbefore * 50) / 100;
		SlotGetmoney += SpinBet * 5;
		SlotGetmoney += SpinBet;

		//MONEYTYPE	totjackpot = jackpotbefore - SlotGetmoney;
		//DailyJackpotUpdate(g_pServerSystem->GetMapNum(), totjackpot );
		//g_pServerSystem->SetMapTotaljackpot(totjackpot);
		g_pServerSystem->MinusMapjackpot(SlotGetmoney);
		
		m_additemachievment = 1;
		
	} else if (((m_pSpinslot1 == 7 && m_pSpinslot2 == 7 && m_pSpinslot3 != 7 && m_pSpinslot4 != 7) ||
		(m_pSpinslot1 != 7 && m_pSpinslot2 != 7 && m_pSpinslot3 == 7 && m_pSpinslot4 == 7) ||
		(m_pSpinslot1 == 7 && m_pSpinslot2 != 7 && m_pSpinslot3 != 7 && m_pSpinslot4 == 7) ||
		(m_pSpinslot1 != 7 && m_pSpinslot2 == 7 && m_pSpinslot3 == 7 && m_pSpinslot4 != 7) ||
		(m_pSpinslot1 != 7 && m_pSpinslot2 == 7 && m_pSpinslot3 != 7 && m_pSpinslot4 == 7) ||
		(m_pSpinslot1 == 7 && m_pSpinslot2 != 7 && m_pSpinslot3 == 7 && m_pSpinslot4 != 7)) && m_wincode == 0)
	{
		//10% money	+ 5x
		m_wincode = 3;
		SlotGetmoney = MONEYTYPE(jackpotbefore * 10) / 100;
		SlotGetmoney += SpinBet * 5;
		SlotGetmoney += SpinBet;

		//MONEYTYPE	totjackpot = jackpotbefore - SlotGetmoney;
		//DailyJackpotUpdate(g_pServerSystem->GetMapNum(), totjackpot );
		//g_pServerSystem->SetMapTotaljackpot(totjackpot);
		g_pServerSystem->MinusMapjackpot(SlotGetmoney);

		
	} else if ((m_pSpinslot1 == m_pSpinslot2 && m_pSpinslot2 == m_pSpinslot3 && m_pSpinslot3 == m_pSpinslot4) && m_wincode == 0)
	{
		//get 5x bet
		m_wincode = 4;
		SlotGetmoney = SpinBet * 5;
		SlotGetmoney += SpinBet;
		if(m_pSpinslot1 == 5)
		{
			m_additemachievment = 2;
		} else if(m_pSpinslot1 == 3)
		{
			m_additemachievment = 3;
		} else if(m_pSpinslot1 == 2)
		{
			m_additemachievment = 4;
		} else if(m_pSpinslot1 == 1)
		{
			m_additemachievment = 5;
		}

	} else if (((m_pSpinslot1 == m_pSpinslot2 && m_pSpinslot2 == m_pSpinslot3 && m_pSpinslot3 != m_pSpinslot4) ||
		(m_pSpinslot1 != m_pSpinslot2 && m_pSpinslot2 == m_pSpinslot3 && m_pSpinslot3 == m_pSpinslot4) ||
		(m_pSpinslot1 == m_pSpinslot2 && m_pSpinslot2 != m_pSpinslot3 && m_pSpinslot3 == m_pSpinslot4)) && m_wincode == 0)
	{
		//get 3x bet
		m_wincode = 5;
		SlotGetmoney = SpinBet * 3;
		SlotGetmoney += SpinBet; //+money back
		if((m_pSpinslot1 == 5 && m_pSpinslot2 == 5) || (m_pSpinslot2 == 5 && m_pSpinslot3 == 5))
		{
			m_additemachievment = 2;
		} else if((m_pSpinslot1 == 3 && m_pSpinslot2 == 3) || (m_pSpinslot2 == 3 && m_pSpinslot3 == 3))
		{
			m_additemachievment = 3;
		} else if((m_pSpinslot1 == 2 && m_pSpinslot2 == 2) || (m_pSpinslot2 == 2 && m_pSpinslot3 == 2))
		{
			m_additemachievment = 4;
		} else if((m_pSpinslot1 == 1 && m_pSpinslot2 == 1) || (m_pSpinslot2 == 1 && m_pSpinslot3 == 1))
		{
			m_additemachievment = 5;
		}
	} else if (((m_pSpinslot1 == m_pSpinslot2 && m_pSpinslot2 != m_pSpinslot3 && m_pSpinslot3 != m_pSpinslot4)||
		(m_pSpinslot1 != m_pSpinslot2 && m_pSpinslot2 != m_pSpinslot3 && m_pSpinslot3 == m_pSpinslot4)||
		(m_pSpinslot1 != m_pSpinslot2 && m_pSpinslot2 == m_pSpinslot3 && m_pSpinslot3 != m_pSpinslot4)) && m_wincode == 0)
	{
		//get back money
		m_wincode = 6;
		//SlotGetmoney = SpinBet;
		SlotGetmoney = SpinBet * 2;
	} else {
		//null hahahah
		//m_wincode = 0;
		SlotGetmoney = 0;
		DWORD forjackpot = DWORD((SpinBet*3) / 4);
		//MONEYTYPE	totjackpot = jackpotbefore + forjackpot;
		//DailyJackpotUpdate(g_pServerSystem->GetMapNum(), totjackpot );
		//g_pServerSystem->SetMapTotaljackpot(totjackpot);
		g_pServerSystem->PlusMapjackpot(forjackpot);
	}

	if (SlotGetmoney > 0)
	{
		SetMoney(SlotGetmoney, MONEY_ADDITION ) ;
	}

	m_lastspinslot = gCurTime + 8000;
	m_pSpinslothasil1 = WORD(m_pSpinslot1);
	m_pSpinslothasil2 = WORD(m_pSpinslot2);
	m_pSpinslothasil3 = WORD(m_pSpinslot3);
	m_pSpinslothasil4 = WORD(m_pSpinslot4);
	m_wincodehasil = m_wincode;
	SlotGetmoneyhasil = SlotGetmoney;

	/*
	MSG_SLOT_SYN_ACK msg3;
	msg3.Category	= MP_USERCONN;
	msg3.Protocol	= MP_SLOT_ACK;
	msg3.dwObjectID = GetID();
	msg3.dweData1	= SlotGetmoney;
	msg3.dweData2	= WORD(m_pSpinslot1);
	msg3.dweData3	= WORD(m_pSpinslot2);
	msg3.dweData4	= WORD(m_pSpinslot3);
	msg3.dweData5	= WORD(m_pSpinslot4);
	msg3.dweData6	= g_pServerSystem->GetTotjackpot();
	msg3.dweData7	= m_wincode;
	
	SendMsg( &msg3, sizeof(msg3) );
	*/
	TCHAR szFile[_MAX_PATH] = {0, } ;
	sprintf( szFile, "Log/SLOT%d_%02d%02d.log", g_pServerSystem->GetMapNum(), sysTime.wMonth, sysTime.wDay ) ;
	FILE *fp = fopen(szFile, "a+") ;
	if (fp)
	{
		fprintf(fp, "[%02d:%02d][%s]:[%d] -> [%f] data: [%d][%d][%d][%d]\n", sysTime.wHour, sysTime.wMinute, GetObjectName(), SpinBet, (float)SlotGetmoney, m_pSpinslot1, m_pSpinslot2, m_pSpinslot3, m_pSpinslot4 ) ;
		fclose(fp) ;
	}
	
}

void CPlayer::SendSlotHasil()
{
	SYSTEMTIME sysTime;
	GetLocalTime( &sysTime );

	char fnamex[256];
	sprintf(fnamex,"./webstream/event_month%02ddate%02d.txt", sysTime.wMonth, sysTime.wDay);

	MSG_SLOT_SYN_ACK msg3;
	msg3.Category	= MP_USERCONN;
	msg3.Protocol	= MP_SLOT_ACK;
	msg3.dwObjectID = GetID();
	msg3.dweData1	= SlotGetmoneyhasil;
	msg3.dweData2	= WORD(m_pSpinslothasil1);
	msg3.dweData3	= WORD(m_pSpinslothasil2);
	msg3.dweData4	= WORD(m_pSpinslothasil3);
	msg3.dweData5	= WORD(m_pSpinslothasil4);
	msg3.dweData6	= g_pServerSystem->GetTotjackpot();
	msg3.dweData7	= m_wincodehasil;
	
	SendMsg( &msg3, sizeof(msg3) );

	if(m_wincodehasil < 4)
	{
		char bufmsg[MAX_PATH] = {0,};
		if(m_wincodehasil == 1)
		{
			_stprintf(bufmsg, "[Slot Machine] Grats! %s win jackpot 7777 get %I64u gold!", GetObjectName(), SlotGetmoneyhasil);
			MSG_CHAT_WORD msg;
			msg.Category	= MP_CHEAT;
			msg.Protocol	= MP_CHEAT_NOTICE_SYN_BOSS;
			msg.dwObjectID	= 255671;
			msg.wData = 0;
			SafeStrCpy( msg.Msg, bufmsg, sizeof(msg.Msg));
			g_Network.Broadcast2AgentServer((char*)&msg, sizeof(msg));

			FILE* fpLogx = fopen( fnamex, "a+" );
			if( fpLogx )
			{
				fprintf( fpLogx, "<font color='green'>[%02d:%02d][ITEM]: Congratulations! <b>%s</b> has win Jackpot 7777<b>%f</b></font>\n", sysTime.wHour, sysTime.wMinute, GetObjectName(), (float)SlotGetmoneyhasil );
				fclose( fpLogx );
			}

		} else if(m_wincodehasil == 2)
		{
			_stprintf(bufmsg, "[Slot Machine] Grats! %s win jackpot 777 get %I64u gold!", GetObjectName(), SlotGetmoneyhasil);
			MSG_CHAT_WORD msg;
			msg.Category	= MP_CHEAT;
			msg.Protocol	= MP_CHEAT_NOTICE_SYN_BOSS;
			msg.dwObjectID	= 255671;
			msg.wData = 0;
			SafeStrCpy( msg.Msg, bufmsg, sizeof(msg.Msg));
			g_Network.Broadcast2AgentServer((char*)&msg, sizeof(msg));

			FILE* fpLogx = fopen( fnamex, "a+" );
			if( fpLogx )
			{
				fprintf( fpLogx, "<font color='green'>[%02d:%02d][ITEM]: Congratulations! <b>%s</b> has win Jackpot 777<b>%f</b></font>\n", sysTime.wHour, sysTime.wMinute, GetObjectName(), (float)SlotGetmoneyhasil );
				fclose( fpLogx );
			}

			
		} else if(m_wincodehasil == 3)
		{
			_stprintf(bufmsg, "[Slot Machine] Grats! %s win jackpot 77 get %I64u gold!", GetObjectName(), SlotGetmoneyhasil);
			MSG_CHAT_WORD msg;
			msg.Category	= MP_CHEAT;
			msg.Protocol	= MP_CHEAT_NOTICE_SYN_BOSS;
			msg.dwObjectID	= 255671;
			msg.wData = 0;
			SafeStrCpy( msg.Msg, bufmsg, sizeof(msg.Msg));
			g_Network.Broadcast2AgentServer((char*)&msg, sizeof(msg));

			FILE* fpLogx = fopen( fnamex, "a+" );
			if( fpLogx )
			{
				fprintf( fpLogx, "<font color='green'>[%02d:%02d][ITEM]: Congratulations! <b>%s</b> has win Jackpot 77<b>%f</b></font>\n", sysTime.wHour, sysTime.wMinute, GetObjectName(), (float)SlotGetmoneyhasil );
				fclose( fpLogx );
			}

		}
		
	}
	if(m_additemachievment == 1)
	{
		ITEMMGR->ObtainGeneralItem(this, 30000532, 1, eLog_ItemObtainMonster, MP_ITEM_MONSTER_OBTAIN_NOTIFY);// blue diamond

	} else if(m_additemachievment == 2)
	{
		ITEMMGR->ObtainGeneralItem(this, 30000532, 1, eLog_ItemObtainMonster, MP_ITEM_MONSTER_OBTAIN_NOTIFY);

	} else if(m_additemachievment == 3)
	{
		ITEMMGR->ObtainGeneralItem(this, 30000532, 1, eLog_ItemObtainMonster, MP_ITEM_MONSTER_OBTAIN_NOTIFY);

	} else if(m_additemachievment == 4)
	{
		ITEMMGR->ObtainGeneralItem(this, 30000532, 1, eLog_ItemObtainMonster, MP_ITEM_MONSTER_OBTAIN_NOTIFY);

	} else if(m_additemachievment == 5)
	{
		ITEMMGR->ObtainGeneralItem(this, 30000532, 1, eLog_ItemObtainMonster, MP_ITEM_MONSTER_OBTAIN_NOTIFY);

	}
	
	m_additemachievment = 0;
    m_pSpinslothasil1 = 0;
	m_pSpinslothasil2 = 0;
	m_pSpinslothasil3 = 0;
	m_pSpinslothasil4 = 0;
	m_wincodehasil = 0;
	SlotGetmoneyhasil = 0;
}

void CPlayer::CheckImmortalTime()
{
	if(g_pServerSystem->GetMapNum() != GTMAPNUM)
		return;

	if(gTickTime<m_dwImmortalTimeOnGTMAP)
		m_dwImmortalTimeOnGTMAP-=gTickTime;
	else
	{
		m_dwImmortalTimeOnGTMAP = 0;

		MSG_DWORD2 msg;
		msg.Category = MP_GTOURNAMENT;
		msg.Protocol = MP_GTOURNAMENT_TEAMMEMBER_MORTAL;
		msg.dwObjectID = GetID();
		msg.dwData1 = GetID();
		msg.dwData2 = 0;
		
		PACKEDDATA_OBJ->QuickSend(this,&msg,sizeof(msg));
	}
}

void CPlayer::SetNickName(char* NickName)
{
	SafeStrCpy(m_HeroCharacterInfo.NickName,NickName, MAX_GUILD_NICKNAME+1);
}

void CPlayer::SetMarryPlayerName(char* MarryPlayerName)
{
	SafeStrCpy(m_HeroCharacterInfo.MarryName,MarryPlayerName, 32+1);
}


void CPlayer::SetFamilyNickName(char* NickName)
{
	SafeStrCpy(m_HeroCharacterInfo.FamilyNickName,NickName, sizeof( m_HeroCharacterInfo.FamilyNickName ) );
}

LEVELTYPE CPlayer::GetLevel()
{ 
	return m_HeroCharacterInfo.Level; 
}

DWORD CPlayer::GetLife() 
{ 
	return m_HeroCharacterInfo.Life; 
}

DWORD CPlayer::GetMana()
{ 
	return m_HeroInfo.Mana; 
}

DWORD CPlayer::DoGetMaxLife()
{ 
	return m_HeroCharacterInfo.MaxLife; 
}

DWORD CPlayer::DoGetMaxMana()
{ 
	return m_HeroInfo.MaxMana; 
}

void CPlayer::SetStage( BYTE grade, BYTE index )
{
	MSG_BYTE2 msg ;

	msg.Category	= MP_CHAR ;
	msg.Protocol	= MP_CHAR_STAGE_NOTIFY ;
	msg.dwObjectID	= GetID() ;
	msg.bData1		= grade ;
	msg.bData2		= index ;
	PACKEDDATA_OBJ->QuickSend( this, &msg, sizeof(msg) );

	CharacterTotalInfoUpdate( this );	
}

WORD CPlayer::GetJobCodeForGT ()
{
	WORD JobCategory = m_HeroCharacterInfo.Job[0];
	WORD JobGrade = m_HeroCharacterInfo.JobGrade;
	WORD JobIndex = m_HeroCharacterInfo.Job[JobGrade - 1];

	if(JobGrade == 1)
		JobIndex = 1;

	WORD JobCode = JobCategory*100 + JobGrade*10 + JobIndex;

	return JobCode;
}

// 070415 LYW --- Plsyer : Add function to setting job.
void CPlayer::SetJob( BYTE jobGrade, BYTE jobIdx )
{
	MSG_BYTE2 msg ;

	msg.Category	= MP_CHAR ;
	msg.Protocol	= MP_CHAR_STAGE_NOTIFY ;
	msg.dwObjectID	= GetID() ;
	msg.bData1		= jobGrade ;
	msg.bData2		= jobIdx ;

	// 070522 LYW --- Player : Modified send job notify.
	//PACKEDDATA_OBJ->QuickSend( this, &msg, sizeof(msg) );
	SendMsg( &msg, sizeof(msg) );

	m_HeroCharacterInfo.JobGrade = jobGrade ;
	m_HeroCharacterInfo.Job[jobGrade - 1] = jobIdx ;

	CharacterJobUpdate( GetID(), jobGrade,
						m_HeroCharacterInfo.Job[0],
						m_HeroCharacterInfo.Job[1],
						m_HeroCharacterInfo.Job[2],
						m_HeroCharacterInfo.Job[3],
						m_HeroCharacterInfo.Job[4],
						m_HeroCharacterInfo.Job[5] );


	// 071112 ����, Ŭ���� �α׸� �����
	InsertLogJob( this, m_HeroCharacterInfo.Job[0], jobGrade, jobIdx );

	// 081022 KTH -- 
	CHARCALCMGR->AddPlayerJobSkill(this);

	WebEvent( GetUserID(), 2 );


	// 100113 ONS �ֹε�������� Ŭ���������� ����Ǿ��� ��� ������Ʈ�� �����Ѵ�.
	CHARACTER_TOTALINFO TotalInfo;
	ZeroMemory(&TotalInfo, sizeof(TotalInfo));
	GetCharacterTotalInfo(&TotalInfo);

	WORD idx = 1;
	if( TotalInfo.JobGrade > 1 )
	{
		idx = TotalInfo.Job[TotalInfo.JobGrade - 1];
	}
	DWORD dwClass = ( TotalInfo.Job[0] * 1000 ) + ( ( TotalInfo.Race + 1 ) * 100 ) + ( TotalInfo.JobGrade * 10 ) + idx;

	MSG_DWORD Packet;
	ZeroMemory(&Packet, sizeof(Packet));
	Packet.Category	= MP_RESIDENTREGIST;
	Packet.Protocol	= MP_RESIDENTREGIST_REGIST_CHANGE;
	Packet.dwObjectID = GetID();
	Packet.dwData = dwClass;
	g_Network.Broadcast2AgentServer( ( char* )&Packet, sizeof( Packet ) );

	// 080225 LUJ, ��� ȸ���� ��� ���� ���� ������ �� ������ �����ؾ��Ѵ�
	{
		CGuild* guild = GUILDMGR->GetGuild( GetGuildIdx() );

		if( ! guild )
		{
			return;
		}

		GUILDMEMBERINFO* member = guild->GetMemberInfo( GetID() );

		if( ! member )
		{
			return;
		}

		member->mJobGrade	= jobGrade;
		member->mRace		= m_HeroCharacterInfo.Race;
		memcpy( member->mJob, m_HeroCharacterInfo.Job, sizeof( member->mJob ) );

		SEND_GUILD_MEMBER_INFO message;
		message.Category	= MP_GUILD;
		message.Protocol	= MP_GUILD_SET_MEMBER_TO_MAP;
		message.GuildIdx	= GetGuildIdx();
		message.MemberInfo	= *member;

		g_Network.Send2AgentServer( ( char* )&message, sizeof( message ) );

		GUILDMGR->NetworkMsgParse( message.Protocol, &message );
	}	
}

void CPlayer::SendPlayerToMap(MAPTYPE mapNum, float xpos, float zpos)
{
	MSG_DWORD3 msg ;														// �޽��� ����ü�� �����Ѵ�.
	memset(&msg, 0, sizeof(MSG_DWORD3)) ;									// �޽��� �ʱ�ȭ.

	msg.Category	= MP_USERCONN ;
	msg.Protocol	= MP_USERCONN_QUEST_CHANGEMAP_SYN ;						// ī�װ��� ���������� �����Ѵ�.

	msg.dwObjectID	= GetID() ;												// ������Ʈ ���̵� �����Ѵ�.

	msg.dwData1		= (DWORD)mapNum ;										// ���� ������ �� ��ȣ�� �����Ѵ�.
	msg.dwData2		= (DWORD)xpos ;											// X��ǥ�� �����Ѵ�.
	msg.dwData3		= (DWORD)zpos ;											// Z��ǥ�� �����Ѵ�.

	SendMsg( &msg, sizeof(MSG_DWORD3) ) ;									// �޽����� �����Ѵ�.
}

DOUBLE CPlayer::GetPlayerTotalExpPoint()
{
	DOUBLE exp = 0;

	for(LEVELTYPE i=1; i<GetLevel(); ++i)
	{
		exp = exp + GAMERESRCMNGR->GetMaxExpPoint( i ) ;
	}

	exp += GetPlayerExpPoint();

	return exp;
}

void CPlayer::InitGuildUnionInfo( DWORD dwGuildUnionIdx, char* pGuildUnionName, DWORD dwMarkIdx )
{
	m_HeroCharacterInfo.dwGuildUnionIdx = dwGuildUnionIdx;
	strncpy( m_HeroCharacterInfo.sGuildUnionName, pGuildUnionName, MAX_GUILD_NAME+1 );
	m_HeroCharacterInfo.dwGuildUnionMarkIdx = dwMarkIdx;
}

void CPlayer::SetGuildUnionInfo( DWORD dwGuildUnionIdx, char* pGuildUnionName, DWORD dwMarkIdx )
{
	m_HeroCharacterInfo.dwGuildUnionIdx = dwGuildUnionIdx;
	strncpy( m_HeroCharacterInfo.sGuildUnionName, pGuildUnionName, sizeof( m_HeroCharacterInfo.sGuildUnionName ) );
	m_HeroCharacterInfo.dwGuildUnionMarkIdx = dwMarkIdx;

	MSG_NAME_DWORD3 Msg;
	Msg.Category = MP_GUILD_UNION;
	Msg.Protocol = MP_GUILD_UNION_PLAYER_INFO;
	Msg.dwData1 = GetID();
	Msg.dwData2 = dwGuildUnionIdx;
	Msg.dwData3 = dwMarkIdx;
	strncpy( Msg.Name, pGuildUnionName, sizeof( Msg.Name ) );

	PACKEDDATA_OBJ->QuickSendExceptObjectSelf( this, &Msg, sizeof(Msg) );
}

float CPlayer::GetAccuracy()
{
	return mAccuracy;
}

float CPlayer::GetAvoid()
{
	return mAvoid;
}

// 100221 ShinJS --- ������ ����
float CPlayer::GetBlock()
{
	const float rate	= mRateBuffStatus.Block + mRatePassiveStatus.Block;
	float		bonus	= 0;

	switch( GetJop( 0 ) )
	{
		// 080910 LUJ, ������
	case 1:
		{
			bonus = 15.f;
			break;
		}
		// 080910 LUJ, �α�
	case 2:
		{
			bonus = 10.f;
			break;
		}
		// 080910 LUJ, ������
	case 3:
		{
			bonus = 5.f;
			break;
		}
		// 100218 ShinJS --- ����
	case 4:
		{
			bonus = 9.f;
			break;
		}
	}
	// Decrese Block Chance From Devide by 27 to devide by 270
	return ( float( GetDexterity() / 270.f ) + rate + bonus );
}

float CPlayer::GetPhysicAttackMax()
{
	return mPhysicAttackMax;
}

float CPlayer::GetPhysicAttackMin()
{
	return mPhysicAttackMin;
}

float CPlayer::GetPhysicAttack()
{
	return float( random( mPhysicAttackMin, mPhysicAttackMax ) );;
}

float CPlayer::GetMagicAttackMax()
{
	return mMagicAttackMax;
}

float CPlayer::GetMagicAttackMin()
{
	return mMagicAttackMin;
}

float CPlayer::GetMagicAttack()
{
	return float( random( mMagicAttackMin, mMagicAttackMax ) );;
}

float CPlayer::GetPhysicDefense()
{
	return mPhysicDefense;
}

float CPlayer::GetMagicDefense()
{
	return mMagicDefense;
}

float CPlayer::GetCriticalRate()
{
	return mCriticalRate;
}

float CPlayer::GetCriticalDamageRate()
{
	return mCriticalDamageRate;
}

float CPlayer::GetCriticalDamagePlus()
{
	return mCriticalDamagePlus;
}

float CPlayer::GetMagicCriticalRate()
{
	return mMagicCriticalRate;
}

float CPlayer::GetMagicCriticalDamageRate()
{
	return mMagicCriticalDamageRate;
}

float CPlayer::GetMagicCriticalDamagePlus()
{
	return mMagicCriticalDamagePlus;
}

float CPlayer::GetLifeRecover()
{
	return mLifeRecover;
}

float CPlayer::GetManaRecover()
{
	return mManaRecover;
}

////////////////////////////////////////////////////////////////////////////
// 06. 07. ���°������� - �̿���
// �����̻� ��ų �����н� Ư���� ���¸� ������ ������ ���´� ��� �ʱ�ȭ
// �������� ���¿����� ���� �ʱ�ȭ
// ���а��� �������� �ʴ´�.
// ���� ���нÿ��� ��������
BOOL CPlayer::CanSkillState()
{
	//���� 5ȸ �̻� ���н�
	if(m_SkillFailCount >= 5)
	{
		switch(m_BaseObjectInfo.ObjectState)
		{
		// ��ų�� ����Ҽ� �ִ� ���¸� ���� �ʱ�ȭ�ϰ� TRUE ����
		case eObjectState_None:
		case eObjectState_Move:
		case eObjectState_TiedUp_CanSkill:
			{
				m_SkillFailCount = 0;
				return TRUE;
			}
			break;
		// ��ų�� ����Ҽ� ���� ������ Ǯ���൵ ������ ���¸� ���� �ʱ�ȭ �ϰ�
		// ���¸� �ʱ�ȭ �ѵ� TRUE ����
		case eObjectState_SkillStart:
		case eObjectState_SkillSyn:	
		case eObjectState_SkillBinding:
		case eObjectState_SkillUsing:
		case eObjectState_SkillDelay:
		case eObjectState_Society:
		case eObjectState_Enter:
		case eObjectState_Rest:
			{
				m_SkillFailCount = 0;
				m_BaseObjectInfo.ObjectState = eObjectState_None;
				return TRUE;
			}
			break;
		// �� ���� ��쿣 ���� �ʱ�ȭ �ϰ� FALSE ����
		default:
			{
				m_SkillFailCount = 0;
				return FALSE;
			}
			break;
		}
	}

	// 5ȸ ���� �϶� ��ų�� ����Ҽ� ���� ���¸�
	// ���� �����ϰ� FALSE ����
	if(m_BaseObjectInfo.ObjectState != eObjectState_None &&
	   m_BaseObjectInfo.ObjectState != eObjectState_Move &&
	   m_BaseObjectInfo.ObjectState != eObjectState_TiedUp_CanSkill )
	{
		m_SkillFailCount++;
		return FALSE;
	}

	// ���� �����϶� ���� �ʱ�ȭ�ϰ� TRUE ����
	m_SkillFailCount = 0;
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////

BOOL CPlayer::AddQuick( BYTE sheet, WORD pos, SLOT_INFO* pSlot )
{
	return m_QuickSlot[ sheet ].Update( pos, pSlot );
}

SLOT_INFO*	CPlayer::GetQuick( BYTE sheet, WORD pos )
{
	return m_QuickSlot[ sheet ].GetQuick( pos );
}

// desc_hseos_���͹���01
// S ���͹��� �߰� added by hseos 2007.05.23	2007.07.08
void CPlayer::ProcMonstermeterPlayTime()
{
	if (gCurTime - m_stMonstermeterInfo.nPlayTimeTick > SHMath_MINUTE(1))
	{
		m_stMonstermeterInfo.nPlayTimeTick = gCurTime;
		m_stMonstermeterInfo.nPlayTime++;
		m_stMonstermeterInfo.nPlayTimeTotal++;

		// DB�� ����
		//MonsterMeter_Save(GetID(), m_stMonstermeterInfo.nPlayTime, m_stMonstermeterInfo.nKillMonsterNum, m_stMonstermeterInfo.nPlayTimeTotal, m_stMonstermeterInfo.nKillMonsterNumTotal);

		// �÷��̽ð��� Ŭ���̾�Ʈ������ ��� �����ϹǷ� Ŭ���̾�Ʈ���� ����ϵ��� �ϰ�
		// Ȥ�� �� ���������� 10�и��� ������ ��ġ�� �����ش�.
		if ((m_stMonstermeterInfo.nPlayTime%10) == 0)
		{
			// Ŭ���̾�Ʈ�� �˸���
			MSG_DWORD2 msg;
			msg.Category = MP_CHAR;
			msg.Protocol = MP_CHAR_MONSTERMETER_PLAYTIME;
			msg.dwObjectID = GetID();
			msg.dwData1 = m_stMonstermeterInfo.nPlayTime;
			msg.dwData2 = m_stMonstermeterInfo.nPlayTimeTotal;
			SendMsg(&msg, sizeof(msg));
		}

		if( m_stMonstermeterInfo.nPlayTimeTotal == 30 * 60 )
		{
			WebEvent( GetUserID(), 6 );
		}
		// ���� ó��
		g_csMonstermeterManager.ProcessReward(this, CSHMonstermeterManager::RBT_PLAYTIME, m_stMonstermeterInfo.nPlayTimeTotal);
	}
}
// E ���͹��� �߰� added by hseos 2007.05.23	2007.07.08

// desc_hseos_���͹���01
// S ���͹��� �߰� added by hseos 2007.05.23	2007.07.08
void CPlayer::ProcMonstermeterKillMon()
{
	m_stMonstermeterInfo.nKillMonsterNum++;
	m_stMonstermeterInfo.nKillMonsterNumTotal++;
	m_VerifyKillCount++;

	// DB�� ����
	//MonsterMeter_Save(GetID(), m_stMonstermeterInfo.nPlayTime, m_stMonstermeterInfo.nKillMonsterNum, m_stMonstermeterInfo.nPlayTimeTotal, m_stMonstermeterInfo.nKillMonsterNumTotal);

	// Ŭ���̾�Ʈ�� �˸���
	MSG_DWORD2 msg;
	msg.Category = MP_CHAR;
	msg.Protocol = MP_CHAR_MONSTERMETER_KILLMONSTER;
	msg.dwObjectID = GetID();
	msg.dwData1 = m_stMonstermeterInfo.nKillMonsterNum;
	msg.dwData2 = m_stMonstermeterInfo.nKillMonsterNumTotal;
	SendMsg(&msg, sizeof(msg));

	if( m_stMonstermeterInfo.nKillMonsterNumTotal == 1000 )
	{
		WebEvent( GetUserID(), 9 );
	}
	
	if(m_VerifyKillCount > 500 && m_VerifyCaptcha == 0) 
	{
		m_VerifyCaptcha = random(1000, 9999);

		MSG_DWORD2 msg;
		msg.Category = MP_USERCONN;
		msg.Protocol = MP_USERCONN_CAPTCHA_SYN;
		msg.dwObjectID = GetID();
		msg.dwData1 = m_VerifyCaptcha;
		msg.dwData2 = 2;
		SendMsg( &msg, sizeof(msg) );
	}
	// ���� ó��
	g_csMonstermeterManager.ProcessReward(this, CSHMonstermeterManager::RBT_MONSTERKILL, m_stMonstermeterInfo.nKillMonsterNumTotal);
}

void CPlayer::ProcFarmTime()
{
	if (m_stFarmInfo.nCropPlantRetryTime)
	{
		if (gCurTime - m_stFarmInfo.nCropPlantRetryTimeTick > SHMath_SECOND(1))
		{
			m_stFarmInfo.nCropPlantRetryTimeTick = gCurTime;
			m_stFarmInfo.nCropPlantRetryTime--;

			// DB�� ����
			Farm_SetTimeDelay(GetID(), CSHFarmManager::FARM_TIMEDELAY_KIND_PLANT, m_stFarmInfo.nCropPlantRetryTime);
		}
	}

	if (m_stFarmInfo.nCropManureRetryTime)
	{
		if (gCurTime - m_stFarmInfo.nCropManureRetryTimeTick > SHMath_SECOND(1))
		{
			m_stFarmInfo.nCropManureRetryTimeTick = gCurTime;
			m_stFarmInfo.nCropManureRetryTime--;

			// DB�� ��?			Farm_SetTimeDelay(GetID(), CSHFarmManager::FARM_TIMEDELAY_KIND_MANURE, m_stFarmInfo.nCropManureRetryTime);
		}
	}

	// 080430 KTH Animal Delay Add  (��)������ ���⼭ ��ġ�� Decrease ���ִ±�...
	if( m_stFarmInfo.nAnimalCleanRetryTime )
	{
		if( gCurTime - m_stFarmInfo.nAnimalCleanRetryTimeTick > SHMath_MINUTE(1) )
		{
			m_stFarmInfo.nAnimalCleanRetryTimeTick = gCurTime;
			m_stFarmInfo.nAnimalCleanRetryTime--;

			Farm_SetTimeDelay(GetID(), CSHFarmManager::FARM_TIMEDELAY_KIND_CLEAN, m_stFarmInfo.nAnimalCleanRetryTime);
		}
	}

	if( m_stFarmInfo.nAnimalFeedRetryTime )
	{
		if( gCurTime - m_stFarmInfo.nAnimalFeedRetryTimeTick > SHMath_SECOND(1) )
		{
			m_stFarmInfo.nAnimalFeedRetryTimeTick = gCurTime;
			m_stFarmInfo.nAnimalFeedRetryTime--;

			Farm_SetTimeDelay(GetID(), CSHFarmManager::FARM_TIMEDELAY_KIND_FEED, m_stFarmInfo.nAnimalFeedRetryTime);
		}
	}
}
// E ����ý��� �߰� added by hseos 2007.08.23

BOOL CPlayer::IsInventoryPosition( POSTYPE position )
{
	const CItemSlot* slot = GetSlot( eItemTable_Inventory );

    const POSTYPE begin = slot->GetStartPos();
	const POSTYPE end	= slot->GetSlotNum() + begin;

	return begin <= position && end >= position;
}


void CPlayer::ResetSetItemStatus()
{
	mSetItemLevel.clear();
	ZeroMemory(
		&m_SetItemStats,
		sizeof(m_SetItemStats));
}


const CPlayer::SetItemLevel& CPlayer::GetSetItemLevel() const
{
	return mSetItemLevel;
}


CPlayer::SetItemLevel& CPlayer::GetSetItemLevel()
{
	return mSetItemLevel;
}

void CPlayer::AddJobSkill( DWORD skillIndex, BYTE level )
{
	SKILL_BASE SkillBase;
	SkillBase.dwDBIdx = 0;
	SkillBase.wSkillIdx = skillIndex;
	SkillBase.Level = level;
	m_JobSkillList.push_back(SkillBase);

	AddSetSkill(skillIndex, level);
}

void CPlayer::ClearJobSkill()
{
	for(std::list< SKILL_BASE >::iterator iterator = m_JobSkillList.begin();
		iterator != m_JobSkillList.end();
		++iterator)
	{
		SKILL_BASE* skill = m_SkillTree->GetData(iterator->wSkillIdx);

		if( skill )
		{
			RemoveSetSkill(
				skill->wSkillIdx,
				skill->Level);
		}
	}

	m_JobSkillList.clear();
}

void CPlayer::AddSetSkill(DWORD skillIndex, LEVELTYPE level)
{
	SKILL_BASE skill = {0};
	skill.wSkillIdx = skillIndex;
	skill.Level = level;

	const SKILL_BASE* const oldSkill = m_SkillTree->GetData(
		skillIndex);

	if(oldSkill)
	{
		skill = *oldSkill;
		skill.Level = skill.Level + level;
	}

	m_SkillTree->Update(
		skill);
}


void CPlayer::RemoveSetSkill(DWORD skillIndex, LEVELTYPE level)
{
	SKILL_BASE* const oldSkill = m_SkillTree->GetData(
		skillIndex);

	if(0 == oldSkill)
	{
		return;
	}

	SKILL_BASE skill = *oldSkill;
	skill.Level = skill.Level - level;

	m_SkillTree->Update(
		skill);
}

// 090217 LUJ, ������ �µ��� �Լ� �̸� ����
void CPlayer::SetHideLevel( WORD level )
{ 
	m_HeroCharacterInfo.HideLevel = level; 
	
	if( level )
	{
		m_HeroCharacterInfo.bVisible = false;
	}
	else
	{
		m_HeroCharacterInfo.bVisible = true;
	}

	MSG_WORD msg;
	msg.Category = MP_CHAR;
	msg.Protocol = MP_CHAR_HIDE_NOTIFY;
	msg.dwObjectID = GetID();
	msg.wData = level;

	PACKEDDATA_OBJ->QuickSend( this, &msg, sizeof( msg ) );	
}

// 090217 LUJ, ������ �°� �Լ� �̸� ����
void CPlayer::SetDetectLevel( WORD level )
{ 
	m_HeroCharacterInfo.DetectLevel = level; 
	
	MSG_WORD msg;
	msg.Category = MP_CHAR;
	msg.Protocol = MP_CHAR_DETECT_NOTIFY;
	msg.dwObjectID = GetID();
	msg.wData = level;

	PACKEDDATA_OBJ->QuickSend( this, &msg, sizeof( msg ) );	
}	

void CPlayer::RemoveAllAggroed()
{
	while(false == mAggroObjectContainer.empty())
	{
		const ObjectIndex objectIndex = *(mAggroObjectContainer.begin());
		mAggroObjectContainer.erase(
			objectIndex);

		CMonster* const monster = (CMonster*)g_pUserTable->FindUser(
			objectIndex);

		if(0 == monster ||
			FALSE == (eObjectKind_Monster & monster->GetObjectKind()))
		{
			continue;
		}

		monster->RemoveFromAggro(
			GetID());
		monster->GetAbnormalStatus()->Attract = 0;
	}
}

void CPlayer::AddAggroToMyMonsters(int nAggroAdd, DWORD targetObjectIndex, DWORD skillIndex)
{
	for(ObjectIndexContainer::const_iterator iterator = mAggroObjectContainer.begin();
		mAggroObjectContainer.end() != iterator;
		++iterator)
	{
		const ObjectIndex objectIndex = *iterator;
		CMonster* const monster = (CMonster*)g_pUserTable->FindUser(
			objectIndex);

		if(0 == monster ||
			FALSE == (eObjectKind_Monster & monster->GetObjectKind()))
		{
			continue;
		}

		monster->AddAggro(
			targetObjectIndex,
			nAggroAdd,
			skillIndex);
	}
}

// 080910 LUJ, ������ ������ ��ȯ�Ѵ�
DWORD CPlayer::GetShieldDefense()
{
	return mShieldDefense;
}

// 080910 LUJ, ������ ������ �����Ѵ�
void CPlayer::SetShieldDefence( DWORD shieldDefense )
{
	mShieldDefense = shieldDefense;
}

BOOL CPlayer::AddCoolTime( DWORD coolTimeGroupIndex, DWORD coolTime )
{
	ProcCoolTime();

	if(mCoolTimeMap.end() != mCoolTimeMap.find( coolTimeGroupIndex ))
	{
		return FALSE;
	}

	CoolTime& time = mCoolTimeMap[ coolTimeGroupIndex ];
	const DWORD tick = GetTickCount();

	time.mBeginTick = tick;
	time.mEndTick	= tick + coolTime;

	return TRUE;
}

void CPlayer::RemoveCoolTime( DWORD coolTimeGroupIndex)
{
	mCoolTimeMap.erase(
		coolTimeGroupIndex);
}


void CPlayer::ProcCoolTime()
{
	if(true == mCoolTimeMap.empty())
	{
		return;
	}

	const DWORD tick = GetTickCount();

	std::set< DWORD > group;

	for(CoolTimeMap::iterator it = mCoolTimeMap.begin(); mCoolTimeMap.end() != it; ++it)
	{
		const CoolTime& time = it->second;

		if( ( time.mBeginTick < time.mEndTick && time.mEndTick < tick ) ||
			( time.mBeginTick > time.mEndTick && time.mBeginTick > tick && time.mEndTick < tick ) )
		{
			// ����: ������ ���Ŀ��� �ݺ��ڰ� �߸��ȴٴ� ����� ���. ���� ���� ���μ������� ó���ؾ� �Ѵ�.
			group.insert( it->first );
		}
	}

	for( std::set< DWORD > ::const_iterator it = group.begin(); group.end() != it; ++it )
	{
		mCoolTimeMap.erase(*it);
	}
}

DWORD CPlayer::GetVitality() 
{ 
	const float rate =
		mRatePassiveStatus.Vit +
		mRateBuffStatus.Vit +
		m_itemBaseStats.mVitality.mPercent +
		m_itemOptionStats.mVitality.mPercent +
		m_SetItemStats.mVitality.mPercent * 100.f;
	const float plus =
		mPassiveStatus.Vit +
		mBuffStatus.Vit +
		m_itemBaseStats.mVitality.mPlus +
		m_itemOptionStats.mVitality.mPlus +
		m_SetItemStats.mVitality.mPlus;
	float Result = m_charStats.mVitality.mPlus * (1.f + rate / 100.f) + plus;

	if( Result < 0 )
	{
		Result = 0;
	}

	return (DWORD)( Round( Result, 1 ) );
}

DWORD CPlayer::GetWisdom() 
{ 
	const float	rate =
		mRatePassiveStatus.Wis +
		mRateBuffStatus.Wis +
		m_itemBaseStats.mWisdom.mPercent +
		m_itemOptionStats.mWisdom.mPercent +
		m_SetItemStats.mWisdom.mPercent * 100.f;
	const float plus =
		mPassiveStatus.Wis +
		mBuffStatus.Wis +
		m_itemBaseStats.mWisdom.mPlus +
		m_itemOptionStats.mWisdom.mPlus +
		m_SetItemStats.mWisdom.mPlus;
	float Result = m_charStats.mWisdom.mPlus * (1.f + rate / 100.f) + plus;

	if( Result < 0 )
	{
		Result = 0;
	}

	return (DWORD)( Round( Result, 1 ) );
}

DWORD CPlayer::GetStrength() 
{
	const float	rate =
		mRatePassiveStatus.Str +
		mRateBuffStatus.Str +
		m_itemBaseStats.mStrength.mPercent +
		m_itemOptionStats.mStrength.mPercent +
		m_SetItemStats.mStrength.mPercent * 100.f;
	const float	plus =
		mPassiveStatus.Str +
		mBuffStatus.Str +
		m_itemBaseStats.mStrength.mPlus +
		m_itemOptionStats.mStrength.mPlus +
		m_SetItemStats.mStrength.mPlus;
	float Result = m_charStats.mStrength.mPlus * (1.f + rate / 100.f) + plus;

	if( Result < 0 )
	{
		Result = 0;
	}

	return (DWORD)( Round( Result, 1 ) );
}

DWORD CPlayer::GetDexterity() 
{ 
	const float rate =
		mRatePassiveStatus.Dex +
		mRateBuffStatus.Dex +
		m_itemBaseStats.mDexterity.mPercent +
		m_itemOptionStats.mDexterity.mPercent +
		m_SetItemStats.mDexterity.mPercent * 100.f;
	const float plus =
		mPassiveStatus.Dex +
		mBuffStatus.Dex +
		m_itemBaseStats.mDexterity.mPlus +
		m_itemOptionStats.mDexterity.mPlus +
		m_SetItemStats.mDexterity.mPlus;
	float Result = m_charStats.mDexterity.mPlus * (1.f + rate / 100.f) + plus;

	if( Result < 0 )
	{
		Result = 0;
	}

	return (DWORD)( Round( Result, 1 ) );
}

DWORD CPlayer::GetIntelligence() 
{
	const float rate =
		mRatePassiveStatus.Int +
		mRateBuffStatus.Int +
		m_itemBaseStats.mIntelligence.mPercent +
		m_itemOptionStats.mIntelligence.mPercent +
		m_SetItemStats.mIntelligence.mPercent * 100.f;
	const float plus =
		mPassiveStatus.Int +
		mBuffStatus.Int +
		m_itemBaseStats.mIntelligence.mPlus +
		m_itemOptionStats.mIntelligence.mPlus +
		m_SetItemStats.mIntelligence.mPlus;
	float Result = m_charStats.mIntelligence.mPlus * (1.f + rate / 100.f) + plus;

	if( Result < 0 )
	{
		Result = 0;
	}

	return (DWORD)( Round( Result, 1 ) );
}

void CPlayer::SetObjectBattleState(eObjectBattleState state)
{ 
	m_BaseObjectInfo.ObjectBattleState = state; 

	if( state )	//eObjectBattleState_Battle
	{
		mpBattleStateDelay->Start();
	}
}

void CPlayer::ProcessTimeCheckItem( BOOL bForceDBUpdate )
{
	DWORD dwElapsedMili = gCurTime - m_dwLastTimeCheckItem;
	if(  dwElapsedMili < 60 * 1000 && !bForceDBUpdate )
		return;

	DWORD dwElapsedSecond = dwElapsedMili / 1000;

	m_dwLastTimeCheckItem = gCurTime - ( dwElapsedMili - dwElapsedSecond * 1000 ) ;

	BOOL bDBUpdate = bForceDBUpdate ? TRUE : FALSE;

	if( gCurTime - m_dwLastTimeCheckItemDBUpdate > 5 * 60 * 1000 )
	{
		bDBUpdate = TRUE;
		m_dwLastTimeCheckItemDBUpdate = gCurTime;
	}

	int iCheckItemMaxNum = TP_WEAR_END;
	if( IsGotWarehouseItems() )
	{
		iCheckItemMaxNum = TP_STORAGE_END;
	}

	for( POSTYPE i = TP_INVENTORY_START ; i <= iCheckItemMaxNum ; ++i )
	{
		ITEMBASE* pItemBase = (ITEMBASE*)m_ItemContainer.GetItemInfoAbs( i );
		const ITEM_INFO* info = ITEMMGR->GetItemInfo( pItemBase->wIconIdx );

		if(0 == info)
		{
			continue;
		}
		else if(info->SupplyType == ITEM_KIND_COOLTIME )
		{
			pItemBase->nRemainSecond = max( 0, int( pItemBase->nRemainSecond - dwElapsedSecond ) );
			pItemBase->ItemParam = gCurTime;

			UpdateRemainTime(
				GetID(),
				pItemBase->dwDBIdx,
				pItemBase->nSealed,
				pItemBase->nRemainSecond);
		}
		else if( pItemBase->nSealed == eITEM_TYPE_UNSEAL )
		{
			if( pItemBase->nRemainSecond > (int)dwElapsedSecond )
			{
				pItemBase->nRemainSecond -= dwElapsedSecond;

				// 071125 KTH --- Player "RemainSecond�� 1�� �̸��� ��� Ŭ���̾�Ʈ���� ������ �����ش�."
				if( pItemBase->nRemainSecond <= 60 )
				{
					MSG_DWORD2 msg;
					msg.Category = MP_ITEM;
					msg.Protocol = MP_ITEM_TIMELIMT_ITEM_ONEMINUTE;
					msg.dwData1 = pItemBase->wIconIdx;
					msg.dwData2 = pItemBase->Position;
	
					SendMsg(&msg, sizeof(msg));
				}

				if( bDBUpdate )
				{
					if( info->nTimeKind == eKIND_PLAYTIME )
					{
						UpdateRemainTime(
							GetID(),
							pItemBase->dwDBIdx,
							pItemBase->nSealed,
							pItemBase->nRemainSecond);
					}
				}
			}
			else
			{
				POSTYPE	position = pItemBase->Position;
				DWORD	iconIdx = pItemBase->wIconIdx;
				int returnValue = ITEMMGR->DiscardItem(
					this,
					pItemBase->Position,
					pItemBase->wIconIdx,
					pItemBase->Durability);

				if( EI_TRUE == returnValue )
				{
					CVehicle* const vehicle = ( CVehicle* )g_pUserTable->FindUser( GetSummonedVehicle() );

					// 090316 LUJ, �ش� ���������� ��ȯ�� Ż ���� ��ȯ �����Ѵ�
					if( vehicle &&
						vehicle->GetObjectKind() == eObjectKind_Vehicle )
					{
						const ICONBASE& useItem = vehicle->GetUseItem();

						if( useItem.wIconIdx == iconIdx )
						{
							vehicle->DoDie( 0 );
						}
					}

					MSG_ITEM_DISCARD_SYN msg;
					ZeroMemory(&msg, sizeof(msg));
					msg.Category = MP_ITEM;
					msg.Protocol = MP_ITEM_DISCARD_ACK;
					msg.TargetPos = position;
					msg.wItemIdx = iconIdx;
					msg.ItemNum = 1;
					SendMsg(&msg, sizeof(msg));	

					LogItemMoney(
						GetID(),
						GetObjectName(),
						0,
						"",
						eLog_ShopItemUseEnd,
						GetMoney(),
						0,
						0,
						iconIdx,
						pItemBase->dwDBIdx,
						pItemBase->Position,
						0,
						1,
						GetPlayerExpPoint() );
				}
			}
		}
	}

	// NYJ - �ð��� �丮������ �ð��Ҹ�
	ProcessRecipeTimeCheck(dwElapsedMili);

	{
		CPet* const petObject = PETMGR->GetPet(
			GetPetItemDbIndex());

		if(petObject)
		{
			petObject->ProcessTimeCheckItem(
				bDBUpdate,
				dwElapsedMili);
		}
	}

	// 100525 NYJ - �ǸŴ��� ��ϻ�ǰ�� ���� �ð����üũ ����
	Consignment_CheckDate(GetID());
	Note_CheckDate(GetID());
}

// desc_hseos_��ȥ_01
// S ��ȥ �߰� added by hseos 2008.01.29
BOOL CPlayer::RemoveItem(DWORD nItemID, DWORD nItemNum, eLogitemmoney eLogKind)
{
	int iCheckItemMaxNum = TP_WEAR_END;
	if( IsGotWarehouseItems() )
	{
		iCheckItemMaxNum = TP_STORAGE_END;
	}

	for( POSTYPE i = TP_INVENTORY_START ; i <= iCheckItemMaxNum ; ++i )
	{
		ITEMBASE* pItemBase = (ITEMBASE*)m_ItemContainer.GetItemInfoAbs( i );

		if( pItemBase->wIconIdx == nItemID )
		{
			BOOL dbidx = pItemBase->dwDBIdx;
			POSTYPE	position = pItemBase->Position;
			DWORD	iconIdx = pItemBase->wIconIdx;
			int nResult = ITEMMGR->DiscardItem( this, pItemBase->Position, pItemBase->wIconIdx, nItemNum );
			if (nResult != EI_TRUE)
			{
				return FALSE;
			}

			LogItemMoney(
				GetID(),
				GetObjectName(),
				0,
				"",
				eLogKind,
				GetMoney(),
				0,
				0,
				iconIdx,
				dbidx,
				position,
				0,
				nItemNum,
				0);

			MSG_ITEM_DISCARD_SYN msg;
			ZeroMemory(&msg, sizeof(msg));
			msg.Category = MP_ITEM;
			msg.Protocol = MP_ITEM_DISCARD_ACK;
			msg.TargetPos = position;
			msg.wItemIdx = iconIdx;
			msg.ItemNum = nItemNum;
			SendMsg(&msg, sizeof(msg));	
		}
	}

	return TRUE;
}
// E ��ȥ �߰� added by hseos 2008.01.29


BOOL CPlayer::IncreaseInventoryExpansion()
{
	if( GetInventoryExpansion() >= 2 )
		return FALSE;

	IncreaseCharacterInventory(this->GetID());

	return TRUE;
}

void CPlayer::PassiveSkillCheckForWeareItem()
{
	memset( &mPassiveStatus, 0, sizeof( Status ) );
	memset( &mRatePassiveStatus, 0, sizeof( Status ) );

	m_SkillTree->SetPositionHead();

	for(SKILL_BASE* pSkillBase = m_SkillTree->GetData();
		0 < pSkillBase;
		pSkillBase = m_SkillTree->GetData())
	{
		const DWORD skillLevel = min(
			pSkillBase->Level,
			SKILLMGR->GetSkillSize(pSkillBase->wSkillIdx));
		const cActiveSkillInfo* const pSkill = SKILLMGR->GetActiveInfo( 
			pSkillBase->wSkillIdx - 1 + skillLevel);
		
		if(0 == pSkill)
		{
			continue;
		}
		else if(SKILLKIND_PASSIVE != pSkill->GetKind())
		{
			continue;
		}

		for(DWORD index = 0; index < MAX_BUFF_COUNT; ++index)
		{
			const cBuffSkillInfo* const pSkillInfo = ( cBuffSkillInfo* )SKILLMGR->GetBuffInfo(
				pSkill->GetInfo().Buff[index]);

			if(0 == pSkillInfo)
			{
				break;
			}
			else if(FALSE == IsEnable(pSkillInfo->GetInfo()))
			{
				continue;
			}

			pSkillInfo->AddPassiveStatus(
				this);
		}
	}
}

BOOL CPlayer::ClearInventory()
{
	for( POSTYPE i = TP_INVENTORY_START ; i < TP_WEAR_START ; ++i )
	{
		ITEMBASE* pItemBase = (ITEMBASE*)m_ItemContainer.GetItemInfoAbs( i );
		if( pItemBase->dwDBIdx == 0 ) continue;

		int returnValue = ITEMMGR->DiscardItem( this, pItemBase->Position, pItemBase->wIconIdx, pItemBase->Durability );
		if( EI_TRUE == returnValue )
		{
			continue;
		}
	}
	return TRUE;
}

void CPlayer::FishingInfoClear()
{
	SetFishingPlace(0);
	SetFishingStartTime(0);
	SetFishingProcessTime(0);
	SetFishingBait(0);
	ZeroMemory(
		m_fFishBaitRate,
		sizeof(m_fFishBaitRate));
}

void CPlayer::SetFishingExp(EXPTYPE dwExp)
{
	const LEVELTYPE& level = m_wFishingLevel;

	if(MAX_FISHING_LEVEL <= level)
	{
		m_wFishingLevel = MAX_FISHING_LEVEL;
		m_dwFishingExp = 0;
		return;
	}
	
	// ����ġ�� ���� �ܰ迡�� �䱸�ϴ� �ͺ��� �ξ� ���� �� �����Ƿ�,
	// ��� üũ�ؼ� ����������
	{
		EXPTYPE nextPoint = 0 ;
		nextPoint = GAMERESRCMNGR->GetFishingMaxExpPoint( level ) ;

		if(nextPoint == 0)
		{
			return ;
		}

		while( dwExp >= nextPoint )
		{
			if(nextPoint==0)
				break;

			m_wFishingLevel++;

			if(GetLevel() < GAMERESRCMNGR->GetFishingLevelUpRestrict(level))
			{
				m_wFishingLevel--;
				m_dwFishingExp = dwExp = nextPoint;

				MSG_WORD msg;
				msg.Category = MP_FISHING;
				msg.Protocol = MP_FISHING_LEVELUP_NACK;
				msg.wData = m_wFishingLevel;
				SendMsg(&msg, sizeof(msg));	
				break;
			}

			Log_Fishing(
				GetID(),
				eFishingLog_SetLevel,
				0,
				GetFishPoint(),
				0,
				0,
				dwExp - nextPoint,
				GetFishingLevel() );

			// 100607 NYJ ������� �������� ����������
			DWORD dwRewardItem = GAMERESRCMNGR->GetFishingLevelUpReward(level);
			if(dwRewardItem)
			{
				ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(dwRewardItem);
				if(pInfo)
				{
					// 2286, 2287, "2288" �� SystemMsg.bin�� �ε���
					ItemInsertToNote(GetID(), dwRewardItem, 1, pInfo->wSeal, 0, eNoteParsing_FishingLevelUp, 2286, 2287, "2288");
				}
			}

			if( m_wFishingLevel == 2 )
			{
				WebEvent( GetUserID(), 8 );
			}

			MSG_WORD msg;
			msg.Category = MP_FISHING;
			msg.Protocol = MP_FISHING_LEVELUP_ACK;
			msg.wData = m_wFishingLevel;
			SendMsg(&msg, sizeof(msg));	

			dwExp		-=	nextPoint;
			nextPoint	=	GAMERESRCMNGR->GetFishingMaxExpPoint( level );
		}

		m_dwFishingExp = dwExp;
	}
}

// 080509 LUJ, ��ų ��Ÿ���� ������ �ʾ����� ���� ��ȯ�Ѵ�
// 080514 LUJ, ��ų �ִϸ��̼� �ð� üũ
// 080515 LUJ, ��Ÿ�Ӱ� �ִϸ��̼� �ð� üũ�� ���� ��ų�� �󸶳� �����ϴ��� �����ϱ� ���� �α׸� �ۼ��Ѵ�
// 080516 LUJ, ��Ÿ�� üũ ���а� ��� ���� �̻��� �� ������ �����Ŵ
BOOL CPlayer::IsCoolTime( const ACTIVE_SKILL_INFO& skill )
{
	// 080516 LUJ, ��Ÿ�� ���а� �߻��ص� ���� ȸ�� �̻��� ����Ѵ�. �� �̻��� �߻��ϸ� ������ ������ �����Ų��
	struct
	{
		void operator() ( CPlayer& player, CPlayer::CheckCoolTime& checkCoolTime )
		{
			const DWORD maxCheckTick = 1000 * 60;

			// 080516 LUJ, ��Ÿ�� ���а� �߻��� �� ������ �ð��� �������� üũ �����͸� �ʱ�ȭ�Ѵ�
			// 080519 LUJ, maxCheckTick ���� �߻��� ���� üũ�� ���� ���ϴ� �� ����
			if( maxCheckTick < ( checkCoolTime.mCheckedTick - gCurTime ) )
			{
				// 080519 LUJ, üũ �ð��� ���ݺ��� maxCheckTick�������� �Ѵ�
				checkCoolTime.mCheckedTick	= gCurTime + maxCheckTick;
				checkCoolTime.mFailureCount	= 0;
			}

			const DWORD maxCheckCount = 10;

			// 080516 LUJ, ��Ÿ�� ���а� ��� ȸ�� �����̸�, ó������ �ʴ´�
			if( maxCheckCount > ++checkCoolTime.mFailureCount )
			{
				return;
			}

			// 080516 LUJ, ���� ȸ���̻� ������ ��� ������ �����Ų��
			{
				MSG_DWORD message;
				ZeroMemory( &message, sizeof( message ) );
				message.Category	= MP_USERCONN;
				message.Protocol	= MP_USERCONN_GAMEIN_NACK;
				message.dwData		= player.GetID();
				
				g_Network.Broadcast2AgentServer( (char*)&message, sizeof( message ) );

				// 100812 NYJ - �������� �ַܼα׸� ������.
				g_Console.LOG(4, "Force KickOut!! (CoolTime Check Failed.) : ID: %d, NAME: %s",  player.GetID(), player.GetObjectName() );
			}
		}
	}
	Punish;

	// 080514 LUJ, �ִϸ��̼��� ǥ�õǴ� �������� ��ų�� ����� �� ����
	const SkillAnimTimeMap::const_iterator itAnim = mSkillAnimTimeMap.find( skill.Index );
	if(mSkillAnimTimeMap.end() != itAnim &&
		itAnim->second > gCurTime)
	{
		// 080516 LUJ, ��Ÿ�� üũ�� ���� ���� �̻� �������� �� ��Ģ�� �ο��Ѵ�
		Punish( *this, mCheckCoolTime );
		// 080519 LUJ, ��Ÿ�� üũ ���� ���� ��� �����ϰ� ��ȯ�Ѵ�. ���� ȸ�� �̻� ���� �� ��Ģ�� �ο��ϱ� ����
		return FALSE;
	}

	const SkillCoolTimeMap::const_iterator it = mSkillCoolTimeMap.find( skill.Index );

	if(mSkillCoolTimeMap.end() == it)
	{
		return FALSE;
	}

	const DWORD endTime		= it->second;
	const BOOL	isCoolTime	=  endTime > gCurTime;

	if( isCoolTime )
	{
		// 080516 LUJ, ��Ÿ�� üũ�� ���� ���� �̻� �������� �� ��Ģ�� �ο��Ѵ�
		Punish( *this, mCheckCoolTime );
	}
	
	// 080519 LUJ, ��Ÿ�� üũ ���� ���� ��� �����ϰ� ��ȯ�Ѵ�. ���� ȸ�� �̻� ���� �� ��Ģ�� �ο��ϱ� ����
	return FALSE;
}

// 080511 LUJ, ��ų ��Ÿ���� �߰��Ѵ�
// 080514 LUJ, ��ų �ִϸ��̼� ���� �ð� ����
// 080605 LUJ, ��ų ������ ���� �ִϸ��̼� �ð��� �����Ѵ�
void CPlayer::SetCoolTime( const ACTIVE_SKILL_INFO& skill )
{
	// 080605 LUJ, �ִϸ��̼� Ÿ���� ��ų ������ ���� ������Ű�� ���� ���� ����
	float animationTime = float( skill.AnimationTime );

	// 080605 LUJ, ��ų ������ ���� �ִϸ��̼� Ÿ���� ������ �����Ѵ�.
	{
		const Status* ratePassiveStatus = GetRatePassiveStatus();
		const Status* rateBuffStatus	= GetRateBuffStatus();

		if( rateBuffStatus &&
			ratePassiveStatus )
		{
			if( ( skill.Index / 100000 ) % 10 )
			{
				switch( skill.Unit )
				{
				case UNITKIND_MAGIC_ATTCK:
					{
						animationTime = animationTime * ( 1.0f - ( rateBuffStatus->MagicSkillSpeedRate + ratePassiveStatus->MagicSkillSpeedRate ) / 100.0f );
						break;
					}
				case UNITKIND_PHYSIC_ATTCK:
					{
						animationTime = animationTime * ( 1.0f - ( rateBuffStatus->PhysicSkillSpeedRate + ratePassiveStatus->PhysicSkillSpeedRate ) / 100.0f );
						break;
					}
				}
			}
			else
			{
				animationTime = animationTime * ( 1.0f - ( rateBuffStatus->NormalSpeedRate + ratePassiveStatus->NormalSpeedRate ) / 100.0f );
			}
		}
	}
	
	// 080514 LUJ, �ִϸ��̼��� ������ �ð��� �����Ѵ�. ��Ʈ��ũ ������ �����Ͽ� 0.1�� ������ ����Ѵ�
	// 080520 LUJ, �׽�Ʈ ����� 0.1->0.3�ʷ� ��� �ð� ����
	// 080605 LUJ, �ּ� 0�� ���� ����Ѵ�. animationTime�� �Ǽ��� ����Ǿ� �����÷� ������ �ֱ� ����
	mSkillAnimTimeMap[ skill.Index ] = DWORD( max( 0, animationTime ) ) + gCurTime - 300;
	mSkillCoolTimeMap[ skill.Index ] = skill.CoolTime + gCurTime - 300;	
}

void CPlayer::ResetCoolTime( const ACTIVE_SKILL_INFO& skill )
{
	mSkillAnimTimeMap[skill.Index] = 0;
	mSkillCoolTimeMap[skill.Index] = 0;
}

BOOL CPlayer::IsCanCancelSkill()
{
	// 100618 ShinJS �����ð����� Ư��ȸ��(����3ȸ)�̻� ��û�� ��ҿ�û�� ������ �ش�.
	if( m_dwSkillCancelLastTime > gCurTime )
	{
		if( ++m_dwSkillCancelCount >= eSkillCancelLimit_Count )
		{
			m_dwSkillCancelLastTime = gCurTime + eSkillCancelLimit_CheckTime;
			return FALSE;
		}

		return TRUE;
	}
	
	m_dwSkillCancelCount = 0;
	m_dwSkillCancelLastTime = gCurTime + eSkillCancelLimit_CheckTime;

	return TRUE;
}

const DWORD CPlayer::GetSkillCancelDelay() const
{
	return eSkillCancelLimit_CheckTime;
}

// 100621 ShinJS ���� ĳ�������� ��ų ���
void CPlayer::CancelCurrentCastingSkill( BOOL bUseSkillCancelRate )
{
	cActiveSkillObject* const activeSkillObject = ( cActiveSkillObject* )SKILLMGR->GetSkillObject( mCurrentSkillID );

	// 090109 LUJ, ��Ƽ�� ��ų�� ��ҵ� �� �ִ�
	// 090109 LUJ, ĳ���� �߿��� ��ҵ� �� �ֵ��� üũ�Ѵ�
	if( ! activeSkillObject || 
		cSkillObject::TypeActive != activeSkillObject->GetType() ||
		! activeSkillObject->IsCasting() )
	{
		return;
	}

	if( bUseSkillCancelRate && 
		activeSkillObject->GetInfo().Cancel <= (rand() % 100) ) 
	{
		return;
	}

	MSG_DWORD message;
	ZeroMemory( &message, sizeof( message ) );
	message.Category	= MP_SKILL;
	message.Protocol	= MP_SKILL_CANCEL_NOTIFY;
	message.dwObjectID	= GetID();
	message.dwData		= mCurrentSkillID;
	PACKEDDATA_OBJ->QuickSend( this, &message, sizeof( MSG_DWORD ) );

	cSkillObject* const skillObject = SKILLMGR->GetSkillObject( mCurrentSkillID );
	if( skillObject )
	{
		skillObject->SetEndState();
		skillObject->EndState();
	}
}

eArmorType CPlayer::GetArmorType(EWEARED_ITEM wearType) const
{
	const ITEM_INFO* const itemInfo = ITEMMGR->GetItemInfo( GetWearedItemIdx( wearType ) );

	return itemInfo ? eArmorType( itemInfo->ArmorType ) : eArmorType_None;
}

void CPlayer::AddSpecialSkill(const cBuffSkillInfo* buffSkillInfo)
{
	// 090204 LUJ, ó������ �׻� �����Ѵ�. ���μ��� �� �˻��ϸ鼭 ���ǿ� ���� ���� ��� �����Ѵ�
	buffSkillInfo->AddBuffStatus( this );

	SpecialSkillData specialSkillData = { 0 };
	specialSkillData.mBuffSkillInfo	= buffSkillInfo;
	specialSkillData.mIsTurnOn = TRUE;
	mSpecialSkillList.push_back( specialSkillData );
}

void CPlayer::RemoveSpecialSkill(const cBuffSkillInfo* buffSkillInfo)
{
	SpecialSkillList::iterator it = mSpecialSkillList.end();

	for(; mSpecialSkillList.end() != it; ++it )
	{
		const SpecialSkillData& specialSkillData = *it;

		if( specialSkillData.mBuffSkillInfo == buffSkillInfo )
		{
			break;
		}
	}

	if(mSpecialSkillList.end() == it)
	{
		return;
	}

	const SpecialSkillData& specialSkillData = *it;

	// 090204 LUJ, ���ǿ� ���� ������ ������ �� �����Ƿ� �˻� �� ����ؾ� �Ѵ�
	if( specialSkillData.mIsTurnOn )
	{
		buffSkillInfo->RemoveBuffStatus( this );
	}

	mSpecialSkillList.erase(it);
}

// 090204 LUJ, Ư�� ��ų�� ���μ��� Ÿ�ӿ� üũ�Ѵ�
void CPlayer::ProcSpecialSkill()
{
	if(true == mSpecialSkillList.empty())
	{
		return;
	}

	// 090204 LUJ, ȿ������ ���� �����̳� �� ���� ��ų�� �˻��� �� �� �ڷ� ������
	SpecialSkillData specialSkillData = mSpecialSkillList.front();
	mSpecialSkillList.pop_front();

	const cBuffSkillInfo* const buffSkillInfo = specialSkillData.mBuffSkillInfo;
	const BOOL validCondition = IsEnable(buffSkillInfo->GetInfo());
	const BOOL isTurnOn = ( ! specialSkillData.mIsTurnOn && validCondition );
	const BOOL isTurnOff = ( specialSkillData.mIsTurnOn && ! validCondition );

	if( isTurnOn )
	{
		buffSkillInfo->AddBuffStatus( this );
		specialSkillData.mIsTurnOn = TRUE;
	}
	else if( isTurnOff )
	{
		buffSkillInfo->RemoveBuffStatus( this );
		specialSkillData.mIsTurnOn = FALSE;
	}

	// 090204 LUJ, ���� �˻縦 ���� �߰��Ѵ�
	mSpecialSkillList.push_back(
		specialSkillData);
}

void CPlayer::SetMasterRecipe(POSTYPE pos, DWORD dwRecipe, DWORD dwRemainTime)
{
	if(pos<0 || MAX_RECIPE_LV4_LIST<=pos)	return;

	m_MasterRecipe[pos].dwRecipeIdx = dwRecipe;
	m_MasterRecipe[pos].dwRemainTime = dwRemainTime;
}

stRecipeLv4Info* CPlayer::GetMasterRecipe(POSTYPE pos)
{
	if(pos<0 || MAX_RECIPE_LV4_LIST<=pos)	return NULL;

	return &m_MasterRecipe[pos];
}

int CPlayer::CanAddRecipe(DWORD dwRecipe)
{
	int i, nEmptySlot = -1;
	for(i=0; i<MAX_RECIPE_LV4_LIST; i++)
	{
		if(nEmptySlot<0 && 0==m_MasterRecipe[i].dwRecipeIdx)
			nEmptySlot = i;

		if(m_MasterRecipe[i].dwRecipeIdx == dwRecipe)
			return -2;
	}

	return nEmptySlot;
}

void CPlayer::ProcessRecipeTimeCheck(DWORD dwElapsedTime)
{
	DWORD dwElapsedMili = dwElapsedTime;

	for(POSTYPE i =0; i<MAX_RECIPE_LV4_LIST; i++)
	{
		if(0!=m_MasterRecipe[i].dwRecipeIdx && 0!=m_MasterRecipe[i].dwRemainTime)
		{
			if(dwElapsedMili > m_MasterRecipe[i].dwRemainTime)
			{
				// ����
				DWORD dwRecipeIdx = m_MasterRecipe[i].dwRecipeIdx;
				SetMasterRecipe(i, 0, 0);
				Cooking_Recipe_Update(GetID(), eCOOKRECIPE_DEL, dwRecipeIdx, i, 0);
				CookRecipe_Log(GetID(), dwRecipeIdx, i, eCOOKRECIPE_DEL, 0);

				MSG_DWORD4 msg;
				msg.Category	= MP_COOK;
				msg.Protocol	= MP_COOK_UPDATERECIPE;
				msg.dwObjectID	= GetID();
				msg.dwData1		= eCOOKRECIPE_DEL;
				msg.dwData2		= dwRecipeIdx;
				msg.dwData3		= i;
				msg.dwData4		= 0;
				SendMsg(&msg, sizeof(msg));
			}
			else
			{
				// ����
				DWORD dwRemainTime = m_MasterRecipe[i].dwRemainTime - dwElapsedMili;
				SetMasterRecipe(i, m_MasterRecipe[i].dwRecipeIdx, dwRemainTime);
				Cooking_Recipe_Update(GetID(), eCOOKRECIPE_UPDATE, m_MasterRecipe[i].dwRecipeIdx, i, dwRemainTime);

				MSG_DWORD4 msg;
				msg.Category	= MP_COOK;
				msg.Protocol	= MP_COOK_UPDATERECIPE;
				msg.dwObjectID	= GetID();
				msg.dwData1		= eCOOKRECIPE_UPDATE;
				msg.dwData2		= m_MasterRecipe[i].dwRecipeIdx;
				msg.dwData3		= i;
				msg.dwData4		= dwRemainTime;
				SendMsg(&msg, sizeof(msg));
			}
		}
	}
}

void CPlayer::ProceedToTrigger()
{
	if(gCurTime < mNextCheckedTick)
	{
		return;
	}

	// ������������ �޽����� �߻���Ű�� �ʴ´�.
	if(m_bDungeonObserver)
		return;

	// 091116 LUJ, �ֱ������� �߼��ϴ� �޽��� ������ �ø�(0.5 -> 1.0��)
	const DWORD stepTick = 1000;
	mNextCheckedTick = gCurTime + stepTick;
	// 091116 LUJ, ä�ο� �ش��ϴ� �޽����� �Ҵ�޵��� �Ѵ�
	Trigger::CMessage* const message = TRIGGERMGR->AllocateMessage(GetGridID());
	message->AddValue(Trigger::eProperty_ObjectIndex, GetID());
	message->AddValue(Trigger::eProperty_ObjectKind, GetObjectKind());
	message->AddValue(Trigger::eProperty_Event, Trigger::eEvent_CheckSelf);
}

float CPlayer::GetBonusRange() const
{	
	const float value = mPassiveStatus.Range + mBuffStatus.Range;
	const float percent = mRatePassiveStatus.Range + mRateBuffStatus.Range;

	return value * (1.0f + percent);
}

BOOL CPlayer::IsNoEquip(eArmorType armorType, eWeaponType weaponType, eWeaponAnimationType weaponAnimationType)
{
	switch(armorType)
	{
	case eArmorType_Robe:
	case eArmorType_Leather:
	case eArmorType_Metal:
		{
			if(GetArmorType(eWearedItem_Dress) != armorType)
			{
				return TRUE;
			}

			break;
		}
	case eArmorType_Shield:
		{
			if(GetArmorType(eWearedItem_Shield) != armorType)
			{
				return TRUE;
			}

			break;
		}
	}

	if(eWeaponType_None != weaponType)
	{
		if(GetWeaponEquipType() != weaponType)
		{
			return TRUE;
		}
	}

	switch( weaponAnimationType )
	{
	case eWeaponAnimationType_None:
		break;
	case eWeaponAnimationType_OneHand:
		{
			switch( GetWeaponAniType() )
			{
			case eWeaponAnimationType_OneHand:
			case eWeaponAnimationType_Dagger:
			case eWeaponAnimationType_TwoBlade:
				break;
			default:
				return TRUE;
			}
		}
		break;

	case eWeaponAnimationType_TwoHand:
		{
			switch( GetWeaponAniType() )
			{
			case eWeaponAnimationType_TwoHand:
			case eWeaponAnimationType_Staff:
			case eWeaponAnimationType_Bow:
			case eWeaponAnimationType_Gun:
			case eWeaponAnimationType_Spear:
				break;
			default:
				return TRUE;
			}
		}
		break;

	default:
		{
			if( GetWeaponAniType() != weaponAnimationType )
				return TRUE;
		}
		break;
	}

	return FALSE;
}

BOOL CPlayer::IsEnable(const BUFF_SKILL_INFO& info)
{
	if(IsNoEquip(
		info.mArmorType,
		info.mWeaponType,
		info.mWeaponAnimationType))
	{
		return FALSE;
	}

	float checkValue = 0;
	const BUFF_SKILL_INFO::Condition& condition = info.mCondition;

	switch(condition.mType)
	{
	case BUFF_SKILL_INFO::Condition::TypeLifePercent:
		{
			checkValue = float(GetLifePercent());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeLife:
		{
			checkValue = float(GetLife());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeLifeMax:
		{
			checkValue = float(GetMaxLife());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeLifeRecovery:
		{
			checkValue = float(GetLifeRecover());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeManaPercent:
		{
			checkValue = float(GetManaPercent());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeMana:
		{
			checkValue = float(GetMana());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeManaMax:
		{
			checkValue = float(GetMaxMana());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeManaRecovery:
		{
			checkValue = float(GetManaRecover());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeStrength:
		{
			checkValue = float(GetStrength());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeIntelligence:
		{
			checkValue = float(GetIntelligence());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeDexterity:
		{
			checkValue = float(GetDexterity());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeWisdom:
		{
			checkValue = float(GetWisdom());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeVitality:
		{
			checkValue = float(GetVitality());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypePhysicalAttack:
		{
			checkValue = float(GetPhysicAttackMax());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeMagicalAttack:
		{
			checkValue = float(GetMagicAttackMax());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypePhysicalDefense:
		{
			checkValue = float(GetPhysicDefense());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeMagicalDefense:
		{
			checkValue = float(GetMagicDefense());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeCriticalRate:
		{
			checkValue = float(GetCriticalRate());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeCriticalDamage:
		{
			checkValue = float(GetCriticalDamageRate());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeAccuracy:
		{
			checkValue = float(GetAccuracy());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeEvade:
		{
			checkValue = float(GetAvoid());
			break;
		}
	case BUFF_SKILL_INFO::Condition::TypeMoveSpeed:
		{
			checkValue = float(GetMoveSpeed());
			break;
		}
	}

	BOOL isEnable = TRUE;

	switch(condition.mOperation)
	{
	case BUFF_SKILL_INFO::Condition::OperationMore:
		{
			isEnable = (checkValue > condition.mValue);
			break;
		}
	case BUFF_SKILL_INFO::Condition::OperationMoreEqual:
		{
			isEnable = (checkValue >= condition.mValue);
			break;
		}
	case BUFF_SKILL_INFO::Condition::OperationLess:
		{
			isEnable = (checkValue < condition.mValue);
			break;
		}
	case BUFF_SKILL_INFO::Condition::OperationLessEqual:
		{
			isEnable = (checkValue <= condition.mValue);
			break;
		}
	case BUFF_SKILL_INFO::Condition::OperationEqual:
		{
			isEnable = (checkValue == condition.mValue);
			break;
		}
	}

	return isEnable;
}

cSkillTree& CPlayer::GetSkillTree()
{
	return *m_SkillTree;
}

void CPlayer::SetPartyIdx( DWORD PartyIDx )
{
	m_HeroInfo.PartyID = PartyIDx; 

	if(m_HeroInfo.PartyID)
	{
		return;
	}

	// ��Ƽ ������ ��� Ư�� ��ų�� �����Ѵ�
	{
		cPtrList templist;
		m_BuffSkillList.SetPositionHead();

		while(cBuffSkillObject* pSObj = (cBuffSkillObject*)m_BuffSkillList.GetData())
		{
			if(GetPartyIdx() != 0 && pSObj->GetInfo().Party == 1 )
			{
				pSObj->SetEndState();
				pSObj->EndState();
				templist.AddTail( pSObj );
			}
		}

		PTRLISTPOS pos = templist.GetHeadPosition();
		while( pos )
		{
			cSkillObject* const pSObj = ( cSkillObject* )templist.GetNext( pos );
			m_BuffSkillList.Remove( pSObj->GetSkillIdx() );
		}
		templist.RemoveAll();
	}
}

CObject* CPlayer::GetTObject() const
{
	if(mAggroObjectContainer.empty())
	{
		return 0;
	}

	ObjectIndexContainer::const_iterator iterator = mAggroObjectContainer.begin();

	std::advance(
		iterator,
		rand() % mAggroObjectContainer.size());

	return g_pUserTable->FindUser(
		*iterator);
}

void CPlayer::AddToAggroed(DWORD objectIndex)
{
	// ������ �������� ���� ��� �����ڰ� �ڱ� �ڽ��� �ȴ�. �ڽ��� ��׷� �����̳ʿ� ������ �ʿ�� ����
	if(GetID() == objectIndex)
	{
		return;
	}

	CObject* const pObject = g_pUserTable->FindUser( objectIndex );
	if( !pObject )
		return;

	if( pObject->GetObjectKind() & eObjectKind_Monster )
	{
		// 100616 ShinJS --- ��뿡�� �ڽ��� ����ϵ��� �Ͽ� Die/Release�� ��׷θ� �����Ҽ� �ֵ��� �Ѵ�.
		((CMonster*)pObject)->AddToAggroed( GetID() );
	}

	mAggroObjectContainer.insert(objectIndex);
}

void CPlayer::LogOnRelease()
{
	if(DungeonMGR->IsDungeon(g_pServerSystem->GetMapNum()))
	{
		TCHAR text[MAX_PATH] = {0};
		_sntprintf(
			text,
			_countof(text),
			"map:%d",
			g_pServerSystem->GetMapNum());
		LogItemMoney(
			GetID(),
			GetObjectName(),
			0,
			text,
			eLog_DungeonEnd,
			GetMoney(),
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
		return;
	}
	else if(g_csDateManager.IsChallengeZone(g_pServerSystem->GetMapNum()))
	{
		TCHAR text[MAX_PATH] = {0};
		_sntprintf(
			text,
			_countof(text),
			"map:%d",
			g_pServerSystem->GetMapNum());
		LogItemMoney(
			GetID(),
			GetObjectName(),
			0,
			text,
			eLog_DateMatchEnd,
			GetMoney(),
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
		return;
	}

	const LimitDungeonScript& script = g_CGameResourceManager.GetLimitDungeonScript(
		g_pServerSystem->GetMapNum(),
		GetChannelID());

	if(script.mMapType == g_pServerSystem->GetMapNum())
	{
		TCHAR text[MAX_PATH] = {0};
		_sntprintf(
			text,
			_countof(text),
			"%map:%d(%d)",
			g_pServerSystem->GetMapNum(),
			GetChannelID());
		LogItemMoney(
			GetID(),
			GetObjectName(),
			0,
			text,
			eLog_LimitDungeonEnd,
			GetMoney(),
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
	}
}

// 100624 ONS HP������Ʈ���� ó�� �߰�
void CPlayer::AddLifeRecoverTime( const YYRECOVER_TIME& recoverTime )
{
	m_YYLifeRecoverTimeQueue.push( recoverTime );
}

void CPlayer::UpdateLife()
{
	if( 0 != m_LifeRecoverDirectlyAmount )
	{
		AddLife( m_LifeRecoverDirectlyAmount, NULL );
		m_LifeRecoverDirectlyAmount = 0;
		return;
	}

	if( 0 == m_YYLifeRecoverTime.count )
	{
		if( FALSE == m_YYLifeRecoverTimeQueue.empty() )
		{
			// �ϳ��� ť���� ������ ������Ʈ��Ű���� �Ѵ�.
			m_YYLifeRecoverTime = m_YYLifeRecoverTimeQueue.front();
			m_YYLifeRecoverTimeQueue.pop();
		}
	}

	if( m_YYLifeRecoverTime.count > 0 )
	{
		if( m_YYLifeRecoverTime.lastCheckTime < gCurTime )
		{
			// HP�� ������ ť�� ����� �����͸� ��� �����Ѵ�.
			if( GetMaxLife() <= GetLife() )
			{
				while( !m_YYLifeRecoverTimeQueue.empty() )
				{
					m_YYLifeRecoverTimeQueue.pop();
				}
				m_YYLifeRecoverTime.count = 0;
				return;
			}

			// HP������Ʈ�Ѵ�.
			m_YYLifeRecoverTime.lastCheckTime = gCurTime + m_YYLifeRecoverTime.recoverDelayTime;
			AddLife( m_YYLifeRecoverTime.recoverUnitAmout, NULL );
			--m_YYLifeRecoverTime.count;
		}
	}
}

// 100624 ONS MP������Ʈ���� ó�� �߰�
void CPlayer::AddManaRecoverTime( const YYRECOVER_TIME& recoverTime )
{
	m_YYManaRecoverTimeQueue.push( recoverTime );
}

void CPlayer::UpdateMana()
{
	if( 0 != m_ManaRecoverDirectlyAmount )
	{
		AddMana( m_ManaRecoverDirectlyAmount, NULL );
		m_ManaRecoverDirectlyAmount = 0;
		return;
	}

	if( 0 == m_YYManaRecoverTime.count )
	{
		if( FALSE == m_YYManaRecoverTimeQueue.empty() )
		{
			m_YYManaRecoverTime = m_YYManaRecoverTimeQueue.front();
			m_YYManaRecoverTimeQueue.pop();
		}
	}

	if( m_YYManaRecoverTime.count > 0 )
	{
		if( m_YYManaRecoverTime.lastCheckTime < gCurTime )
		{
			if( GetMaxMana() <= GetMana() )
			{
				while( !m_YYManaRecoverTimeQueue.empty() )
				{
					m_YYManaRecoverTimeQueue.pop();
				}
				m_YYManaRecoverTime.count = 0;
				return;
			}

			m_YYManaRecoverTime.lastCheckTime = gCurTime + m_YYManaRecoverTime.recoverDelayTime;
			AddMana( m_YYManaRecoverTime.recoverUnitAmout, NULL );
			--m_YYManaRecoverTime.count;
		}
	}
}

// 100611 ONS ä�ñ������� ���� �Ǵ�.
BOOL CPlayer::IsForbidChat() const
{
	__time64_t time = 0;
	_time64( &time );

	if( time > ForbidChatTime || 0 == ForbidChatTime )
		return FALSE;

	return TRUE;
}
void CPlayer::VerifyCaptcha(DWORD verify, DWORD type)
{
	if(verify != m_VerifyCaptcha)
	{
		if(m_VerifyCount > 2)
		{
			m_VerifyCaptcha = 0;
			m_VerifyCount = 0;
			char buff [MAX_NAME_LENGTH+1];
			sprintf(buff, "%s", GetObjectName());
			MSG_NAME msg;
			ZeroMemory(&msg, sizeof(msg));
			msg.Category	= MP_CHEAT;
			msg.Protocol	= MP_CHEAT_DISCONNECTPLAYER_SYN;
			msg.dwObjectID	= 198775;
			SafeStrCpy( msg.Name, buff, MAX_NAME_LENGTH+1 );
			g_Network.Send2AgentServer(LPTSTR(&msg), sizeof(msg));
		} else {
			m_VerifyCount += 1;
			m_VerifyCaptcha = random(1000, 9999);

			MSG_DWORD2 msg;
			msg.Category = MP_USERCONN;
			msg.Protocol = MP_USERCONN_CAPTCHA_SYN;
			msg.dwObjectID = GetID();
			msg.dwData1 = m_VerifyCaptcha;
			msg.dwData2 = type;
			SendMsg( &msg, sizeof(msg) );
		}

	} else if (m_VerifyCaptcha != 0) {
		
		WORD itemnumx = WORD(random(1, 3));
		ITEMMGR->ObtainGeneralItem(this, 11000001, itemnumx, eLog_ItemObtainMonster, MP_ITEM_MONSTER_OBTAIN_NOTIFY);
		
		MSGBASE msg;
		msg.Category = MP_USERCONN;
		msg.Protocol = MP_USERCONN_CAPTCHA_ACK;
		msg.dwObjectID = GetID();
		SendMsg( &msg, sizeof(msg) );

		m_MaxCountSpin = 0;
		m_VerifyCaptcha = 0;
		m_VerifyCount = 0;
		m_VerifyKillCount = 0;
	}
}

// --- skr 12/01/2020
void CPlayer::UpdateRelife()
{
  if( RelifeStartTime ){
    DWORD spend = gCurTime - RelifeStartTime;
    if( RelifeTimer  <= spend ){
      RelifeStartTime = 0;
      RelifeON = FALSE;
    }
  }
}
void CPlayer::SetRelifeTimer(DWORD anum)
{
  RelifeTimer = anum;
}
void CPlayer::SetRelifeStart()
{
  if( RELIFEEMGR->isRelifeMod() && !RelifeON ){
    RelifeStartTime = gCurTime;
    RelifeON = TRUE;
    if( RELIFEEMGR->getBuffIdx() != 0){
    SKILLMGR->BuffSkillStart(
      GetID(),
      RELIFEEMGR->getBuffIdx(),
      RELIFEEMGR->getBuffRemainTime(),
      RELIFEEMGR->getBuffCount()
      );
    }
  }
}
BOOL CPlayer::CheckReLifeBuff(cBuffSkillInfo* abuff)
{
  BOOL ret = FALSE;
  DWORD biffindex = abuff->GetIndex();
  if( biffindex == 0 ){}
  else{
    ret = RELIFEEMGR->isAllowBuff( biffindex );
  }
  return ret;
}
BOOL CPlayer::CheckReLifeSkill(DWORD abuff)
{
  BOOL ret = FALSE;
  if( abuff == 0 ){}
  else{
    ret = RELIFEEMGR->isAllowSkill( abuff );
  }
  return ret;
}
// --- skr : warehouse 2020agt28
void CPlayer::GetWarehouseStartEnd(DWORD & _start, DWORD & _end)
{
	DWORD startpos = 0, endpos = 0;
	switch( currentwarehouseset ){
		case 0:
		{
			startpos = TP_STORAGE_START;
			endpos = TP_STORAGE_END;
		}
		break;
		case 1:
		{
			startpos = TP_STORAGE_START_SET1;
			endpos = TP_STORAGE_END_SET1;
		}
		break;
		case 2:
		{
			startpos = TP_STORAGE_START_SET2;
			endpos = TP_STORAGE_END_SET2;
		}
		break;
		case 3:
		{
			startpos = TP_STORAGE_START_SET3;
			endpos = TP_STORAGE_END_SET3;
		}
		break;
		case 4:
		{
			startpos = TP_STORAGE_START_SET4;
			endpos = TP_STORAGE_END_SET4;
		}
		break;
		default: break;
	}
	_start = startpos;
	_end = endpos;
	return;
}