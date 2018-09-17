// DDCBlockAccessDlg.cpp : implementation file
//

#include "stdafx.h"
#include <codecvt>
#include "..\..\include\adl_sdk.h"
#include "DDCBlockAccess.h"
#include "DDCBlockAccessDlg.h"
#include "mccs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//********************************
// Prototypes
//********************************
void	vGetVcpCommand(UCHAR ucVcp, UINT * ulMaxVal, UINT * ulCurVal, int iAdapterIndex, int iDisplayIndex);
void	vSetVcpCommand(UCHAR ucVcp, UINT ulVal, int iAdapterIndex, int iDisplayIndex);
bool	vGetCapabilitiesCommand(UCHAR ucVcp, int iAdapterIndex, int iDisplayIndex);
int		vWriteI2c(char * lpucSendMsgBuf, int iSendMsgLen, int iAdapterIndex, int iDisplayIndex);
int		vWriteAndReadI2c(char *lpucSendMsgBuf, int iSendMsgLen, char * lpucRecvMsgBuf, int iRecvMsgLen,  int iAdapterIndex, int iDisplayIndex);
bool	InitADL();
void	FreeADL();


void* __stdcall ADL_Main_Memory_Alloc ( int iSize )
{
    void* lpBuffer = malloc ( iSize );
    return lpBuffer;
}

void __stdcall ADL_Main_Memory_Free ( void** lpBuffer )
{
    if ( NULL != *lpBuffer )
    {
        free ( *lpBuffer );
        *lpBuffer = NULL;
    }
}

//********************************
// Globals
//********************************
UINT aAllConnectedDisplays[MAX_NUM_DISPLAY_DEVICES]; //int array of connected displays for each of the ATI devices (aligned with sDriverNames)
ADLPROCS adlprocs = {0,0,0,0};
UCHAR ucSetCommandWrite[SETWRITESIZE]		=						{0x6e,0x51,0x84,0x03,0x00,0x00,0x00,0x00};
UCHAR ucGetCommandRequestWrite[GETRQWRITESIZE]	=		{0x6e,0x51,0x82,0x01,0x00,0x00};
UCHAR ucGetCommandCapabilitiesWrite[GETCAPWRITESIZE]={0x6e,0x51,0x83,0xf3,0x00,0x00,0x00};
UCHAR ucGetCommandReplyWrite[GETREPLYWRITESIZE]	=		{0x6f};
UCHAR ucGetCommandReplyRead[MAXREADSIZE];
UCHAR ucGlobalVcp = VCP_CODE_BRIGHTNESS;

LPAdapterInfo			lpAdapterInfo = NULL;
LPADLDisplayInfo	lpAdlDisplayInfo = NULL;
char 							MonitorNames[MAX_NUM_DISPLAY_DEVICES] [128];		// Array of Monitor names


//********************************
// Helper Functions
//********************************

