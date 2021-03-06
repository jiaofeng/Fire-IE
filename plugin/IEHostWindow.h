/*
This file is part of Fire-IE.

Fire-IE is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fire-IE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fire-IE.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "resource.h"
#include "IECtrl.h"
#include "Cookie\CookieManager.h"
namespace Plugin
{
	class CPlugin;
}

namespace HttpMonitor
{
	class MonitorSink;
}

namespace UserMessage
{
	// 自定义窗口消息
	static const UINT WM_USER_MESSAGE =  WM_USER + 200;

	// Sub-types of the user defined window message
	static const WPARAM WPARAM_SET_FIREFOX_COOKIE = 0;
	struct LParamSetFirefoxCookie
	{
		CString strURL;
		CString strCookie;
	};
}

// Firefox 4.0 开始采用了新的窗口结构
// 对于插件，是放在 GeckoPluginWindow 窗口里，往上有一个 MozillaWindowClass，再往上是顶层的
// MozillaWindowClass，我们的消息要发到顶层，所以再写一个查找的函数
HWND GetTopMozillaWindowClassWindow(HWND hwndIECtrl);


// CIEHostWindow dialog

class CIEHostWindow : public CDialog
{
	DECLARE_DYNAMIC(CIEHostWindow)
	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()

	friend class HttpMonitor::MonitorSink;
	friend class Cookie::CookieManager;
	
public:
	static CIEHostWindow* CreateNewIEHostWindow(DWORD dwId);

	/** 根据 CIEHostWindow 的 HWND 寻找对应的 CIEHostWindow 对象 */
	static CIEHostWindow* GetInstance(HWND hwnd);

	/** 根据Internet Explorer_Server窗口找到对应的 CIEHostWindow 对象*/
	static CIEHostWindow* FromInternetExplorerServer(HWND hwndIEServer);

	static void AddCookieIEWindow(CIEHostWindow *pWnd);

	static void SetFirefoxCookie(CString strURL, CString strCookie);

	/** 获取Firefox UserAgent*/
	static CString GetFirefoxUserAgent();

	/** 
	 * 获取IE控件UserAgent
	 * 由于IE控件没有提供直接获取UserAgent的接口，需要从IE控件加载的HTML
	 * 文档中获取UserAgent。
	 * @return 在IE控件首次加载完毕后，该类函数才能返回正确的UserAgent;否则返回空字符串。
	 */
	static CString GetIEUserAgentString() {return s_strIEUserAgent;}
		
public:
	
	virtual ~CIEHostWindow();

	virtual BOOL CreateControlSite(COleControlContainer* pContainer, 
		COleControlSite** ppSite, UINT nID, REFCLSID clsid);

	// Dialog Data
	enum { IDD = IDD_MAIN_WINDOW };

	// Overrides
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	virtual BOOL Create(UINT nIDTemplate,CWnd* pParentWnd = NULL);

	/** 设置窗口关联的Plugin对象 */
	void SetPlugin(Plugin::CPlugin* pPlugin) {m_pPlugin = pPlugin;}
protected:
	CIEHostWindow(Plugin::CPlugin* pPlugin = NULL, CWnd* pParent = NULL);   // standard constructor

	/** HWND到 CIEWindow 对象的映射, 用于通过 HWND 快速找到已打开的 CIEWindow 对象 */
	static CSimpleMap<HWND, CIEHostWindow *> s_IEWindowMap;
	
	/** 与 s_IEWindowMap 配对使用的, 保证线程安全 */
	static CCriticalSection s_csIEWindowMap;

	/** ID到 CIEWindow 对象的映射, 用于通过 ID 快速找到创建后未使用的 CIEWindow 对象 */
	static CSimpleMap<DWORD, CIEHostWindow *> s_NewIEWindowMap;

	/** 与 s_csNewIEWindowMap 配对使用的, 保证线程安全 */
	static CCriticalSection s_csNewIEWindowMap;

	/** 用于同步Cookie的 CIEHostWindow 对象 */
	static CSimpleMap<HWND, CIEHostWindow *> s_CookieIEWindowMap;

	/** 与 s_CookieIEWindowMap 配对使用的, 保证线程安全 */
	static CCriticalSection s_csCookieIEWindowMap;

	/** IE控件的UserAgent */
	static CString s_strIEUserAgent;

	void InitIE();
	void UninitIE();

	// 从IE控件的HTML文档中获取UserAgent
	CString GetDocumentUserAgent();

	// 检测浏览器命令是否可用
	BOOL IsOleCmdEnabled(OLECMDID cmdID);

	// 执行浏览器命令
	void ExecOleCmd(OLECMDID cmdID);

	// 自定义窗口消息响应函数
	void OnSetFirefoxCookie(const CString& strURL, const CString& strCookie);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnUserMessage(WPARAM wParam, LPARAM lParam);
	void OnCommandStateChange(long Command, BOOL Enable);
	void OnStatusTextChange(LPCTSTR Text);
	void OnTitleChange(LPCTSTR Text);
	void OnProgressChange(long Progress, long ProgressMax);
	void OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, BOOL* Cancel);
	void OnDocumentComplete(LPDISPATCH pDisp, VARIANT* URL);
	void OnNewWindow3Ie(LPDISPATCH* ppDisp, BOOL* Cancel, unsigned long dwFlags, LPCTSTR bstrUrlContext, LPCTSTR bstrUrl);

public:
	CIECtrl m_ie;

	/** 正在加载的 URL. */
	CString m_strLoadingUrl;

	// plugin methods
	void Navigate(const CString& strURL);
	void Refresh();
	void Stop();
	void Back();
	void Forward();
	void Focus();
	void Copy();
	void Cut();
	void Paste();
	void SelectAll();
	void Find();
	void HandOverFocus();
	void Zoom(double level);
	void DisplaySecurityInfo();
	void SaveAs();
	void Print();
	void PrintPreview();
	void PrintSetup();

	// read only plugin properties
	CString GetURL();
	CString GetTitle();
	CString GetFaviconURL();
	CString GetFaviconURLFromContent();
	BOOL GetCanBack() {return m_bCanBack;}
	BOOL GetCanForward() {return m_bCanForward;}
	BOOL GetCanStop() {return IsOleCmdEnabled(OLECMDID_STOP);}
	BOOL GetCanRefresh() {return IsOleCmdEnabled(OLECMDID_REFRESH);}
	BOOL GetCanCopy(){return IsOleCmdEnabled(OLECMDID_COPY);}
	BOOL GetCanCut(){return IsOleCmdEnabled(OLECMDID_CUT);}
	BOOL GetCanPaste(){return IsOleCmdEnabled(OLECMDID_PASTE);}
	BOOL GetCanSelectAll(){return IsOleCmdEnabled(OLECMDID_SELECTALL);}
	INT32 GetProgress() {return m_iProgress;}

	// plugin events
	void OnTitleChanged(const CString& title);
	void OnIEProgressChanged(INT32 iProgress);
	void OnStatusChanged(const CString& message);
	void OnCloseIETab();

protected:
	BOOL m_bCanBack;
	BOOL m_bCanForward;
	INT32 m_iProgress;

	/** DIRTY FIX: NewWindow3 里面创建的 IE 窗口不能设置 Referrer */
	CString m_strUrlContext;

	/** 缓存最近的 Favicion URL */
	CString m_strFaviconURL;
	
	Plugin::CPlugin* m_pPlugin;
};
