/*================================================================

 ================================================================*/
#include "FileCliConn.h"
#include "playsound.h"
#include "Common.h"


static ConnMap_t g_file_client_conn_map;

FileCliConn::FileCliConn():
m_bOpen(false)
{
    m_pSeqAlloctor = CSeqAlloctor::getInstance();
	m_transfer_size = 0;
	m_bLast = false;
	m_fp = NULL;
}

FileCliConn::~FileCliConn()
{

}

net_handle_t FileCliConn::connect(const string& strIp, uint16_t nPort)
{
	m_handle = netlib_connect(strIp.c_str(), nPort, imconn_callback, &g_file_client_conn_map);
	if(m_handle != NETLIB_INVALID_HANDLE)
		g_file_client_conn_map.insert(make_pair(m_handle, this));
    return  m_handle;
}



void FileCliConn::OnConfirm()
{
    login();
}

void FileCliConn::OnClose()
{
    log("onclose from handle=%d\n", m_handle);
    Close();
}

void FileCliConn::OnTimer(uint64_t curr_tick)
{
    if (curr_tick > m_last_send_tick + CLIENT_HEARTBEAT_INTERVAL) {
        CImPdu cPdu;
        IM::Other::IMHeartBeat msg;
        cPdu.SetPBMsg(&msg);
        cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_OTHER);
        cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_OTHER_HEARTBEAT);
        uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
        cPdu.SetSeqNum(nSeqNo);
        SendPdu(&cPdu);
    }
    
    if (curr_tick > m_last_recv_tick + CLIENT_TIMEOUT) {
        log("conn to msg_server timeout\n");
        Close();
    }
}


uint32_t FileCliConn::login()
{
	IM::File::IMFileLoginReq msgReq;
	msgReq.set_user_id(m_file_role==IM::BaseDefine::CLIENT_REALTIME_SENDER||m_file_role==IM::BaseDefine::CLIENT_OFFLINE_UPLOAD?m_from_user_id:m_to_user_id);
	msgReq.set_task_id(m_task_id);
	msgReq.set_file_role(m_file_role);
	
	CImPdu cPdu;
	cPdu.SetPBMsg(&msgReq);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_FILE);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_FILE_LOGIN_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
		
    return nSeqNo;
}


void FileCliConn::Close()
{
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_file_client_conn_map.erase(m_handle);
	}
	ReleaseRef();
}

void FileCliConn::HandlePdu(CImPdu* pPdu)
{
    //printf("pdu type = %u\n", pPdu->GetPduType());
	//printf("msg_type=%d\n", pPdu->GetCommandId());
	switch (pPdu->GetCommandId()) {
        case IM::BaseDefine::DFFX_CID_OTHER_HEARTBEAT:
//		printf("Heartbeat\n");
		break;
       
		case IM::BaseDefine::DFFX_CID_FILE_LOGIN_RES:
			printf("DFFX_CID_FILE_LOGIN_RES msg_type=%d\n", pPdu->GetCommandId());
			log("DFFX_CID_FILE_LOGIN_RES msg_type=%d\n", pPdu->GetCommandId());
			_HandleIMFileLoginRsp(pPdu);
			break;

		case IM::BaseDefine::DFFX_CID_FILE_STATE:
			printf("DFFX_CID_FILE_STATE msg_type=%d\n", pPdu->GetCommandId());
			log("DFFX_CID_FILE_STATE msg_type=%d\n", pPdu->GetCommandId());
			_HandleIMFileState(pPdu);
			break;

		case IM::BaseDefine::DFFX_CID_FILE_PULL_DATA_REQ:
			printf("DFFX_CID_FILE_PULL_DATA_REQ msg_type=%d\n", pPdu->GetCommandId());
			log("DFFX_CID_FILE_PULL_DATA_REQ msg_type=%d\n", pPdu->GetCommandId());
			_HandleIMFilePullDataReq(pPdu);
			break;
			
		case IM::BaseDefine::DFFX_CID_FILE_PULL_DATA_RSP:
			printf("DFFX_CID_FILE_PULL_DATA_RSP msg_type=%d\n", pPdu->GetCommandId());
			log("DFFX_CID_FILE_PULL_DATA_RSP msg_type=%d\n", pPdu->GetCommandId());
			_HandleIMFilePullDataRsp(pPdu);
			break;
			
		default:
			printf("wrong msg_type=%d\n", pPdu->GetCommandId());
			log("wrong msg_type=%d\n", pPdu->GetCommandId());
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

void FileCliConn::_HandleIMFileLoginRsp(CImPdu* pPdu)
{
	IM::File::IMFileLoginRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
	
	
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
       
        uint32_t nResult = msgResp.result_code();
       
       	printf("_HandleIMFileLoginRsp:  nResult = %d taskid=%s\n", nResult, msgResp.task_id().c_str());
		
		if(m_file_role==IM::BaseDefine::CLIENT_OFFLINE_DOWNLOAD && !m_fp)
		{
			m_fp = fopen(m_file_name.c_str(), "wb+"); 
			if (!m_fp)
			{
				log("can not open file");

				return;
			}
			
			IM::File::IMFilePullDataReq msgReq;
			msgReq.set_task_id(msgResp.task_id());
			msgReq.set_user_id(m_to_user_id);
			msgReq.set_trans_mode(m_trans_mode);
			msgReq.set_offset(0);
			msgReq.set_data_size(65535);

			CImPdu pduReq;
			pduReq.SetPBMsg( &msgReq);
			pduReq.SetServiceId(IM::BaseDefine::DFFX_SID_FILE);
			pduReq.SetCommandId(IM::BaseDefine::DFFX_CID_FILE_PULL_DATA_REQ);
			uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
			pduReq.SetSeqNum(nSeqNo);
			SendPdu(&pduReq);
		}
    }
	else
    {
        printf("get error:%d, msg:%s\n", pPdu->GetCommandId(), "parse pb error");
    }
}

void FileCliConn::_HandleIMFileState(CImPdu* pPdu)
{
	IM::File::IMFileState msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
	
	
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
       
       	printf("_HandleIMFileState: user_id=%d state = %d taskid=%s\n", msgResp.user_id(), (int)(msgResp.state()), msgResp.task_id().c_str());

		if(msgResp.state()==IM::BaseDefine::CLIENT_FILE_PEER_READY && !m_fp)
		{
			m_fp = fopen(m_file_name.c_str(), "wb+"); 
			if (!m_fp)
			{
				log("can not open file");

				return;
			}
			
			IM::File::IMFilePullDataReq msgReq;
			msgReq.set_task_id(msgResp.task_id());
			msgReq.set_user_id(m_to_user_id);
			msgReq.set_trans_mode(m_trans_mode);
			msgReq.set_offset(0);
			msgReq.set_data_size(65535);

			CImPdu pduReq;
			pduReq.SetPBMsg( &msgReq);
			pduReq.SetServiceId(IM::BaseDefine::DFFX_SID_FILE);
			pduReq.SetCommandId(IM::BaseDefine::DFFX_CID_FILE_PULL_DATA_REQ);
			uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
			pduReq.SetSeqNum(nSeqNo);
			SendPdu(&pduReq);
		}
    }
	else
    {
        printf("get error:%d, msg:%s\n", pPdu->GetCommandId(), "parse pb error");
    }
}