//****************************************************************************
//
//  int iInitDisplayNames()
//
//  This function initializes the display name buffers passed in the 
//  parameter list as an array of CStrings 
//
//  Returns: The function will return the number of connected adapters.
//           So if only one adapter/driver is loaded, this will return 1.
//
//****************************************************************************/
int iInitDisplayNames(CString *aDriverNames)
{
  int			iNumberAdapters = 0;
  int			iAdapterIndex;
  int			iDisplayIndex;
  int			iNumDisplays;
  int			ADL_Err = ADL_ERR;

		adlprocs.ADL_Adapter_NumberOfAdapters_Get ( &iNumberAdapters );

        if ( 0 >=  iNumberAdapters )		// No adapters found!!!
			return 0;

        lpAdapterInfo = (LPAdapterInfo)malloc ( sizeof (AdapterInfo) * iNumberAdapters );
        memset ( lpAdapterInfo,'\0', sizeof (AdapterInfo) * iNumberAdapters );

        // Get the AdapterInfo structure for all adapters in the system
        adlprocs.ADL_Adapter_AdapterInfo_Get (lpAdapterInfo, sizeof (AdapterInfo) * iNumberAdapters);

		// Repeat for all available adapters in the system
		for ( int i = 0; i < iNumberAdapters; i++ )
		{
			iAdapterIndex = lpAdapterInfo[ i ].iAdapterIndex;
			ADL_Main_Memory_Free ( (void **) &lpAdlDisplayInfo );

			ADL_Err = adlprocs.ADL_Display_DisplayInfo_Get (lpAdapterInfo[i].iAdapterIndex, &iNumDisplays, &lpAdlDisplayInfo, 0);

			for ( int j = 0; j < iNumDisplays; j++ )
			{
				// For each display, check its status. Use the display only if it's connected AND mapped (iDisplayInfoValue: bit 0 and 1 )
                if (  ( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED ) != 
					( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED	& lpAdlDisplayInfo[ j ].iDisplayInfoValue ) )
					continue;   // Skip the not connected or non-active displays

				// Is the display mapped to this adapter?
//				if ( iAdapterIndex != lpAdlDisplayInfo[ j ].displayID.iDisplayLogicalAdapterIndex )
//					continue;
				
				// Preserve the Connected displays in PDL style :-)
				iDisplayIndex = lpAdlDisplayInfo[ j ].displayID.iDisplayLogicalIndex;
				aAllConnectedDisplays[ iAdapterIndex ] |= 1 << iDisplayIndex;

				strncpy_s( MonitorNames[ j ], lpAdlDisplayInfo[ j ].strDisplayName, 127 );
			}
			char tmp[4];
			sprintf_s( tmp, "%d", lpAdapterInfo[ i ].iAdapterIndex );
			aDriverNames[i] = tmp;
		}
		return iNumberAdapters;
}

// Function:
// void vGetCapabilitiesCommand
// Purpose:
// Get the MCCS capabilities
// Input: UCHAR ucVcp, VCP code (CONTRAST, BRIGHTNESS, etc)
//        int iDisplayIndex, display index
// Output: VOID
bool vGetCapabilitiesCommand(UCHAR ucVcp, int iAdapterIndex, int iDisplayIndex)
{
  unsigned int i;
  unsigned char chk=0;
  int val=0;
  int read_val=0;
  int temp_val=1;
  int replySize=0;
  bool ret=TRUE;

  ucGetCommandCapabilitiesWrite[CAP_LOW_OFFSET]=0;

  if (ucVcp == VCP_CODE_CAPABILITIES)
    replySize = GETREPLYCAPSIZEFIXED;
  else
    replySize = GETREPLYCAPSIZEVARIABLE;

  while(temp_val!=0)
  {
    // set the offset
    ucGetCommandCapabilitiesWrite[CAP_LOW_OFFSET]+=(UCHAR)val;
    // get checksum
    for ( i = 0; i < CAP_CHK_OFFSET; i++)
      chk=chk^ucGetCommandCapabilitiesWrite[i];

    ucGetCommandCapabilitiesWrite[CAP_CHK_OFFSET] = chk;

    // write get capability with offset
    vWriteI2c((char*)&ucGetCommandCapabilitiesWrite[0], GETCAPWRITESIZE, iAdapterIndex, iDisplayIndex);
    Sleep(40);
    // initial read to get the length to determine actual read length
    vWriteAndReadI2c((char*)&ucGetCommandReplyWrite[0], GETREPLYWRITESIZE, (char*)&ucGetCommandReplyRead[0], replySize,
                     iAdapterIndex, iDisplayIndex);

    if (ucVcp == VCP_CODE_CAPABILITIES_NEW)
    {
      // compute read length
      read_val= (int)ucGetCommandReplyRead[GETRP_LENGHTH_OFFSET] & 0x7F;
      read_val += 0x3;
      // re-write get capability with offset

      vWriteI2c((char*)&ucGetCommandCapabilitiesWrite[0],GETCAPWRITESIZE, iAdapterIndex, iDisplayIndex);
      Sleep(40);
      // read with actual read length computed from above
      vWriteAndReadI2c((char*)&ucGetCommandReplyWrite[0],GETREPLYWRITESIZE,(char*)&ucGetCommandReplyRead[0],read_val,
                       iAdapterIndex, iDisplayIndex);
    }
    if ((int)ucGetCommandReplyRead[GETRP_LENGHTH_OFFSET] == 0)
    {
      ret=FALSE;
      break;
    }
    // compute new offset
    val=(int)ucGetCommandReplyRead[GETRP_LENGHTH_OFFSET]-0x83;
    temp_val=val;
    chk=0;
  }

  return ret;
}



