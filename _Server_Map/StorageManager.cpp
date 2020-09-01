// StorageManager.cpp: implementation of the CStorageManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StorageManager.h"
#include "MapDBMsgParser.h"
#include "Player.h"
#include "UserTable.h"
#include "Network.h"
#include "MHFile.h"
#include "ItemManager.h"
#include "..\[CC]Header\GameResourceManager.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStorageManager::CStorageManager()
{
}

CStorageManager::~CStorageManager()
{
}

void CStorageManager::NetworkMsgParse( BYTE Protocol, void* pMsg )
{
	switch( Protocol )
	{
	case MP_STORAGE_BUY_SYN:
		{
			MSGBASE* pmsg = (MSGBASE*)pMsg;
			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser(pmsg->dwObjectID);
			if(!pPlayer)	return;
			
			BuyStorageSyn(pPlayer);
		}
		break;
	case MP_STORAGE_PUTIN_MONEY_SYN:
		{
			MSG_DWORDEX* pmsg = (MSG_DWORDEX*)pMsg;
			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser(pmsg->dwObjectID);
			if(!pPlayer)	return;

			PutInMoneyStorage(pPlayer, (DWORDEX)pmsg->dweData);
		}
		break;
	case MP_STORAGE_PUTOUT_MONEY_SYN:
		{
			MSG_DWORDEX* pmsg = (MSG_DWORDEX*)pMsg;
			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser(pmsg->dwObjectID);
			if(!pPlayer)	return;

			PutOutMoneyStorage(pPlayer, (DWORDEX)pmsg->dweData);
		}
		break;

// --- skr : warehouse
	case MP_STORAGE_WAREHOUSE_SYN :
	{
		MSG_DWORD* pmsg = (MSG_DWORD*)pMsg;
		CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser(pmsg->dwObjectID);
		if(!pPlayer)	return;
		ChangeWarehouseSet( pPlayer, (DWORD)pmsg->dwData );
	}
	break;

	}

}

void CStorageManager::LoadStorageList()
{
	CMHFile file;
	if(file.Init("./System/Resource/StorageListInfo.bin","rb") == FALSE)
		return;
	
	int count = 0;
	for(int n=0;n<MAX_STORAGELIST_NUM;++n)
	{
		m_StorageListInfo[count].MaxCellNum = BYTE( 30*(count+1) );
		m_StorageListInfo[count].MaxMoney = file.GetDwordEx();
		m_StorageListInfo[count].BuyPrice = file.GetDwordEx();
		
		++count;
	}
	
	file.Release();

}

void CStorageManager::SendStorageItemInfo(CPlayer* pPlayer)
{
	SEND_STORAGE_ITEM_INFO StorageItem;
	memset(&StorageItem, 0, sizeof(SEND_STORAGE_ITEM_INFO));
	StorageItem.Category = MP_ITEM;
	StorageItem.Protocol = MP_ITEM_STORAGE_ITEM_INFO_ACK;

	// 081010 LUJ, 옵션을 전송할 구조체 선언
	MSG_ITEM_OPTION optionMessage;
	ZeroMemory( &optionMessage, sizeof( optionMessage ) );
	optionMessage.Category	= MP_ITEM;
	optionMessage.Protocol	= MP_ITEM_OPTION_ADD_ACK;

	CItemSlot * pSlot = pPlayer->GetSlot(eItemTable_Storage);
	StorageItem.money = pSlot->GetPurse()->GetPurseCurMoney();
	
	POSTYPE StartPos = TP_STORAGE_START;
	
	for(POSTYPE i=0;i<SLOT_STORAGE_NUM;++i)
	{
		if( pSlot->IsEmpty(StartPos+i) == FALSE )
		{
			 StorageItem.StorageItem[i]= *pSlot->GetItemInfoAbs( POSTYPE( StartPos+i ) );

			ITEMBASE& item = StorageItem.StorageItem[i];

			const ITEM_OPTION& option = ITEMMGR->GetOption( item );

			if( option.mItemDbIndex )
			{
				optionMessage.mOption[ optionMessage.mSize++ ] = option;
			}

			const DWORD maxOptionSize = ( sizeof( optionMessage.mOption ) / sizeof( *( optionMessage.mOption ) ) );

			// 081010 LUJ, 창고 슬롯이 옵션 메시지 수보다 크므로 최대 개수에 도달했을 때 전송시키고 재전송할 준비를 한다
			if( maxOptionSize == optionMessage.mSize )
			{
				pPlayer->SendMsg( &optionMessage, optionMessage.GetSize() );
				ZeroMemory( &optionMessage.mOption, sizeof( optionMessage.mOption ) );

				optionMessage.mSize = 0;
			}
		}
	}
	
	// 081010 LUJ, 남아있는 옵션을 전송한다
	if( optionMessage.mSize )
	{
		pPlayer->SendMsg( &optionMessage, optionMessage.GetSize() );
	}

	pPlayer->SendMsg(&StorageItem, StorageItem.GetSize());
}