void FileCliConn::_HandleIMFilePullDataReq(CImPdu* pPdu)
{
	IM::File::IMFilePullDataReq msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
	
	
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
       
       	printf("_HandleIMFilePullDataReq: user_id=%d offset = %d taskid=%s data_size=%d\n",
			msgResp.user_id(), msgResp.offset(), msgResp.task_id().c_str(), msgResp.data_size());

		IM::File::IMFilePullDataRsp msgReq;
		msgReq.set_result_code(0);
		msgReq.set_task_id(msgResp.task_id());
		msgReq.set_user_id(m_from_user_id);		
		msgReq.set_offset(msgResp.offset());

		FILE* fp = fopen(m_file_name.c_str(), "rb"); 
		if (!fp)
		{
			log("can not open file");

			return;
		}
		int iret = fseek(fp, msgResp.offset(), SEEK_SET);
		if (0 != iret)
		{
			log("seek offset failed.");
			return;
		}
		char* tmpbuf = new char[msgResp.data_size()];
		int size = fread(tmpbuf, 1, msgResp.data_size(), fp);
		msgReq.set_file_data(tmpbuf, size);
		delete [] tmpbuf;
		tmpbuf = NULL;
		
		//string strFileData = "data file send haha";
		//msgReq.set_data( strFileData.c_str(), strFileData.length() + 1);

		//sleep(1);

		CImPdu pduReq;
		pduReq.SetPBMsg( &msgReq);
		pduReq.SetServiceId(IM::BaseDefine::DFFX_SID_FILE);
		pduReq.SetCommandId(IM::BaseDefine::DFFX_CID_FILE_PULL_DATA_RSP);
		uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
		pduReq.SetSeqNum(nSeqNo);
		SendPdu(&pduReq);
    }
	else
    {
        printf("get error:%d, msg:%s\n", pPdu->GetCommandId(), "parse pb error");
    }
}

void FileCliConn::_HandleIMFilePullDataRsp(CImPdu* pPdu)
{
	IM::File::IMFilePullDataRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
	
	
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {       
       	printf("_HandleIMFilePullDataRsp: user_id=%d result_code = %d taskid=%s offset=%d \n", 
			msgResp.user_id(), msgResp.result_code(), msgResp.task_id().c_str(), msgResp.offset());
		
		m_transfer_size += msgResp.file_data().length();
		if(m_fp)
		{
			fwrite(msgResp.file_data().c_str(),1,msgResp.file_data().length(),m_fp);
		}
		if(m_transfer_size<m_file_size || !m_bLast)
		{
			if(m_transfer_size>=m_file_size) 
				m_bLast = true;

			//sleep(1);
			
			IM::File::IMFilePullDataReq msgReq;
			msgReq.set_task_id(m_task_id);
			msgReq.set_user_id(m_to_user_id);
			msgReq.set_trans_mode(m_trans_mode);
			msgReq.set_offset(m_transfer_size);
			msgReq.set_data_size(65535);

			CImPdu pduReq;
			pduReq.SetPBMsg( &msgReq);
			pduReq.SetServiceId(IM::BaseDefine::DFFX_SID_FILE);
			pduReq.SetCommandId(IM::BaseDefine::DFFX_CID_FILE_PULL_DATA_REQ);
			uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
			pduReq.SetSeqNum(nSeqNo);
			SendPdu(&pduReq);
		}
		else if(m_fp)
		{
			fclose(m_fp);
			m_fp = NULL;
		}
    }
	else
    {
        printf("get error:%d, msg:%s\n", pPdu->GetCommandId(), "parse pb error");
    }
}


