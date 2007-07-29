#pragma once

class CMainFrame;

class CSKMobileView;

class Controller
{
public :

	virtual void Init(CMainFrame * mainFrame) = 0;

	virtual void Init(CSKMobileView * view) = 0;

	virtual void Fini() = 0;

	virtual BOOL OnCopyData(CWindow wnd, PCOPYDATASTRUCT pCopyDataStruct) = 0;

	virtual LRESULT OnBackLastApplication(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) = 0;

	virtual LRESULT OnSaveContentHtml(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) = 0;

	virtual LRESULT OnSaveContentXml(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) = 0;

	// virtual LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) = 0;

	virtual LRESULT OnInputChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) = 0;

	virtual void OnTimer(UINT_PTR nIDEvent) = 0;

	virtual LRESULT OnInputSetFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) = 0;

	virtual LRESULT OnInputKillFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) = 0;

	virtual LRESULT OnContentTabChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) = 0;

	virtual LRESULT OnDocumentComplete(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) = 0;

	virtual LRESULT OnHotspot(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) = 0;

	virtual LRESULT OnLookup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) = 0;

	virtual LRESULT OnAction(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) = 0;

	virtual LRESULT OnCloseTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) = 0;

	virtual LRESULT OnQueryMode(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) = 0;

	virtual BOOL OnReturnKey(UINT nRepCnt, UINT nFlags) = 0;

	virtual BOOL OnUpKey(UINT nRepCnt, UINT nFlags) = 0;

	virtual BOOL OnDownKey(UINT nRepCnt, UINT nFlags) = 0;

	virtual BOOL OnLeftKey(UINT nRepCnt, UINT nFlags) = 0;

	virtual BOOL OnRightKey(UINT nRepCnt, UINT nFlags) = 0;

};

class ControllerFactory
{
public :

	void listAvailableControllerNames(char const* names[], unsigned * count);

	Controller * create(char const* name);

};