void CStorageManager::BuyStorageSyn(CPlayer* pPlayer)
{
	WORD storagenum = pPlayer->GetStorageNum();

	STORAGELISTINFO* pBuyStorageInfo = GetStorageInfo(storagenum+1);

	if( storagenum >= TAB_BASE_STORAGE_NUM || pPlayer->GetMoney() < pBuyStorageInfo->BuyPrice )
	{
		MSGBASE msg;
		msg.Category = MP_STORAGE;
		msg.Protocol = MP_STORAGE_BUY_NACK;
		pPlayer->SendMsg(&msg, sizeof(msg));
	}
	else
	{
		pPlayer->SetMoney(pBuyStorageInfo->BuyPrice, MONEY_SUBTRACTION, 0, eItemTable_Inventory, eMoneyLog_LoseStorageBuy, 0);
		pPlayer->SetStorageNum( BYTE( storagenum+1 ) );
		pPlayer->SetMaxPurseMoney(eItemTable_Storage, GetStorageInfo(pPlayer->GetStorageNum())->MaxMoney);
		StorageBuyStorage(pPlayer->GetID()); //db update		
		
		MSG_BYTE msg;
		msg.Category = MP_STORAGE;
		msg.Protocol = MP_STORAGE_BUY_ACK;
		msg.bData	= BYTE(storagenum+1);
		pPlayer->SendMsg(&msg, sizeof(msg));
	}
}

void CStorageManager::PutInMoneyStorage(CPlayer* pPlayer, MONEYTYPE setMoney)
{
	MONEYTYPE storagemon = pPlayer->GetMoney(eItemTable_Storage);
	MONEYTYPE maxmon = pPlayer->GetMaxPurseMoney(eItemTable_Storage);
	
	if(pPlayer->GetMoney() < setMoney)
	{
		setMoney = pPlayer->GetMoney();
	}
	if( maxmon - storagemon < setMoney )
	{
		setMoney = maxmon - storagemon;
	}	
	
	if(setMoney == 0)
	{
		MSGBASE msg;
		msg.Category = MP_STORAGE;
		msg.Protocol = MP_STORAGE_PUTIN_MONEY_NACK;
		
		pPlayer->SendMsg(&msg, sizeof(msg));
		return;
	}
		
	setMoney = pPlayer->SetMoney(setMoney, MONEY_SUBTRACTION, 0, eItemTable_Inventory, eMoneyLog_LoseStorage, pPlayer->GetUserID());
	if(setMoney == 0)
	{
		ASSERTMSG(0, "Must Check!");
		return;
	}
	pPlayer->SetMoney(setMoney, MONEY_ADDITION, 0, eItemTable_Storage);
	

	StorageMoneyUpdateToDB(pPlayer->GetUserID(), pPlayer->GetMoney(eItemTable_Storage), pPlayer->GetID());	//dbUpdate

	InsertLogMoney( eMoneyLog_LoseStorage, pPlayer->GetID(), pPlayer->GetMoney(), setMoney, pPlayer->GetUserID(), pPlayer->GetMoney(eItemTable_Storage) );
	
	//YH
	LogItemMoney( pPlayer->GetID(), pPlayer->GetObjectName(), pPlayer->GetUserID(), "#Storage",
				eLog_ItemMoveInvenToStorage, pPlayer->GetMoney(), pPlayer->GetMoney(eItemTable_Storage),
				setMoney, 0, 0, 0, 0, 0, pPlayer->GetPlayerExpPoint());

	MSG_DWORDEX msg;
	msg.Category = MP_STORAGE;
	msg.Protocol = MP_STORAGE_PUTIN_MONEY_ACK;
	msg.dweData	= pPlayer->GetMoney(eItemTable_Storage);

	pPlayer->SendMsg(&msg, sizeof(msg));
}

