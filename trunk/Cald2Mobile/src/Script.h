#pragma once

#include <msscript.h>

class CScript
{
public :

	CScript() : isReady_(FALSE) {};

	void Set(CComPtr<IDispatch> const& scriptDispatch)
	{
		this->pScriptControl_ = scriptDispatch;
		isReady_ = TRUE;
	}

	BOOL isReady() {
		return this->isReady_;
	}

	HRESULT Eval(wchar_t const* expression, CComVariant * result)
	{
		return Eval(CComBSTR(expression), result);
	}

	HRESULT ExecuteStatement(wchar_t const* expression, CComVariant * result)
	{
		return Eval(CComBSTR(expression), result);
	}

	HRESULT Run(wchar_t const* procedureName, CComVariant * result, size_t argc, wchar_t const* args[])
	{
		SK_TRACE(SK_LOG_DEBUG, _T("CScript::Run(procedureName = %s)"), procedureName);

		CComSafeArray<VARIANT> comParams;
		for(size_t i = 0; i < argc; i++)
		{
			SK_TRACE(SK_LOG_DEBUG, _T("param[%d] = %s"), i, args[i]);
			CComVariant param(args[i]);
			comParams.Add(param);
		}

		HRESULT rc = Run(CComBSTR(procedureName), comParams, result);
		if(FAILED(rc))
		{
			THROW_RUNTIME_EXCEPTION(_T("Failed to run '") << procedureName << _T(" (") << args << _T(")'"));
		}
		return rc;
	}

	HRESULT Eval(CComBSTR const& expression, CComVariant * result)
	{
		if(!isReady_)
			return S_FALSE;

		VARIANT var;
		HRESULT rc = pScriptControl_->Eval(expression, &var);
		if(NULL != result) 
		{
			*result = var;
		}
		return rc;
	}

	HRESULT ExecuteStatement(CComBSTR const& expression, CComVariant * result)
	{
		if(!isReady_)
			return S_FALSE;

		VARIANT var;
		HRESULT rc = pScriptControl_->ExecuteStatement(expression, &var);
		if(NULL != result) 
		{
			*result = var;
		}
		return rc;
	}

	HRESULT Run(CComBSTR const& procedureName, CComSafeArray<VARIANT> & params, CComVariant * result)
	{
		if(!isReady_)
			return S_FALSE;

		VARIANT var;
		HRESULT rc = pScriptControl_->Run(procedureName, params.GetSafeArrayPtr(), &var);
		if(NULL != result) 
		{
			*result = var;
		}
		return rc;
	}

	HRESULT AddObject( 
		CComBSTR name,
		IDispatch * object,
		VARIANT_BOOL addMember = 0)
	{
		return pScriptControl_->AddObject(name, object, addMember);
	};

protected :

	BOOL isReady_;

	CComQIPtr<IScriptControl> pScriptControl_;
};