// Function:
// void vGetVcpCommand
// Purpose:
// Get the values from display based on which VCP code
// Input: UCHAR ucVcp, VCP code (CONTRAST, BRIGHTNESS, etc)
//        int*ulMaxVal, return value of the max possible value to can be set
//        int*ulCurVal, return value of current set
//        int iDisplayIndex, display index
// Output: VOID

void vGetVcpCommand(UCHAR ucVcp, UINT * ulMaxVal, UINT * ulCurVal, int iAdapterIndex, int iDisplayIndex)
{
  unsigned int i;
  unsigned char chk=0;
  int ADL_Err = ADL_ERR;

  ucGetCommandRequestWrite[GETRQ_VCPCODE_OFFSET]=ucVcp;

  for( i = 0; i < GETRQ_CHK_OFFSET; i++)
		chk = chk ^ ucGetCommandRequestWrite[ i ];

  ucGetCommandRequestWrite[GETRQ_CHK_OFFSET] = chk;

  ADL_Err = vWriteI2c( (char*) &ucGetCommandRequestWrite[0], GETRQWRITESIZE, iAdapterIndex, iDisplayIndex);
  Sleep(40);
  ADL_Err = vWriteAndReadI2c( (char*)&ucGetCommandReplyWrite[0], GETREPLYWRITESIZE, (char*) &ucGetCommandReplyRead[0],
										GETREPLYREADSIZE, iAdapterIndex, iDisplayIndex);

  *(ulMaxVal) = (ucGetCommandReplyRead[GETRP_MAXHIGH_OFFSET] << 8 |ucGetCommandReplyRead[GETRP_MAXLOW_OFFSET]);
  *(ulCurVal) = (ucGetCommandReplyRead[GETRP_CURHIGH_OFFSET] << 8 |ucGetCommandReplyRead[GETRP_CURLOW_OFFSET]);
}


//
// Function:
//  void vSetVcpCommand
// Purpose:
// Set the values from display based on which VCP code
// Input: UCHAR ucVcp, VCP code (CONTRAST, BRIGHTNESS, etc)
//       int ulVal, which value to set
//       int iDisplayIndex, display index
// Output: VOID
//
void vSetVcpCommand(UCHAR ucVcp, UINT ulVal, int iAdapterIndex, int iDisplayIndex)
{
  unsigned int i;
  unsigned char chk=0;
  int ADL_Err = ADL_ERR;

  ucSetCommandWrite[SET_VCPCODE_OFFSET] = ucVcp;
  ucSetCommandWrite[SET_LOW_OFFSET] = (char)(ulVal & 0x0ff);
  ucSetCommandWrite[SET_HIGH_OFFSET] = (char)((ulVal>>8) & 0x0ff);

  for( i = 0; i < SET_CHK_OFFSET; i++)
    chk=chk ^ ucSetCommandWrite[i];

  ucSetCommandWrite[SET_CHK_OFFSET] = chk;
  ADL_Err = vWriteI2c( (char*)&ucSetCommandWrite[0], SETWRITESIZE, iAdapterIndex, iDisplayIndex);
  Sleep(50);
}


// Function:
//	void vWriteI2c
// Purpose:
// Write to and read from an i2s address 
// Input: char * lpucSendMsgBuf			Data to write
//			int iSendMsgLen					Length of data
//			int iAdapterIndex, int iDisplayIndex
//	Output: result code
//
int vWriteI2c(char * lpucSendMsgBuf, int iSendMsgLen, int iAdapterIndex, int iDisplayIndex)
{
    int iRev = 0;
	return adlprocs.ADL_Display_DDCBlockAccess_Get( iAdapterIndex, iDisplayIndex, NULL, NULL, iSendMsgLen, lpucSendMsgBuf, &iRev, NULL);
}

