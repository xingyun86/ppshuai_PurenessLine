
// PurenessLineDlg.h : ͷ�ļ�
//

#pragma once

#include "ChartViewer.h"

#define WM_USER_NOTIFY (WM_USER + 1)

#include <stdlib.h>
#include <time.h>
#include "afxwin.h"
#include <afxsock.h>

//����������ʱ��Ϣ
typedef struct _ServerRunInfo
{
	double m_dCPU;        //��ǰCPUռ����
	UINT64 m_nMemorySize; //��ǰ�ڴ�ռ����

	_ServerRunInfo()
	{
		m_dCPU        = 0.0f;
		m_nMemorySize = 0;
	}

	_ServerRunInfo& operator = (const _ServerRunInfo& ar)
	{
		this->m_dCPU        = ar.m_dCPU;
		this->m_nMemorySize = ar.m_nMemorySize;
		return *this;
	}
}ServerRunInfo, *PServerRunInfo;

//����һ���������
inline void InitRandom()
{
	srand((int)time(NULL));
};

//��һ��ֵ���л�ȡһ�����ֵ
inline int RandomValue(int nMin, int nMax)
{
	return  nMin + (int) (nMax * (rand() / (RAND_MAX + 1.0)));
};

#define TIMER_ID       1        //��ʱ��ID
#define TIMER_INTERVAL 1000     //��ʱ�����
#define MAX_DATA_COUNT 25       //�����������

// CPurenessLineDlg �Ի���
class CPurenessLineDlg : public CDialog
{
// ����
public:
	CPurenessLineDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_PURENESSLINE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnViewPortChanged();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg LRESULT OnUserNotify(WPARAM /*wParam*/, LPARAM /*lParam*/);
	DECLARE_MESSAGE_MAP()

