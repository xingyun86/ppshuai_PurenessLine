
// PurenessLineDlg.h : ͷ�ļ�
//

#pragma once

#include "ChartViewer.h"
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
	DECLARE_MESSAGE_MAP()

	CEdit m_txtServerIP;
	CEdit m_txtServerPort;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();

private:
	void shiftData(double *data, int len, double newValue);
	void drawChart();
	void InitView();
	static DWORD WINAPI GetServerStateInfo(void * p);
	BOOL StartUserThread() {
		m_bUserRunning = TRUE;
		m_hUserThread = CreateThread(NULL, 0, &CPurenessLineDlg::GetServerStateInfo, this, 0, &m_dwUserThreadId);
		return (m_hUserThread && m_hUserThread != INVALID_HANDLE_VALUE);
	}
	void StopUserThread() {
		if (m_bUserConnected)
		{
			this->setUserConnected(FALSE);
		}
		if (m_hUserThread && m_hUserThread != INVALID_HANDLE_VALUE)
		{
			m_bUserRunning = FALSE;
			CloseHandle(m_hUserThread);
			m_hUserThread = 0;
			m_dwUserThreadId = 0;
		}
	}
public:
	BOOL InitConn() {
		//�����˿ڵ�ַ
		int nPort = 0;
		BOOL blFlag = FALSE;
		nPort = _ttoi(this->getServerPort());

		this->setUserConnected(FALSE);

		AfxSocketInit();

		blFlag = this->getServerSocket()->Create();
		if (FALSE == blFlag)
		{
			this->MessageBox(_T("��ʼ��Socketʧ�ܡ�"), _T("������Ϣ"), MB_OK);
			return (FALSE);
		}

		blFlag = this->getServerSocket()->Connect(this->getServerIP(), nPort);
		if (FALSE == blFlag)
		{
			_TCHAR szError[MAXCHAR] = { _T('\0') };
			wsprintf(szError, _T("����[%s]���˿�[%d]ʧ�ܣ�error[%d]��"), this->getServerIP(), nPort, GetLastError());
			this->MessageBox((LPCTSTR)szError, _T("������Ϣ"), MB_OK);
			this->getServerSocket()->ShutDown();
			this->getServerSocket()->Close();
			AfxSocketTerm();
			return (FALSE);
		}

		this->setUserConnected(TRUE);
		
		return TRUE;
	}
	VOID TermConn()
	{
		this->getServerSocket()->ShutDown();
		this->getServerSocket()->Close();
		this->setUserConnected(FALSE);
		AfxSocketTerm();
	}
public:
	BOOL IsUserRunning() { return m_bUserRunning; }
	BOOL IsUserConnected() { return m_bUserConnected; }
	VOID setUserConnected(BOOL bUserConnected) { 
		m_bUserConnected = bUserConnected;
		this->GetDlgItem(IDC_EDIT1)->EnableWindow(!m_bUserConnected);
		this->GetDlgItem(IDC_EDIT2)->EnableWindow(!m_bUserConnected);
		this->GetDlgItem(IDC_BUTTON1)->EnableWindow(!m_bUserConnected);
		this->GetDlgItem(IDC_BUTTON2)->EnableWindow(m_bUserConnected);
	}
	CSocket * getServerSocket() { return &m_socketServer; }
	ServerRunInfo * getServerRunInfo() { return &m_serverRunInfo; }
	LPCTSTR getServerIP() { return m_strServerIP; };
	LPCTSTR getServerPort() { return m_strServerPort; };
private:
	CChartViewer	m_cvCPU;                     //CPUʹ����
	CChartViewer	m_cvMemory;                  //�ڴ�ʹ����
	double			m_dbCPU[MAX_DATA_COUNT];
	double			m_dbMemory[MAX_DATA_COUNT];
	CString			m_strServerIP;
	CString			m_strServerPort;
	CSocket			m_socketServer;                 //����Զ��Server������
	BOOL			m_bUserConnected;                   //����״̬
	ServerRunInfo	m_serverRunInfo;			  //���·���������״̬
	DWORD			m_dwUserThreadId;//�û��̱߳�ʶ
	HANDLE			m_hUserThread;//�û��߳̾��
	BOOL			m_bUserRunning;//�û��߳�״̬
};