// Function:
// void vWriteAndReadI2c
// Purpose:
// Write to and read from an i2s address 
// Input: char * lpucSendMsgBuf			Data to write
//			int iSendMsgLen					Length of data
//			char * lpucRecvMsgBuf			Read buffer
//			int iRecvMsgLen						Read buffer size
//			int iAdapterIndex, int iDisplayIndex
//	Output: result code
//
int vWriteAndReadI2c(char * lpucSendMsgBuf, int iSendMsgLen, char * lpucRecvMsgBuf, int iRecvMsgLen,  int iAdapterIndex, int iDisplayIndex)
{
	return adlprocs.ADL_Display_DDCBlockAccess_Get( iAdapterIndex, iDisplayIndex, NULL, NULL,
																			iSendMsgLen, lpucSendMsgBuf, &iRecvMsgLen, lpucRecvMsgBuf);
}

//******************************
// CDDCBlockAccessDlg dialog
//******************************
CDDCBlockAccessDlg::CDDCBlockAccessDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDDCBlockAccessDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDDCBlockAccessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_COMBO_VCPCODE, cb_VCPCode);
  DDX_Control(pDX, IDC_COMBO_DISPLAYINDEX, m_indexList);
  DDX_Control(pDX, IDC_COMBO_DRIVERINDEX, m_deviceList);
}