	CEdit m_txtServerIP;
	CEdit m_txtServerPort;

private:
	void shiftData(double *data, int len, double newValue);
	void drawChart();
	void InitView();
	static DWORD WINAPI GetServerStateInfo(void * p);
	BOOL StartUserThread() {
		m_bUserRunning = TRUE;
		// Create a manual-reset event object. The write thread sets this
		// object to the signaled state when it finishes writing to a 
		// shared buffer. 
		m_hQuitEvent = CreateEvent(
			NULL,               // default security attributes
			TRUE,               // manual-reset event
			FALSE,              // initial state is nonsignaled
			TEXT("__EVENT")  // object name
		);
		m_hUserThread = CreateThread(NULL, 0, &CPurenessLineDlg::GetServerStateInfo, this, 0, &m_dwUserThreadId);
		return (m_hUserThread && (m_hUserThread != INVALID_HANDLE_VALUE));
	}
	void StopUserThread() {
		if (m_bUserRunning)
		{
			m_bUserRunning = FALSE;
		}
		if (m_bUserConnected != NetworkConnectType::NCTYPE_DISCONNECT)
		{
			this->setUserConnected(NetworkConnectType::NCTYPE_DISCONNECT);
		}
		if (m_hUserThread && (m_hUserThread != INVALID_HANDLE_VALUE))
		{
			if (WaitForSingleObject(m_hUserThread, INFINITE) == WAIT_OBJECT_0)
			{
				;//
			}
			CloseHandle(m_hUserThread);
			m_hUserThread = 0;
		}
		m_dwUserThreadId = 0;
		if (m_hQuitEvent && (m_hQuitEvent != INVALID_HANDLE_VALUE))
		{
			CloseHandle(m_hQuitEvent);
			m_hQuitEvent = NULL;
		}
	}
public:
	VOID sendUserNotify() {
		PostMessage(WM_USER_NOTIFY);
	}
	BOOL IsAutoConnect() { 
		return (((CButton*)this->GetDlgItem(IDC_CHECK1))->GetCheck() == BST_CHECKED); 
	}
	BOOL InitConn() {
		//�����˿ڵ�ַ
		int nPort = 0;
		BOOL blFlag = FALSE;
		_TCHAR szErrMsg[MAXCHAR] = { _T('\0') };

		nPort = _ttoi(this->getServerPort());
		
		AfxSocketInit();

		blFlag = this->getServerSocket()->Create();
		if (!blFlag)
		{
			_stprintf(szErrMsg, _T("��ʼ��Socketʧ�ܡ�\n"));
			//this->MessageBox(szErrMsg, _T("������Ϣ"), MB_OK);
			OutputDebugString(szErrMsg);
			return (FALSE);
		}

		blFlag = this->getServerSocket()->Connect(this->getServerIP(), nPort);
		if (!blFlag)
		{
			_stprintf(szErrMsg, _T("����[%s]���˿�[%d]ʧ�ܣ�error[%d]��\n"), this->getServerIP(), nPort, GetLastError());
			//this->MessageBox(szErrMsg, _T("������Ϣ"), MB_OK);
			OutputDebugString(szErrMsg);
			this->TermConn();
			return (FALSE);
		}
		
		return TRUE;
	}
	VOID TermConn()
	{
		this->getServerSocket()->ShutDown();
		this->getServerSocket()->Close();
		AfxSocketTerm();
	}
public:
	HANDLE GetQuitEvent() { return m_hQuitEvent; }
	BOOL IsUserRunning() { return m_bUserRunning; }
	enum NetworkConnectType {
		NCTYPE_RECONNECTING = 0,
		NCTYPE_DISCONNECT = 1,
		NCTYPE_CONNECTED = 2,
	};
	BOOL getUserConnected() { return m_bUserConnected; }
	VOID setUserConnected(NetworkConnectType bUserConnected) {
		this->m_bUserConnected = bUserConnected;
		this->sendUserNotify();
	}
	VOID setUserControlText()
	{
		this->GetDlgItem(IDC_BUTTON1)->SetWindowText((m_bUserConnected != NetworkConnectType::NCTYPE_CONNECTED) ? ((m_bUserConnected != NetworkConnectType::NCTYPE_DISCONNECT) ? _T("��������...") : _T("��ʼ����")) : _T("���ڼ���..."));
		this->GetDlgItem(IDC_EDIT1)->EnableWindow((m_bUserConnected != NetworkConnectType::NCTYPE_CONNECTED));
		this->GetDlgItem(IDC_EDIT2)->EnableWindow((m_bUserConnected != NetworkConnectType::NCTYPE_CONNECTED));
		this->GetDlgItem(IDC_BUTTON1)->EnableWindow((m_bUserConnected != NetworkConnectType::NCTYPE_CONNECTED) & (m_bUserConnected != NetworkConnectType::NCTYPE_RECONNECTING));
		this->GetDlgItem(IDC_BUTTON2)->EnableWindow((m_bUserConnected == NetworkConnectType::NCTYPE_CONNECTED) | (m_bUserConnected == NetworkConnectType::NCTYPE_RECONNECTING));
	}
	CSocket * getServerSocket() { return &m_socketServer; }
	ServerRunInfo * getServerRunInfo() { return &m_serverRunInfo; }
	LPCTSTR getServerIP() { return m_strServerIP; };
	LPCTSTR getServerPort() { return m_strServerPort; };
private:
	CChartViewer		m_cvCPU;                     //CPUʹ����
	CChartViewer		m_cvMemory;                  //�ڴ�ʹ����
	double				m_dbCPU[MAX_DATA_COUNT];
	double				m_dbMemory[MAX_DATA_COUNT];
	CString				m_strServerIP;
	CString				m_strServerPort;
	CSocket				m_socketServer;   //����Զ��Server������
	NetworkConnectType	m_bUserConnected; //����״̬
	ServerRunInfo		m_serverRunInfo;//���·���������״̬
	DWORD				m_dwUserThreadId;//�û��̱߳�ʶ
	HANDLE				m_hUserThread;//�û��߳̾��
	BOOL				m_bUserRunning;//�û��߳�״̬
	HANDLE				m_hQuitEvent;//�˳��¼����
};
