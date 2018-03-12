#pragma once

#ifndef __H_C_CSROOLDIALOG__
#define __H_C_CSROOLDIALOG__

template <class T>
class CScrollDialog
{
public:
	
	BEGIN_MSG_MAP(CScrollDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_VSCROLL, OnVertScroll)		
		MESSAGE_HANDLER(WM_HSCROLL, OnHorzScroll)	
		//MESSAGE_HANDLER(WM_SIZE, OnSizeChanged)				
		//MESSAGE_HANDLER(WM_WINDOWPOSCHANGED , OnWindowPosChanged)			
	END_MSG_MAP()	

	CScrollDialog(){Minimized=FALSE;};
	BOOL Minimized;
	int count;
	LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{		
		T* pT = static_cast<T*>(this);
		LPWINDOWPOS lpwp = (LPWINDOWPOS) lParam;
		if(lpwp->flags&SWP_NOOWNERZORDER)		
		{	
			if(!Minimized)
			{				
				Minimized=TRUE;
				//pT->ShowWindow(SW_MINIMIZE);
			}
			else
			{
				
				//pT->ShowWindow(SW_SHOWNORMAL);
			}
		}		
		

		bHandled=FALSE;
		return 0;

	}
	

	LRESULT OnSizeChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{		
		int fwSizeType = wParam; 
		int nWidth = LOWORD(lParam); 
		int nHeight = HIWORD(lParam);

		T* pT = static_cast<T*>(this);

		switch(fwSizeType)
		{
		case SIZE_MINIMIZED:			
			if(!Minimized)
			{
				Minimized=TRUE;
				pT->ShowWindow(SW_MINIMIZE);				
			}
			break;	
		case SIZE_RESTORED:
		case SIZE_MAXIMIZED:
			if(Minimized)
			{
				Minimized=FALSE;
				pT->ShowWindow(SW_SHOW);				
			}
			break;
		default:
			bHandled=FALSE;
			break;
		}		
		return 0;
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{			
		T* pT = static_cast<T*>(this);
		
		RECT rwindow;
		pT->GetWindowRect(&rwindow);

		int AdditionalPointsVert = (rwindow.bottom-rwindow.top)-GetSystemMetrics(SM_CYSCREEN);

		if(AdditionalPointsVert>0)
		{			
			SCROLLINFO Info={sizeof(SCROLLINFO),SIF_ALL,0,0,0,0,0};
			pT->GetScrollInfo(SB_VERT,&Info);		
			Info.nMax+=((AdditionalPointsVert/2)*GetSystemMetrics(SM_CYSCREEN)/320);
			//if(GetSystemMetrics(SM_CYSCREEN)==320)Info.nMax+=1000; // some troubles with some devices
			pT->SetScrollInfo(SB_VERT,&Info);
		}

		bHandled=FALSE;		
		return 0;
	}

	int PerformScroll(WPARAM wParam, LPARAM lParam,DWORD Flags)
	{
		if(!lParam)	 return 0;
		
		int nScrollCode = (int)LOWORD(wParam); 		
		//int nPos = (short int)HIWORD(wParam); 
		HWND hwndScrollBar = (HWND) lParam;			

		SCROLLINFO Info={sizeof(SCROLLINFO),SIF_ALL,0,0,0,0,0};
		if(!GetScrollInfo(hwndScrollBar,Flags,&Info)) return 0;

		bool Arrows=false;	
		switch (nScrollCode)
		{
		case SB_LINEUP:
			Info.nTrackPos=Info.nPos-5;
			Arrows=TRUE;
			break;
		case SB_LINEDOWN:
			Info.nTrackPos=Info.nPos+5;
			Arrows=TRUE;
			break;
		case SB_PAGEUP:
			if(Info.nPage==0) Info.nPage=20; // мышкой щелкнули
			Info.nTrackPos=Info.nPos-Info.nPage;
			Arrows=TRUE;
			break;
		case SB_PAGEDOWN:
			if(Info.nPage==0) Info.nPage=20; // мышкой щелкнули

			Info.nTrackPos=Info.nPos+Info.nPage;
			Arrows=TRUE;
			break;
		case SB_THUMBTRACK:
		case SB_TOP:
		case SB_BOTTOM:
		case SB_ENDSCROLL:
			break;
		default: 
			// unknown
			break;
		}

		Info.nTrackPos = min(Info.nTrackPos, Info.nMax);
		Info.nTrackPos = max(Info.nTrackPos, Info.nMin);	

		int Different=Info.nPos-Info.nTrackPos;
		if (Different)
		{	
			int dx=0,dy=Different;
			if(Flags==SB_HORZ) {dx=Different;dy=0;};
			SetScrollPos(hwndScrollBar,Flags, Info.nTrackPos,Arrows);																
	
			ScrollWindowEx(hwndScrollBar,dx,dy,
				NULL,NULL,NULL,NULL,
				SW_SCROLLCHILDREN);							

			RedrawWindow(hwndScrollBar,NULL,NULL,RDW_INVALIDATE|RDW_NOCHILDREN);
		}	

		return 0;
	}

	LRESULT OnVertScroll(UINT uMsg, WPARAM wParam, LPARAM 
		lParam, BOOL& /*bHandled*/)
	{			
		return PerformScroll(wParam,lParam,SB_VERT);
	}

	LRESULT OnHorzScroll(UINT uMsg, WPARAM wParam, LPARAM 
		lParam, BOOL& /*bHandled*/)
	{			
		return PerformScroll(wParam,lParam,SB_HORZ);				
	}
};



#endif //__H_C_CSROOLDIALOG__