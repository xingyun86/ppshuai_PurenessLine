
// PurenessLineDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PurenessLine.h"
#include "PurenessLineDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPurenessLineDlg �Ի���

CPurenessLineDlg::CPurenessLineDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPurenessLineDlg::IDD, pParent),
	m_bUserConnected(FALSE),
	m_bUserRunning(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPurenessLineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CPUVIEW, m_cvCPU);
	DDX_Control(pDX, IDC_MEMORYVIEW, m_cvMemory);
	DDX_Control(pDX, IDC_EDIT1, m_txtServerIP);
	DDX_Control(pDX, IDC_EDIT2, m_txtServerPort);
}

BEGIN_MESSAGE_MAP(CPurenessLineDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CONTROL(CVN_ViewPortChanged, IDC_CPUVIEW, OnViewPortChanged)
	ON_CONTROL(CVN_ViewPortChanged, IDC_MEMORYVIEW, OnViewPortChanged)
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CPurenessLineDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CPurenessLineDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CPurenessLineDlg ��Ϣ�������

BOOL CPurenessLineDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	InitView();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CPurenessLineDlg::InitView()
{
	//��ʼ���б�
	memset(&m_dbCPU, 0, sizeof(m_dbCPU));
	memset(&m_dbMemory, 0, sizeof(m_dbMemory));
	memset(&m_serverRunInfo, 0, sizeof(m_serverRunInfo));

	setUserConnected(FALSE);
	SetDlgItemText(IDC_EDIT1, _T("127.0.0.1"));
	SetDlgItemText(IDC_EDIT2, _T("10010"));

	//�����������
	InitRandom();

	//������ʱ��
	SetTimer(TIMER_ID, TIMER_INTERVAL, 0);

	//������Ӧ������ͼ��
	drawChart();
}

DWORD WINAPI CPurenessLineDlg::GetServerStateInfo(void * p)
{
	DWORD dwResult = 0L;
	CPurenessLineDlg * pThis = (CPurenessLineDlg*)p;
	
	if (pThis->InitConn())
	{
		while (pThis->IsUserRunning())
		{
			if (!pThis->IsUserConnected())
			{
				break;
			}
			int nSend = 0;
			int nRecv = 0;
			//ƴ�ӷ���Э��
			char szSendData[200] = { '\0' };
			char szKey[32] = { '\0' };
			char szCommand[160] = { '\0' };
			size_t stSendEnd = 0;
			sprintf_s(szKey, 32, "%s", "freeeyes");
			sprintf_s(szCommand, 160, "b %s ShowCurrProcessInfo -a&", szKey);
			int nAllLen = (int)strlen(szCommand);
			memcpy(&szSendData, szCommand, nAllLen);

			//��������
			nSend = pThis->getServerSocket()->Send(szSendData, nAllLen);
			if (nSend == SOCKET_ERROR)
			{
				pThis->setUserConnected(FALSE);
				continue;
			}
			if (nSend != nAllLen)
			{
				::Sleep(WAIT_TIMEOUT);
				continue;
			}

			//����ͷ����������
			char czDataLen[6] = { 0 };
			int nDataLen = 0;
			nRecv = pThis->getServerSocket()->Receive(czDataLen, sizeof(czDataLen) / sizeof(*czDataLen));
			if (nRecv == SOCKET_ERROR)
			{
				pThis->setUserConnected(FALSE);
				continue;
			}
			memcpy(&nDataLen, czDataLen, sizeof(nDataLen));
			nDataLen = nDataLen - 2;
			//��������
			char szBuffRecv[500] = { '\0' };
			nRecv = pThis->getServerSocket()->Receive(szBuffRecv, nDataLen);
			if (nRecv == SOCKET_ERROR)
			{
				pThis->setUserConnected(FALSE);
				continue;
			}

			int nStrLen = 0;
			int nPos = 0;
			int nClientCount = 0;

			//�������߳�
			memcpy(&pThis->getServerRunInfo()->m_dCPU, &szBuffRecv[nPos], sizeof(pThis->getServerRunInfo()->m_dCPU));
			nPos += sizeof(pThis->getServerRunInfo()->m_dCPU);
			memcpy(&pThis->getServerRunInfo()->m_nMemorySize, &szBuffRecv[nPos], sizeof(pThis->getServerRunInfo()->m_nMemorySize));
			nPos += sizeof(pThis->getServerRunInfo()->m_nMemorySize);

			::Sleep(WAIT_TIMEOUT);
		}
		pThis->TermConn();
	}
	
	return dwResult;
}
// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CPurenessLineDlg::OnViewPortChanged()
{
	if (IsUserConnected())
	{
		//��������
		shiftData(m_dbCPU, MAX_DATA_COUNT, (double)m_serverRunInfo.m_dCPU);
		shiftData(m_dbMemory, MAX_DATA_COUNT, (double)m_serverRunInfo.m_nMemorySize / (1024 * 1024));
	}
	else
	{
		//��������
		shiftData(m_dbCPU, MAX_DATA_COUNT, (double)0.0f);
		shiftData(m_dbMemory, MAX_DATA_COUNT, (double)0.0f);
	}

	//������Ӧ������ͼ��
	drawChart();
}

void CPurenessLineDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CPurenessLineDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPurenessLineDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	
	OnBnClickedButton2();

	CDialog::OnClose();
}

