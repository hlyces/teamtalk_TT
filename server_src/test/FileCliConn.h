/*================================================================

 ================================================================*/

#ifndef FILECLICONN_H_
#define FILECLICONN_H_

#include "imconn.h"
#include "IM.BaseDefine.pb.h"
#include "IM.Login.pb.h"
#include "IM.Other.pb.h"
#include "IM.Server.pb.h"
#include "IM.Buddy.pb.h"
#include "IM.File.pb.h"
#include "IM.Message.pb.h"
#include "IM.Group.pb.h"
#include "IPacketCallback.h"
#include "SeqAlloctor.h"
#include <sys/stat.h>

class FileCliConn : public CImConn
{
public:
	FileCliConn();
	virtual ~FileCliConn();

	bool IsOpen() { return m_bOpen; }

    net_handle_t connect(const string& strIp, uint16_t nPort);
    
    virtual void Close();
public:
  	uint32_t login();
public:
	virtual void OnConfirm();
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);

	virtual void HandlePdu(CImPdu* pPdu);
private:
	
	void _HandleIMFileLoginRsp(CImPdu* pPdu);
	void _HandleIMFileState(CImPdu* pPdu);
	void _HandleIMFilePullDataReq(CImPdu* pPdu);
	void _HandleIMFilePullDataRsp(CImPdu* pPdu);
	
private:
	bool 		m_bOpen;
    CSeqAlloctor*   m_pSeqAlloctor;

public:
	uint32_t m_from_user_id;
	uint32_t m_to_user_id;
	uint32_t m_file_size;
	string m_file_name;
	string m_task_id;
	IM::BaseDefine::IpAddr m_ip_addr_list;
	IM::BaseDefine::FileType m_trans_mode;
	uint32_t m_offline_ready;
	IM::BaseDefine::ClientFileRole m_file_role;

	uint32_t m_transfer_size;
	bool m_bLast;

	FILE* m_fp;
};


#endif /* FILECLICONN_H_ */