void CStorageManager::PutOutMoneyStorage(CPlayer* pPlayer, MONEYTYPE getMoney)
{
	DWORDEX getmoney = getMoney;
	DWORDEX storagemon = pPlayer->GetMoney(eItemTable_Storage);
	DWORDEX maxmon = pPlayer->GetMaxPurseMoney(eItemTable_Inventory); //인벤토리로 돈 옮기니까.
	
	if( storagemon < getmoney)
	{
		getmoney = storagemon;
	}
	
	if(getmoney + pPlayer->GetMoney() > MAX_INVENTORY_MONEY)
	{
		MSG_DWORDEX msg;
		msg.Category = MP_STORAGE;
		msg.Protocol = MP_STORAGE_PUTOUT_MONEY_NACK;
		msg.dweData	= pPlayer->GetMoney(eItemTable_Storage);

		pPlayer->SendMsg(&msg, sizeof(msg));
		return;
	}
	else if(getmoney + pPlayer->GetMoney() > maxmon)
	{
		getmoney = maxmon - pPlayer->GetMoney();
	}

	if(getmoney == 0)
		return;
	getmoney = pPlayer->SetMoney(MONEYTYPE(getmoney), MONEY_SUBTRACTION, 0, eItemTable_Storage);
	if(getmoney == 0) //purse에서 정말 빠졌는지 
	{
		ASSERTMSG(0, "Must Check!");
		return;
	}
	pPlayer->SetMoney(MONEYTYPE(getmoney), MONEY_ADDITION, 0, eItemTable_Inventory, eMoneyLog_GetStorage, pPlayer->GetUserID());
	

	StorageMoneyUpdateToDB(pPlayer->GetUserID(), pPlayer->GetMoney(eItemTable_Storage), pPlayer->GetID());	//dbUpdate
	
	InsertLogMoney( eMoneyLog_GetStorage, pPlayer->GetID(), pPlayer->GetMoney(), MONEYTYPE(getmoney), pPlayer->GetUserID(), pPlayer->GetMoney(eItemTable_Storage) );

	//YH
	LogItemMoney( pPlayer->GetUserID(), "#Storage", pPlayer->GetID(), pPlayer->GetObjectName(), 
				eLog_ItemMoveStorageToInven, pPlayer->GetMoney(eItemTable_Storage), pPlayer->GetMoney(),
				MONEYTYPE(getmoney), 0, 0, 0, 0, 0, pPlayer->GetPlayerExpPoint());

	MSG_DWORDEX msg;
	msg.Category = MP_STORAGE;
	msg.Protocol = MP_STORAGE_PUTOUT_MONEY_ACK;
	msg.dweData	= pPlayer->GetMoney(eItemTable_Storage);

	pPlayer->SendMsg(&msg, sizeof(msg));
}

STORAGELISTINFO* CStorageManager::GetStorageInfo(DWORDEX i)
{
	ASSERT(i>0);
	//ASSERT(i<=MAX_STORAGELIST_NUM);
	ASSERT(i<=MAX_STORAGELIST_NUMSET4);
	if( i <= MAX_STORAGELIST_NUM ){
		return &m_StorageListInfo[i-1]; 
	}
	else {
		return &m_StorageListInfo[MAX_STORAGELIST_NUM-1]; 
	}
}
// --- skr : warehouse 2020/agt/27
void CStorageManager::ChangeWarehouseSet( CPlayer* pPlayer, DWORD dwdata )
{
	ITEMBASE* aItem = NULL;
	
	POSTYPE startpos = 0, endpos = 0, i = 0;
	DWORD dwstartpos = startpos, dwendpos = endpos, ii = i;
	DWORD dwnum = (DWORD)pPlayer->GetStorageNum();
	DWORD currentset = pPlayer->GetWarehouseSet();
	CItemSlot* pslot = pPlayer->GetSlot( eItemTable_Storage );
	MSG_DWORD msg;
	msg.dwData = pPlayer->GetID();
	msg.Category = MP_STORAGE;
	if( dwdata > dwnum )
	{
		msg.Protocol = MP_STORAGE_WAREHOUSE_NACK;
		pPlayer->SendMsg(&msg, sizeof(msg));
		return;
	}
	if( !pslot )
	{
		msg.Protocol = MP_STORAGE_WAREHOUSE_NACK;
		pPlayer->SendMsg(&msg, sizeof(msg));
		return;
	}
	pPlayer->SetWarehouseSet(dwdata);
	pPlayer->GetWarehouseStartEnd( dwstartpos, dwendpos );
	startpos = dwstartpos;
	endpos = dwendpos;
	if ( startpos == 0 || endpos == 0)
	{
		pPlayer->SetWarehouseSet(currentset);
		msg.Protocol = MP_STORAGE_WAREHOUSE_NACK;
		pPlayer->SendMsg(&msg, sizeof(msg));
		return;
	}
	
	
	
	msg.Protocol = MP_STORAGE_WAREHOUSE_ACK;
	pPlayer->SendMsg(&msg, sizeof(msg));
	
	//pPlayer->SetWarehouseSet(currentset);
	//pPlayer->GetWarehouseStartEnd( dwstartpos, dwendpos );
	for( ii = TP_STORAGE_START; ii < TP_STORAGE_END; ii++)
	{
		if( pslot->IsEmpty( ii )){
			continue;
		}
		aItem = pslot->GetItemInfoAbs( ii );
		if( aItem != NULL && (0 != ITEMMGR->GetOption(*aItem).mItemDbIndex  )){
			ITEMMGR->RemoveOption( aItem->dwDBIdx );
		}
		pslot->ClearItemBaseAndSlotInfo( ii );
	}
	//pPlayer->SetWarehouseSet(dwdata);
	//pPlayer->GetWarehouseStartEnd( dwstartpos, dwendpos );
	
	
	WarehouseItemInfoSet( pPlayer->GetID(), pPlayer->GetUserID(), 0, startpos - 1,endpos, dwdata );
}