BEGIN_MESSAGE_MAP(CDDCBlockAccessDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
  ON_BN_CLICKED(IDOK, &CDDCBlockAccessDlg::OnBnClickedOk)
  ON_BN_CLICKED(IDC_BUTTON_GET, &CDDCBlockAccessDlg::OnBnClickedButtonGet)
  ON_BN_CLICKED(IDC_BUTTON_SET, &CDDCBlockAccessDlg::OnBnClickedButtonSet)
  ON_WM_DESTROY()
  ON_CBN_SELCHANGE(IDC_COMBO_VCPCODE, &CDDCBlockAccessDlg::OnCbnSelchangeComboVcpcode)
  ON_CBN_SELCHANGE(IDC_COMBO_DISPLAYINDEX, &CDDCBlockAccessDlg::OnCbnSelchangeComboDisplayindex)
  ON_CBN_SELCHANGE(IDC_COMBO_DRIVERINDEX, &CDDCBlockAccessDlg::OnCbnSelchangeComboDriverindex)
END_MESSAGE_MAP()


// CDDCBlockAccessDlg message handlers

BOOL CDDCBlockAccessDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	// when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

// Initialize ADL
  if ( !InitADL() )
		exit (1);

	memset(	&aAllConnectedDisplays[0], 0, sizeof (aAllConnectedDisplays) );
  m_deviceList.ResetContent();
  CString sDriverNames[MAX_NUM_DISPLAY_DEVICES];        //String array of all ATI device names (aligned with aAllConnectedDisplays)

  // Enumerate all AMD desktops
  int numberOfATIDevices = iInitDisplayNames( sDriverNames );

  for (int i = 0; i < MAX_NUM_DISPLAY_DEVICES ; i++)
  {
    if (sDriverNames[i].GetLength( ) != 0 )
    {
      //Add the name to the Driver ID combo box (AddString)
      //At the same time set that entry to have i (SetItemData)
      //i will be saved as the driver ID corresponding to this device name
      m_deviceList.SetItemData (m_deviceList.AddString(sDriverNames[i]), i);

      // No need to call ATIDL_GetConnectedDisplays(i, &aAllConnectedDisplays[i])  (Very expensive call).
	  // The connected and mapped displays are already recorded.
    }
  }
  m_deviceList.SetCurSel(0);

  //Refresh the display index for the first ATI device
  RefreshDisplayIndices ();

  // Init VCP combo box
  cb_VCPCode.AddString(_T("Luminance"));
  cb_VCPCode.AddString(_T("Contrast"));
  cb_VCPCode.AddString(_T("Color Temperature"));
  cb_VCPCode.AddString(_T("Get Capabilities (Fixed)"));
  cb_VCPCode.AddString(_T("Get Capabilities (Variable)"));
  cb_VCPCode.SetCurSel(0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDDCBlockAccessDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//This function is to refresh the Display index combo box
void CDDCBlockAccessDlg::RefreshDisplayIndices ()
{
  int iAdapterIndex = (int) m_deviceList.GetItemData ( m_deviceList.GetCurSel() );
  int ulDisplaysConnected = aAllConnectedDisplays[iAdapterIndex];

  //Clear the list
  m_indexList.ResetContent();

  // ulDisplayIndices is a bit vector of connected displays
  // for example, if ulDisplaysConnected == 0x9, i.e binary 1001, this means
  // you have two display devices connected - one has index 0, and another has index 3
  // another example: ulDisplaysConnected = 0x7, i.e. binary 0111, means
  // you have 3 displays connected - one has index 0, another has index 1, and another has index 2  
  for (int j = 0 ; ulDisplaysConnected != 0 ; j++ )
  {
    if (ulDisplaysConnected & 0x1)
    {
      TCHAR ucTemp[5];
      wsprintf(ucTemp,_T("%d"),j);

      //Add the display index to the combo box (AddString)
      //At the same time save the j into that entry (SetItemData)
      //j will be the display index corresponding to this entry
      m_indexList.SetItemData (m_indexList.AddString(ucTemp), j);
    }
    ulDisplaysConnected = ulDisplaysConnected>>1;
  }
  
  m_indexList.SetCurSel(0);
  int iDisplayIndex = (int)m_indexList.GetItemData ( m_indexList.GetCurSel() );

//  SetDlgItemText(IDC_EDIT_MONNAME, MonitorNames[ iDisplayIndex ] );
  std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  std::wstring wideAdapterName = conv.from_bytes(lpAdapterInfo[iAdapterIndex].strAdapterName);
  std::wstring wideDisplayName = conv.from_bytes(lpAdapterInfo[iAdapterIndex].strDisplayName);
  SetDlgItemText(IDC_EDIT_ANAME, wideAdapterName.c_str());
  SetDlgItemText(IDC_EDIT_DNAME, wideDisplayName.c_str());

  // Read the EDID of the first display
  OnCbnSelchangeComboDisplayindex();
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDDCBlockAccessDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDDCBlockAccessDlg::OnBnClickedOk()
{
  OnOK();
}

void CDDCBlockAccessDlg::OnBnClickedButtonGet()
{
  UINT ulMax=0, ulCur=0;
  TCHAR ucBuffer [16];
  int iAdapterIndex = (int) m_deviceList.GetItemData ( m_deviceList.GetCurSel() );
  int iDisplayIndex = (int) m_indexList.GetItemData ( m_indexList.GetCurSel() );

  if (ucGlobalVcp == VCP_CODE_CAPABILITIES || ucGlobalVcp == VCP_CODE_CAPABILITIES_NEW)
  {
    if (vGetCapabilitiesCommand(ucGlobalVcp, iAdapterIndex, iDisplayIndex))
    {
      SetDlgItemText(IDC_EDIT_MAX,_T("PASS"));
      SetDlgItemText(IDC_EDIT_CURRENT,_T("PASS"));
    }
    else
    {
      SetDlgItemText(IDC_EDIT_MAX,_T("FAIL"));
      SetDlgItemText(IDC_EDIT_CURRENT,_T("FAIL"));
    }
  }
  else
  {
    vGetVcpCommand( ucGlobalVcp, &ulMax, &ulCur, iAdapterIndex, iDisplayIndex );

    ulMax &= 0x0ffff;
    ulCur &= 0x0ffff;
    wsprintf(ucBuffer,_T("0x%x"),ulMax);
    SetDlgItemText(IDC_EDIT_MAX,ucBuffer);
    wsprintf(ucBuffer,_T("0x%x"),ulCur);
    SetDlgItemText(IDC_EDIT_CURRENT,ucBuffer);
  }
}

void CDDCBlockAccessDlg::OnBnClickedButtonSet()
{
  UINT ulCur=0;
  TCHAR ucBuffer[255];
  int iAdapterIndex = (int) m_deviceList.GetItemData ( m_deviceList.GetCurSel() );
  int iDisplayIndex = (int) m_indexList.GetItemData ( m_indexList.GetCurSel() );

  if( GetDlgItemText(IDC_EDIT_CURRENT,ucBuffer,10) > 0 )
  {
      if (_stscanf_s(ucBuffer, _T("%x"), &(ulCur)) > 0)
    {
      vSetVcpCommand(ucGlobalVcp,ulCur, iAdapterIndex, iDisplayIndex);
    }
  }
}

void CDDCBlockAccessDlg::OnDestroy()
{
  CDialog::OnDestroy();
  FreeADL();
}

void CDDCBlockAccessDlg::OnCbnSelchangeComboVcpcode()
{
  int sel = cb_VCPCode.GetCurSel();
  GetDlgItem(IDC_BUTTON_SET)->EnableWindow(TRUE);

  /***********************************
  / 0 - "Luminance"
  / 1 - "Contrast"
  / 2 - "Color Temperature"
  / 3 - "Get Capabilities (Fixed)"
  / 4 - "Get Capabilities (Variable)"
  /***********************************/
  switch (sel)
  {
  case 0:
    ucGlobalVcp = VCP_CODE_BRIGHTNESS;
    break;

  case 1: // contrast
    ucGlobalVcp = VCP_CODE_CONTRAST;
    break;

  case 2: // color temperature
    ucGlobalVcp = VCP_CODE_COLORTEMP;
    break;

  case 3: // get capabilities 38 read
    ucGlobalVcp = VCP_CODE_CAPABILITIES;
    GetDlgItem(IDC_BUTTON_SET)->EnableWindow(FALSE);
    break;

  case 4: // get capabilities new
    ucGlobalVcp = VCP_CODE_CAPABILITIES_NEW;
    GetDlgItem(IDC_BUTTON_SET)->EnableWindow(FALSE);
    break;
  }
}

void CDDCBlockAccessDlg::OnCbnSelchangeComboDisplayindex()
{
  TCHAR hexBuffer[1000*2];
  TCHAR tBuffer[1000*2];
  TCHAR cTemp[4*2];
  unsigned char ucSymbol;
  int iAdapterIndex = (int) m_deviceList.GetItemData ( m_deviceList.GetCurSel() );
  int iDisplayIndex = (int) m_indexList.GetItemData ( m_indexList.GetCurSel() );
  
  //Setup structures for the ADL call
  ADLDisplayEDIDData  aBlockOutput;
  ZeroMemory(&aBlockOutput, sizeof(ADLDisplayEDIDData));
  aBlockOutput.iSize   = sizeof(ADLDisplayEDIDData);
  aBlockOutput.iBlockIndex = 0; //First block
  
  std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  std::wstring wideChar = conv.from_bytes(MonitorNames[iDisplayIndex]);
  SetDlgItemText(IDC_EDIT_MONNAME, wideChar.c_str());

  //Call ADL to get the EDID
  adlprocs.ADL_Display_EdidData_Get(iAdapterIndex, iDisplayIndex, &aBlockOutput);

  //Printout
  _tcscpy_s(hexBuffer, _T(""));
  _tcscpy_s(tBuffer, _T(""));
  for (int i = 0; i < aBlockOutput.iEDIDSize ; i++)
  {
    //Newline every 8 bytes
    if (i != 0 && i% 8 == 0)
	{
        _tcscat_s(hexBuffer, _T("\r\n"));
        _tcscat_s(tBuffer, _T("\r\n"));
	}

    //add the byte to the buffer string
	ucSymbol = (unsigned char)aBlockOutput.cEDIDData[i];
    wsprintf( cTemp,_T(" %.2x"), ucSymbol );
    _tcscat_s(hexBuffer, cTemp);

    if ( ucSymbol < 0x20 )			// Mask control characters
		ucSymbol |= 0x20;

    wsprintf(cTemp, _T(" %c"), ucSymbol);
    _tcscat_s(tBuffer, cTemp);
  }

  //Print to Edit box  
  SetDlgItemText(IDC_EDIT_RAWEDID, hexBuffer);
  SetDlgItemText(IDC_EDIT_TEXTEDID, tBuffer);
}

void CDDCBlockAccessDlg::OnCbnSelchangeComboDriverindex()
{
  RefreshDisplayIndices ();
}


// Function:
// void InitADL
// Purpose:
// Initialize ADL functions
// Input: NONE
// Output: bool, Function successful or not
bool InitADL()
{
	int	ADL_Err = ADL_ERR;
  if (!adlprocs.hModule)
  {
    adlprocs.hModule = LoadLibrary(_T("atiadlxx.dll"));
    // A 32 bit calling application on 64 bit OS will fail to LoadLIbrary.
    // Try to load the 32 bit library (atiadlxy.dll) instead
    if (adlprocs.hModule == NULL)
      adlprocs.hModule = LoadLibrary(_T("atiadlxy.dll"));

    if(adlprocs.hModule)
    {
      adlprocs.ADL_Main_Control_Create       = (ADL_MAIN_CONTROL_CREATE) GetProcAddress(adlprocs.hModule, "ADL_Main_Control_Create");
      adlprocs.ADL_Main_Control_Destroy      = (ADL_MAIN_CONTROL_DESTROY) GetProcAddress(adlprocs.hModule, "ADL_Main_Control_Destroy");
      adlprocs.ADL_Adapter_NumberOfAdapters_Get   = (ADL_ADAPTER_NUMBEROFADAPTERS_GET) GetProcAddress(adlprocs.hModule, "ADL_Adapter_NumberOfAdapters_Get");
      adlprocs.ADL_Adapter_AdapterInfo_Get   = (ADL_ADAPTER_ADAPTERINFO_GET) GetProcAddress(adlprocs.hModule, "ADL_Adapter_AdapterInfo_Get");
      adlprocs.ADL_Display_DisplayInfo_Get   = (ADL_DISPLAY_DISPLAYINFO_GET) GetProcAddress(adlprocs.hModule, "ADL_Display_DisplayInfo_Get");
      adlprocs.ADL_Display_DDCBlockAccess_Get   = (ADL_DISPLAY_DDCBLOCKACCESSGET) GetProcAddress(adlprocs.hModule, "ADL_Display_DDCBlockAccess_Get");
      adlprocs.ADL_Display_EdidData_Get   = (ADL_DISPLAY_EDIDDATA_GET) GetProcAddress(adlprocs.hModule, "ADL_Display_EdidData_Get");
    }
    
    if(adlprocs.hModule == NULL              ||
		adlprocs.ADL_Main_Control_Create == NULL ||
		adlprocs.ADL_Main_Control_Destroy == NULL ||
		adlprocs.ADL_Adapter_NumberOfAdapters_Get == NULL ||
		adlprocs.ADL_Adapter_AdapterInfo_Get == NULL ||
		adlprocs.ADL_Display_DisplayInfo_Get == NULL ||
		adlprocs.ADL_Display_DDCBlockAccess_Get   == NULL || 
		adlprocs.ADL_Display_EdidData_Get   == NULL ) 
    {
      AfxMessageBox(_T("Error: ADL initialization failed! This app will NOT work!"), MB_OK, 0);
      return false;
    }
	// Initialize ADL with second parameter = 1, which means: Get the info for only currently active adapters!
    ADL_Err = adlprocs.ADL_Main_Control_Create (ADL_Main_Memory_Alloc, 1);

  }
  return (ADL_OK == ADL_Err) ?	true : false;
	  
}

// Function:
// void FreeADL
// Purpose:
// free the ADL Module
// Input: NONE
// Output: VOID
void FreeADL()
{

  ADL_Main_Memory_Free ( (void **)&lpAdapterInfo );
  ADL_Main_Memory_Free ( (void **)&lpAdlDisplayInfo );

  adlprocs.ADL_Main_Control_Destroy ();
  FreeLibrary (adlprocs.hModule);
  adlprocs.hModule = NULL;
}