void CPurenessLineDlg::drawChart()
{
	RECT rcCPU = { 0 };
	RECT rcMEM = { 0 };
	RECT rcSpace = { 0 };
	RECT rcInnerSpace = { 0 };
	const char* szLabels[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
		"11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23",
		"24"};

	XYChart* pXYCPU    = NULL;
	XYChart* pXYMemory = NULL;

	SetRect(&rcSpace, 10, 10, 20, 20);
	SetRect(&rcInnerSpace, rcSpace.left + 60, rcSpace.top + 6, rcSpace.right + 60, rcSpace.bottom + 36);

	//����Ĭ�ϴ����С(CPU)
	this->GetDlgItem(IDC_STATIC_CPU)->GetClientRect(&rcCPU);
	pXYCPU = new XYChart(rcCPU.right - rcCPU.left - rcSpace.left - rcSpace.right, rcCPU.bottom - rcCPU.top - rcSpace.bottom - rcSpace.top, 0xeeeeff, 0x000000, 1);
	pXYCPU->setRoundedFrame();

	//����X���±�
	pXYCPU->xAxis()->setLabels(StringArray(szLabels, (int)(sizeof(szLabels) / sizeof(szLabels[0]))));

	//����λ��
	pXYCPU->setPlotArea(rcInnerSpace.left, rcInnerSpace.top,
		rcCPU.right - rcCPU.left - rcInnerSpace.left - rcInnerSpace.right, 
		rcCPU.bottom - rcCPU.top - rcInnerSpace.bottom - rcInnerSpace.top,
		0x000200, -1, -1, 0x337f59);

	//���һ���µ��߲�
	LineLayer* pLayer = pXYCPU->addLineLayer();

	if(NULL != pLayer)
	{
		pLayer->setLineWidth(2);
		pLayer->addDataSet(DoubleArray(m_dbCPU, (int)(sizeof(m_dbCPU) / sizeof(m_dbCPU[0]))), 0xff0000, "Server #1");
	}

	//���ƶ���Chart����󶨸�ָ���ؼ�
	m_cvCPU.setChart(pXYCPU);
	delete pXYCPU;
	pXYCPU = NULL;

	//����Ĭ�ϴ����С(Memory)
	this->GetDlgItem(IDC_STATIC_MEM)->GetClientRect(&rcMEM);
	pXYMemory = new XYChart(rcMEM.right - rcMEM.left - rcSpace.left - rcSpace.right, rcMEM.bottom - rcMEM.top - rcSpace.bottom - rcSpace.top, 0xeeeeff, 0x000000, 1);
	pXYMemory->setRoundedFrame();

	//����X���±�
	pXYMemory->xAxis()->setLabels(StringArray(szLabels, (int)(sizeof(szLabels) / sizeof(szLabels[0]))));

	//����λ��
	pXYMemory->setPlotArea(rcInnerSpace.left, rcInnerSpace.top,
		rcMEM.right - rcMEM.left - rcInnerSpace.left - rcInnerSpace.right,
		rcMEM.bottom - rcMEM.top - rcInnerSpace.bottom - rcInnerSpace.top, 
		0x000200, -1, -1, 0x337f59);

	//���һ���µ��߲�
	pLayer = pXYMemory->addLineLayer();

	if(NULL != pLayer)
	{
		pLayer->setLineWidth(2);
		pLayer->addDataSet(DoubleArray(m_dbMemory, (int)(sizeof(m_dbMemory) / sizeof(m_dbMemory[0]))), 0xff0000, "Server #1");
	}

	//���ƶ���Chart����󶨸�ָ���ؼ�
	m_cvMemory.setChart(pXYMemory);
	delete pXYMemory;
	pXYMemory = NULL;
}

void CPurenessLineDlg::shiftData(double *data, int len, double newValue)
{
	memmove(data, data + 1, sizeof(*data) * (len - 1));
	data[len - 1] = newValue;
}

void CPurenessLineDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == TIMER_ID)
	{
		m_cvCPU.updateViewPort(true, true);
	}

	CDialog::OnTimer(nIDEvent);
}

void CPurenessLineDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	m_strServerIP = m_strServerPort = _T("");
	m_txtServerIP.GetWindowText(m_strServerIP);
	m_txtServerPort.GetWindowText(m_strServerPort);

	if(m_strServerIP.GetLength() <= 0 || m_strServerPort.GetLength() <= 0)
	{
		MessageBox(_T("�����������ķ�����IP��Port��"), _T("������Ϣ"), MB_OK);
		return;
	}
	//����������״̬�û��߳�	
	StartUserThread();
}
void CPurenessLineDlg::OnBnClickedButton2()
{
	StopUserThread();
}