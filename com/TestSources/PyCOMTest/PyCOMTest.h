
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0280 */
/* at Wed Feb 14 16:26:20 2001
 */
/* Compiler settings for PyCOMTest.idl:
    Os (OptLev=s), W1, Zp8, env=Win32 (32b run), ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __PyCOMTest_h__
#define __PyCOMTest_h__

/* Forward Declarations */ 

#ifndef __ISimpleCounter_FWD_DEFINED__
#define __ISimpleCounter_FWD_DEFINED__
typedef interface ISimpleCounter ISimpleCounter;
#endif 	/* __ISimpleCounter_FWD_DEFINED__ */


#ifndef __ISimpleCounterPro_FWD_DEFINED__
#define __ISimpleCounterPro_FWD_DEFINED__
typedef interface ISimpleCounterPro ISimpleCounterPro;
#endif 	/* __ISimpleCounterPro_FWD_DEFINED__ */


#ifndef __CoSimpleCounter_FWD_DEFINED__
#define __CoSimpleCounter_FWD_DEFINED__

#ifdef __cplusplus
typedef class CoSimpleCounter CoSimpleCounter;
#else
typedef struct CoSimpleCounter CoSimpleCounter;
#endif /* __cplusplus */

#endif 	/* __CoSimpleCounter_FWD_DEFINED__ */


#ifndef __CoPyCOMTest_FWD_DEFINED__
#define __CoPyCOMTest_FWD_DEFINED__

#ifdef __cplusplus
typedef class CoPyCOMTest CoPyCOMTest;
#else
typedef struct CoPyCOMTest CoPyCOMTest;
#endif /* __cplusplus */

#endif 	/* __CoPyCOMTest_FWD_DEFINED__ */


#ifndef __IPyCOMTest_FWD_DEFINED__
#define __IPyCOMTest_FWD_DEFINED__
typedef interface IPyCOMTest IPyCOMTest;
#endif 	/* __IPyCOMTest_FWD_DEFINED__ */


#ifndef __CoPyCOMTest2_FWD_DEFINED__
#define __CoPyCOMTest2_FWD_DEFINED__

#ifdef __cplusplus
typedef class CoPyCOMTest2 CoPyCOMTest2;
#else
typedef struct CoPyCOMTest2 CoPyCOMTest2;
#endif /* __cplusplus */

#endif 	/* __CoPyCOMTest2_FWD_DEFINED__ */


#ifndef __IPyCOMTest2_FWD_DEFINED__
#define __IPyCOMTest2_FWD_DEFINED__
typedef interface IPyCOMTest2 IPyCOMTest2;
#endif 	/* __IPyCOMTest2_FWD_DEFINED__ */


#ifndef __IPyCOMTestEvent_FWD_DEFINED__
#define __IPyCOMTestEvent_FWD_DEFINED__
typedef interface IPyCOMTestEvent IPyCOMTestEvent;
#endif 	/* __IPyCOMTestEvent_FWD_DEFINED__ */


#ifndef __PyCOMTestEvent_FWD_DEFINED__
#define __PyCOMTestEvent_FWD_DEFINED__
typedef interface PyCOMTestEvent PyCOMTestEvent;
#endif 	/* __PyCOMTestEvent_FWD_DEFINED__ */


#ifndef __IPyCOMTestNoDispatch_FWD_DEFINED__
#define __IPyCOMTestNoDispatch_FWD_DEFINED__
typedef interface IPyCOMTestNoDispatch IPyCOMTestNoDispatch;
#endif 	/* __IPyCOMTestNoDispatch_FWD_DEFINED__ */


#ifndef __IPyCOMTestNoDispatchEvent_FWD_DEFINED__
#define __IPyCOMTestNoDispatchEvent_FWD_DEFINED__
typedef interface IPyCOMTestNoDispatchEvent IPyCOMTestNoDispatchEvent;
#endif 	/* __IPyCOMTestNoDispatchEvent_FWD_DEFINED__ */


#ifndef __CoPyCOMTestNoDispatch_FWD_DEFINED__
#define __CoPyCOMTestNoDispatch_FWD_DEFINED__

#ifdef __cplusplus
typedef class CoPyCOMTestNoDispatch CoPyCOMTestNoDispatch;
#else
typedef struct CoPyCOMTestNoDispatch CoPyCOMTestNoDispatch;
#endif /* __cplusplus */

#endif 	/* __CoPyCOMTestNoDispatch_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_PyCOMTest_0000 */
/* [local] */ 

typedef 
enum EnumTestAttributes1
    {	TestAttr1	= 0,
	TestAttr1_1	= TestAttr1 + 1
    }	TestAttributes1;

typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_PyCOMTest_0000_0001
    {	TestAttr2	= 0
    }	TestAttributes2;



extern RPC_IF_HANDLE __MIDL_itf_PyCOMTest_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_PyCOMTest_0000_v0_0_s_ifspec;


#ifndef __PyCOMTestLib_LIBRARY_DEFINED__
#define __PyCOMTestLib_LIBRARY_DEFINED__

/* library PyCOMTestLib */
/* [helpstring][version][uuid] */ 

typedef /* [public] */ VARIANT_BOOL QsBoolean;

const QsBoolean True = VARIANT_TRUE;
const QsBoolean False = VARIANT_FALSE;
typedef /* [public] */ long HCON;

typedef /* [public] */ HCON CONNECTID;

typedef /* [uuid] */  DECLSPEC_UUID("14894ca0-554a-11d0-ae5f-cadd4c000000") 
enum tagQsAttribute
    {	Attr1	= 0,
	Attr2	= Attr1 + 1,
	Attr3	= 0x80000000,
	NumberOfAttribs	= Attr3 + 1
    }	QsAttribute;


enum TestAttributes3
    {	TestAttr3	= 0
    };
typedef /* [version][uuid] */  DECLSPEC_UUID("7a4ce6a7-7959-4e85-a3c0-b41442ff0f67") struct tagTestStruct1
    {
    int int_value;
    BSTR str_value;
    }	TestStruct1;


EXTERN_C const IID LIBID_PyCOMTestLib;

#ifndef __ISimpleCounter_INTERFACE_DEFINED__
#define __ISimpleCounter_INTERFACE_DEFINED__

/* interface ISimpleCounter */
/* [unique][helpstring][dual][uuid][public][object] */ 


EXTERN_C const IID IID_ISimpleCounter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("528d6940-5a31-11d0-ae5f-cadd4c000000")
    ISimpleCounter : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *retval) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ long Index,
            /* [retval][out] */ VARIANT __RPC_FAR *retval) = 0;
        
        virtual /* [helpstring][id][restricted][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_LBound( 
            /* [in] */ long lbound) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_LBound( 
            /* [retval][out] */ long __RPC_FAR *lbound) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_UBound( 
            /* [in] */ long lbound) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_UBound( 
            /* [retval][out] */ long __RPC_FAR *lbound) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetBounds( 
            /* [in] */ long lowerbound,
            /* [in] */ long upperbound) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetBounds( 
            /* [out] */ long __RPC_FAR *lbound,
            /* [out] */ long __RPC_FAR *ubound) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_TestProperty( 
            /* [in] */ long propval1,
            /* [defaultvalue][in] */ long propval2 = 0) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_TestPropertyWithDef( 
            /* [defaultvalue][in] */ long arg1,
            /* [retval][out] */ long __RPC_FAR *result) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_TestPropertyNoDef( 
            /* [in] */ long arg1,
            /* [retval][out] */ long __RPC_FAR *result) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_TestProperty2( 
            /* [in] */ long propval1,
            /* [in] */ long propval2,
            /* [defaultvalue][in] */ long propval3 = 0) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISimpleCounterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISimpleCounter __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISimpleCounter __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISimpleCounter __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISimpleCounter __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISimpleCounter __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISimpleCounter __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISimpleCounter __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            ISimpleCounter __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *retval);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Item )( 
            ISimpleCounter __RPC_FAR * This,
            /* [in] */ long Index,
            /* [retval][out] */ VARIANT __RPC_FAR *retval);
        
        /* [helpstring][id][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            ISimpleCounter __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *retval);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LBound )( 
            ISimpleCounter __RPC_FAR * This,
            /* [in] */ long lbound);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LBound )( 
            ISimpleCounter __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *lbound);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_UBound )( 
            ISimpleCounter __RPC_FAR * This,
            /* [in] */ long lbound);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_UBound )( 
            ISimpleCounter __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *lbound);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetBounds )( 
            ISimpleCounter __RPC_FAR * This,
            /* [in] */ long lowerbound,
            /* [in] */ long upperbound);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBounds )( 
            ISimpleCounter __RPC_FAR * This,
            /* [out] */ long __RPC_FAR *lbound,
            /* [out] */ long __RPC_FAR *ubound);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_TestProperty )( 
            ISimpleCounter __RPC_FAR * This,
            /* [in] */ long propval1,
            /* [defaultvalue][in] */ long propval2);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TestPropertyWithDef )( 
            ISimpleCounter __RPC_FAR * This,
            /* [defaultvalue][in] */ long arg1,
            /* [retval][out] */ long __RPC_FAR *result);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TestPropertyNoDef )( 
            ISimpleCounter __RPC_FAR * This,
            /* [in] */ long arg1,
            /* [retval][out] */ long __RPC_FAR *result);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_TestProperty2 )( 
            ISimpleCounter __RPC_FAR * This,
            /* [in] */ long propval1,
            /* [in] */ long propval2,
            /* [defaultvalue][in] */ long propval3);
        
        END_INTERFACE
    } ISimpleCounterVtbl;

    interface ISimpleCounter
    {
        CONST_VTBL struct ISimpleCounterVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISimpleCounter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISimpleCounter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISimpleCounter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISimpleCounter_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISimpleCounter_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISimpleCounter_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISimpleCounter_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISimpleCounter_get_Count(This,retval)	\
    (This)->lpVtbl -> get_Count(This,retval)

#define ISimpleCounter_get_Item(This,Index,retval)	\
    (This)->lpVtbl -> get_Item(This,Index,retval)

#define ISimpleCounter_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ISimpleCounter_put_LBound(This,lbound)	\
    (This)->lpVtbl -> put_LBound(This,lbound)

#define ISimpleCounter_get_LBound(This,lbound)	\
    (This)->lpVtbl -> get_LBound(This,lbound)

#define ISimpleCounter_put_UBound(This,lbound)	\
    (This)->lpVtbl -> put_UBound(This,lbound)

#define ISimpleCounter_get_UBound(This,lbound)	\
    (This)->lpVtbl -> get_UBound(This,lbound)

#define ISimpleCounter_SetBounds(This,lowerbound,upperbound)	\
    (This)->lpVtbl -> SetBounds(This,lowerbound,upperbound)

#define ISimpleCounter_GetBounds(This,lbound,ubound)	\
    (This)->lpVtbl -> GetBounds(This,lbound,ubound)

#define ISimpleCounter_put_TestProperty(This,propval1,propval2)	\
    (This)->lpVtbl -> put_TestProperty(This,propval1,propval2)

#define ISimpleCounter_get_TestPropertyWithDef(This,arg1,result)	\
    (This)->lpVtbl -> get_TestPropertyWithDef(This,arg1,result)

#define ISimpleCounter_get_TestPropertyNoDef(This,arg1,result)	\
    (This)->lpVtbl -> get_TestPropertyNoDef(This,arg1,result)

#define ISimpleCounter_put_TestProperty2(This,propval1,propval2,propval3)	\
    (This)->lpVtbl -> put_TestProperty2(This,propval1,propval2,propval3)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISimpleCounter_get_Count_Proxy( 
    ISimpleCounter __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *retval);


void __RPC_STUB ISimpleCounter_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ISimpleCounter_get_Item_Proxy( 
    ISimpleCounter __RPC_FAR * This,
    /* [in] */ long Index,
    /* [retval][out] */ VARIANT __RPC_FAR *retval);


void __RPC_STUB ISimpleCounter_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][restricted][propget] */ HRESULT STDMETHODCALLTYPE ISimpleCounter_get__NewEnum_Proxy( 
    ISimpleCounter __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB ISimpleCounter_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE ISimpleCounter_put_LBound_Proxy( 
    ISimpleCounter __RPC_FAR * This,
    /* [in] */ long lbound);


void __RPC_STUB ISimpleCounter_put_LBound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISimpleCounter_get_LBound_Proxy( 
    ISimpleCounter __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *lbound);


void __RPC_STUB ISimpleCounter_get_LBound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE ISimpleCounter_put_UBound_Proxy( 
    ISimpleCounter __RPC_FAR * This,
    /* [in] */ long lbound);


void __RPC_STUB ISimpleCounter_put_UBound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISimpleCounter_get_UBound_Proxy( 
    ISimpleCounter __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *lbound);


void __RPC_STUB ISimpleCounter_get_UBound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ISimpleCounter_SetBounds_Proxy( 
    ISimpleCounter __RPC_FAR * This,
    /* [in] */ long lowerbound,
    /* [in] */ long upperbound);


void __RPC_STUB ISimpleCounter_SetBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ISimpleCounter_GetBounds_Proxy( 
    ISimpleCounter __RPC_FAR * This,
    /* [out] */ long __RPC_FAR *lbound,
    /* [out] */ long __RPC_FAR *ubound);


void __RPC_STUB ISimpleCounter_GetBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE ISimpleCounter_put_TestProperty_Proxy( 
    ISimpleCounter __RPC_FAR * This,
    /* [in] */ long propval1,
    /* [defaultvalue][in] */ long propval2);


void __RPC_STUB ISimpleCounter_put_TestProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISimpleCounter_get_TestPropertyWithDef_Proxy( 
    ISimpleCounter __RPC_FAR * This,
    /* [defaultvalue][in] */ long arg1,
    /* [retval][out] */ long __RPC_FAR *result);


void __RPC_STUB ISimpleCounter_get_TestPropertyWithDef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISimpleCounter_get_TestPropertyNoDef_Proxy( 
    ISimpleCounter __RPC_FAR * This,
    /* [in] */ long arg1,
    /* [retval][out] */ long __RPC_FAR *result);


void __RPC_STUB ISimpleCounter_get_TestPropertyNoDef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE ISimpleCounter_put_TestProperty2_Proxy( 
    ISimpleCounter __RPC_FAR * This,
    /* [in] */ long propval1,
    /* [in] */ long propval2,
    /* [defaultvalue][in] */ long propval3);


void __RPC_STUB ISimpleCounter_put_TestProperty2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISimpleCounter_INTERFACE_DEFINED__ */


#ifndef __ISimpleCounterPro_DISPINTERFACE_DEFINED__
#define __ISimpleCounterPro_DISPINTERFACE_DEFINED__

/* dispinterface ISimpleCounterPro */
/* [uuid] */ 


EXTERN_C const IID DIID_ISimpleCounterPro;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("e29d77a0-04ca-11d2-a69a-00aa00125a98")
    ISimpleCounterPro : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct ISimpleCounterProVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISimpleCounterPro __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISimpleCounterPro __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISimpleCounterPro __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISimpleCounterPro __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISimpleCounterPro __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISimpleCounterPro __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISimpleCounterPro __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } ISimpleCounterProVtbl;

    interface ISimpleCounterPro
    {
        CONST_VTBL struct ISimpleCounterProVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISimpleCounterPro_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISimpleCounterPro_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISimpleCounterPro_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISimpleCounterPro_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISimpleCounterPro_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISimpleCounterPro_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISimpleCounterPro_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __ISimpleCounterPro_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CoSimpleCounter;

#ifdef __cplusplus

class DECLSPEC_UUID("b88dd310-bae8-11d0-ae86-76f2c1000000")
CoSimpleCounter;
#endif

EXTERN_C const CLSID CLSID_CoPyCOMTest;

#ifdef __cplusplus

class DECLSPEC_UUID("8ee0c520-5605-11d0-ae5f-cadd4c000000")
CoPyCOMTest;
#endif

#ifndef __IPyCOMTest_INTERFACE_DEFINED__
#define __IPyCOMTest_INTERFACE_DEFINED__

/* interface IPyCOMTest */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IPyCOMTest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a0d9ceb0-5605-11d0-ae5f-cadd4c000000")
    IPyCOMTest : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Start( 
            /* [retval][out] */ HCON __RPC_FAR *pnID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( 
            /* [in] */ CONNECTID nID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StopAll( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Test( 
            /* [in] */ VARIANT key,
            /* [in] */ QsBoolean inval,
            /* [retval][out] */ QsBoolean __RPC_FAR *retval) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Test2( 
            /* [in] */ QsAttribute inval,
            /* [retval][out] */ QsAttribute __RPC_FAR *retval) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Test3( 
            /* [in] */ TestAttributes1 inval,
            /* [retval][out] */ TestAttributes1 __RPC_FAR *retval) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Test4( 
            /* [in] */ TestAttributes2 inval,
            /* [retval][out] */ TestAttributes2 __RPC_FAR *retval) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Test5( 
            /* [out][in] */ TestAttributes1 __RPC_FAR *inout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSetInterface( 
            /* [in] */ IPyCOMTest __RPC_FAR *ininterface,
            /* [retval][out] */ IPyCOMTest __RPC_FAR *__RPC_FAR *outinterface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSetInterfaceArray( 
            /* [in] */ SAFEARRAY __RPC_FAR * ininterface,
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *outinterface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMultipleInterfaces( 
            /* [out] */ IPyCOMTest __RPC_FAR *__RPC_FAR *outinterface1,
            /* [out] */ IPyCOMTest __RPC_FAR *__RPC_FAR *outinterface2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSetDispatch( 
            /* [in] */ IDispatch __RPC_FAR *indisp,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *outdisp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSetUnknown( 
            /* [in] */ IUnknown __RPC_FAR *inunk,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *outunk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TakeByRefTypedDispatch( 
            /* [out][in] */ IPyCOMTest __RPC_FAR *__RPC_FAR *inout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TakeByRefDispatch( 
            /* [out][in] */ IDispatch __RPC_FAR *__RPC_FAR *inout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetIntSafeArray( 
            /* [in] */ SAFEARRAY __RPC_FAR * ints,
            /* [retval][out] */ int __RPC_FAR *resultSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVariantSafeArray( 
            /* [in] */ SAFEARRAY __RPC_FAR * vars,
            /* [retval][out] */ int __RPC_FAR *resultSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSimpleSafeArray( 
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *ints) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSafeArrays( 
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *attrs,
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *attrs2,
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *ints) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSimpleCounter( 
            /* [retval][out] */ ISimpleCounter __RPC_FAR *__RPC_FAR *counter) = 0;
        
        virtual /* [vararg] */ HRESULT STDMETHODCALLTYPE SetVarArgs( 
            /* [in] */ SAFEARRAY __RPC_FAR * vars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLastVarArgs( 
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *result) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Fire( 
            /* [in] */ long nId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TestOptionals( 
            /* [defaultvalue][optional][in] */ BSTR strArg,
            /* [defaultvalue][optional][in] */ short sval,
            /* [defaultvalue][optional][in] */ long lval,
            /* [defaultvalue][optional][in] */ double dval,
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pret) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TestOptionals2( 
            double dval,
            /* [defaultvalue][optional] */ BSTR strval,
            /* [defaultvalue][optional] */ short sval,
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pret) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStruct( 
            /* [retval][out] */ TestStruct1 __RPC_FAR *ret) = 0;
        
        virtual /* [restricted] */ HRESULT STDMETHODCALLTYPE NotScriptable( 
            /* [out][in] */ int __RPC_FAR *val) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TestMyInterface( 
            /* [in] */ IUnknown __RPC_FAR *tester) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPyCOMTestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPyCOMTest __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPyCOMTest __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IPyCOMTest __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Start )( 
            IPyCOMTest __RPC_FAR * This,
            /* [retval][out] */ HCON __RPC_FAR *pnID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stop )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ CONNECTID nID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StopAll )( 
            IPyCOMTest __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Test )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ VARIANT key,
            /* [in] */ QsBoolean inval,
            /* [retval][out] */ QsBoolean __RPC_FAR *retval);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Test2 )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ QsAttribute inval,
            /* [retval][out] */ QsAttribute __RPC_FAR *retval);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Test3 )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ TestAttributes1 inval,
            /* [retval][out] */ TestAttributes1 __RPC_FAR *retval);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Test4 )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ TestAttributes2 inval,
            /* [retval][out] */ TestAttributes2 __RPC_FAR *retval);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Test5 )( 
            IPyCOMTest __RPC_FAR * This,
            /* [out][in] */ TestAttributes1 __RPC_FAR *inout);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSetInterface )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ IPyCOMTest __RPC_FAR *ininterface,
            /* [retval][out] */ IPyCOMTest __RPC_FAR *__RPC_FAR *outinterface);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSetInterfaceArray )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ SAFEARRAY __RPC_FAR * ininterface,
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *outinterface);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMultipleInterfaces )( 
            IPyCOMTest __RPC_FAR * This,
            /* [out] */ IPyCOMTest __RPC_FAR *__RPC_FAR *outinterface1,
            /* [out] */ IPyCOMTest __RPC_FAR *__RPC_FAR *outinterface2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSetDispatch )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *indisp,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *outdisp);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSetUnknown )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *inunk,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *outunk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TakeByRefTypedDispatch )( 
            IPyCOMTest __RPC_FAR * This,
            /* [out][in] */ IPyCOMTest __RPC_FAR *__RPC_FAR *inout);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TakeByRefDispatch )( 
            IPyCOMTest __RPC_FAR * This,
            /* [out][in] */ IDispatch __RPC_FAR *__RPC_FAR *inout);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetIntSafeArray )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ SAFEARRAY __RPC_FAR * ints,
            /* [retval][out] */ int __RPC_FAR *resultSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetVariantSafeArray )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ SAFEARRAY __RPC_FAR * vars,
            /* [retval][out] */ int __RPC_FAR *resultSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSimpleSafeArray )( 
            IPyCOMTest __RPC_FAR * This,
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *ints);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSafeArrays )( 
            IPyCOMTest __RPC_FAR * This,
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *attrs,
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *attrs2,
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *ints);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSimpleCounter )( 
            IPyCOMTest __RPC_FAR * This,
            /* [retval][out] */ ISimpleCounter __RPC_FAR *__RPC_FAR *counter);
        
        /* [vararg] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetVarArgs )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ SAFEARRAY __RPC_FAR * vars);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLastVarArgs )( 
            IPyCOMTest __RPC_FAR * This,
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *result);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Fire )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ long nId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TestOptionals )( 
            IPyCOMTest __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR strArg,
            /* [defaultvalue][optional][in] */ short sval,
            /* [defaultvalue][optional][in] */ long lval,
            /* [defaultvalue][optional][in] */ double dval,
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pret);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TestOptionals2 )( 
            IPyCOMTest __RPC_FAR * This,
            double dval,
            /* [defaultvalue][optional] */ BSTR strval,
            /* [defaultvalue][optional] */ short sval,
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pret);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStruct )( 
            IPyCOMTest __RPC_FAR * This,
            /* [retval][out] */ TestStruct1 __RPC_FAR *ret);
        
        /* [restricted] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NotScriptable )( 
            IPyCOMTest __RPC_FAR * This,
            /* [out][in] */ int __RPC_FAR *val);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TestMyInterface )( 
            IPyCOMTest __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *tester);
        
        END_INTERFACE
    } IPyCOMTestVtbl;

    interface IPyCOMTest
    {
        CONST_VTBL struct IPyCOMTestVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPyCOMTest_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPyCOMTest_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPyCOMTest_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPyCOMTest_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPyCOMTest_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPyCOMTest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPyCOMTest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPyCOMTest_Start(This,pnID)	\
    (This)->lpVtbl -> Start(This,pnID)

#define IPyCOMTest_Stop(This,nID)	\
    (This)->lpVtbl -> Stop(This,nID)

#define IPyCOMTest_StopAll(This)	\
    (This)->lpVtbl -> StopAll(This)

#define IPyCOMTest_Test(This,key,inval,retval)	\
    (This)->lpVtbl -> Test(This,key,inval,retval)

#define IPyCOMTest_Test2(This,inval,retval)	\
    (This)->lpVtbl -> Test2(This,inval,retval)

#define IPyCOMTest_Test3(This,inval,retval)	\
    (This)->lpVtbl -> Test3(This,inval,retval)

#define IPyCOMTest_Test4(This,inval,retval)	\
    (This)->lpVtbl -> Test4(This,inval,retval)

#define IPyCOMTest_Test5(This,inout)	\
    (This)->lpVtbl -> Test5(This,inout)

#define IPyCOMTest_GetSetInterface(This,ininterface,outinterface)	\
    (This)->lpVtbl -> GetSetInterface(This,ininterface,outinterface)

#define IPyCOMTest_GetSetInterfaceArray(This,ininterface,outinterface)	\
    (This)->lpVtbl -> GetSetInterfaceArray(This,ininterface,outinterface)

#define IPyCOMTest_GetMultipleInterfaces(This,outinterface1,outinterface2)	\
    (This)->lpVtbl -> GetMultipleInterfaces(This,outinterface1,outinterface2)

#define IPyCOMTest_GetSetDispatch(This,indisp,outdisp)	\
    (This)->lpVtbl -> GetSetDispatch(This,indisp,outdisp)

#define IPyCOMTest_GetSetUnknown(This,inunk,outunk)	\
    (This)->lpVtbl -> GetSetUnknown(This,inunk,outunk)

#define IPyCOMTest_TakeByRefTypedDispatch(This,inout)	\
    (This)->lpVtbl -> TakeByRefTypedDispatch(This,inout)

#define IPyCOMTest_TakeByRefDispatch(This,inout)	\
    (This)->lpVtbl -> TakeByRefDispatch(This,inout)

#define IPyCOMTest_SetIntSafeArray(This,ints,resultSize)	\
    (This)->lpVtbl -> SetIntSafeArray(This,ints,resultSize)

#define IPyCOMTest_SetVariantSafeArray(This,vars,resultSize)	\
    (This)->lpVtbl -> SetVariantSafeArray(This,vars,resultSize)

#define IPyCOMTest_GetSimpleSafeArray(This,ints)	\
    (This)->lpVtbl -> GetSimpleSafeArray(This,ints)

#define IPyCOMTest_GetSafeArrays(This,attrs,attrs2,ints)	\
    (This)->lpVtbl -> GetSafeArrays(This,attrs,attrs2,ints)

#define IPyCOMTest_GetSimpleCounter(This,counter)	\
    (This)->lpVtbl -> GetSimpleCounter(This,counter)

#define IPyCOMTest_SetVarArgs(This,vars)	\
    (This)->lpVtbl -> SetVarArgs(This,vars)

#define IPyCOMTest_GetLastVarArgs(This,result)	\
    (This)->lpVtbl -> GetLastVarArgs(This,result)

#define IPyCOMTest_Fire(This,nId)	\
    (This)->lpVtbl -> Fire(This,nId)

#define IPyCOMTest_TestOptionals(This,strArg,sval,lval,dval,pret)	\
    (This)->lpVtbl -> TestOptionals(This,strArg,sval,lval,dval,pret)

#define IPyCOMTest_TestOptionals2(This,dval,strval,sval,pret)	\
    (This)->lpVtbl -> TestOptionals2(This,dval,strval,sval,pret)

#define IPyCOMTest_GetStruct(This,ret)	\
    (This)->lpVtbl -> GetStruct(This,ret)

#define IPyCOMTest_NotScriptable(This,val)	\
    (This)->lpVtbl -> NotScriptable(This,val)

#define IPyCOMTest_TestMyInterface(This,tester)	\
    (This)->lpVtbl -> TestMyInterface(This,tester)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPyCOMTest_Start_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [retval][out] */ HCON __RPC_FAR *pnID);


void __RPC_STUB IPyCOMTest_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_Stop_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ CONNECTID nID);


void __RPC_STUB IPyCOMTest_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_StopAll_Proxy( 
    IPyCOMTest __RPC_FAR * This);


void __RPC_STUB IPyCOMTest_StopAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_Test_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ VARIANT key,
    /* [in] */ QsBoolean inval,
    /* [retval][out] */ QsBoolean __RPC_FAR *retval);


void __RPC_STUB IPyCOMTest_Test_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_Test2_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ QsAttribute inval,
    /* [retval][out] */ QsAttribute __RPC_FAR *retval);


void __RPC_STUB IPyCOMTest_Test2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_Test3_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ TestAttributes1 inval,
    /* [retval][out] */ TestAttributes1 __RPC_FAR *retval);


void __RPC_STUB IPyCOMTest_Test3_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_Test4_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ TestAttributes2 inval,
    /* [retval][out] */ TestAttributes2 __RPC_FAR *retval);


void __RPC_STUB IPyCOMTest_Test4_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_Test5_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [out][in] */ TestAttributes1 __RPC_FAR *inout);


void __RPC_STUB IPyCOMTest_Test5_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_GetSetInterface_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ IPyCOMTest __RPC_FAR *ininterface,
    /* [retval][out] */ IPyCOMTest __RPC_FAR *__RPC_FAR *outinterface);


void __RPC_STUB IPyCOMTest_GetSetInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_GetSetInterfaceArray_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ SAFEARRAY __RPC_FAR * ininterface,
    /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *outinterface);


void __RPC_STUB IPyCOMTest_GetSetInterfaceArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_GetMultipleInterfaces_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [out] */ IPyCOMTest __RPC_FAR *__RPC_FAR *outinterface1,
    /* [out] */ IPyCOMTest __RPC_FAR *__RPC_FAR *outinterface2);


void __RPC_STUB IPyCOMTest_GetMultipleInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_GetSetDispatch_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *indisp,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *outdisp);


void __RPC_STUB IPyCOMTest_GetSetDispatch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_GetSetUnknown_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *inunk,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *outunk);


void __RPC_STUB IPyCOMTest_GetSetUnknown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_TakeByRefTypedDispatch_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [out][in] */ IPyCOMTest __RPC_FAR *__RPC_FAR *inout);


void __RPC_STUB IPyCOMTest_TakeByRefTypedDispatch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_TakeByRefDispatch_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [out][in] */ IDispatch __RPC_FAR *__RPC_FAR *inout);


void __RPC_STUB IPyCOMTest_TakeByRefDispatch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_SetIntSafeArray_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ SAFEARRAY __RPC_FAR * ints,
    /* [retval][out] */ int __RPC_FAR *resultSize);


void __RPC_STUB IPyCOMTest_SetIntSafeArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_SetVariantSafeArray_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ SAFEARRAY __RPC_FAR * vars,
    /* [retval][out] */ int __RPC_FAR *resultSize);


void __RPC_STUB IPyCOMTest_SetVariantSafeArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_GetSimpleSafeArray_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *ints);


void __RPC_STUB IPyCOMTest_GetSimpleSafeArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_GetSafeArrays_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *attrs,
    /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *attrs2,
    /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *ints);


void __RPC_STUB IPyCOMTest_GetSafeArrays_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_GetSimpleCounter_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [retval][out] */ ISimpleCounter __RPC_FAR *__RPC_FAR *counter);


void __RPC_STUB IPyCOMTest_GetSimpleCounter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [vararg] */ HRESULT STDMETHODCALLTYPE IPyCOMTest_SetVarArgs_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ SAFEARRAY __RPC_FAR * vars);


void __RPC_STUB IPyCOMTest_SetVarArgs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_GetLastVarArgs_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *result);


void __RPC_STUB IPyCOMTest_GetLastVarArgs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_Fire_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ long nId);


void __RPC_STUB IPyCOMTest_Fire_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_TestOptionals_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR strArg,
    /* [defaultvalue][optional][in] */ short sval,
    /* [defaultvalue][optional][in] */ long lval,
    /* [defaultvalue][optional][in] */ double dval,
    /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pret);


void __RPC_STUB IPyCOMTest_TestOptionals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_TestOptionals2_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    double dval,
    /* [defaultvalue][optional] */ BSTR strval,
    /* [defaultvalue][optional] */ short sval,
    /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pret);


void __RPC_STUB IPyCOMTest_TestOptionals2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_GetStruct_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [retval][out] */ TestStruct1 __RPC_FAR *ret);


void __RPC_STUB IPyCOMTest_GetStruct_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [restricted] */ HRESULT STDMETHODCALLTYPE IPyCOMTest_NotScriptable_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [out][in] */ int __RPC_FAR *val);


void __RPC_STUB IPyCOMTest_NotScriptable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTest_TestMyInterface_Proxy( 
    IPyCOMTest __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *tester);


void __RPC_STUB IPyCOMTest_TestMyInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPyCOMTest_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CoPyCOMTest2;

#ifdef __cplusplus

class DECLSPEC_UUID("4E58A400-1117-11d1-9C4B-00AA00125A98")
CoPyCOMTest2;
#endif

#ifndef __IPyCOMTest2_INTERFACE_DEFINED__
#define __IPyCOMTest2_INTERFACE_DEFINED__

/* interface IPyCOMTest2 */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IPyCOMTest2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4E58A401-1117-11d1-9C4B-00AA00125A98")
    IPyCOMTest2 : public IPyCOMTest
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE TestDerived( 
            /* [in] */ QsAttribute inval,
            /* [retval][out] */ QsAttribute __RPC_FAR *retval) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPyCOMTest2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPyCOMTest2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPyCOMTest2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Start )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [retval][out] */ HCON __RPC_FAR *pnID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stop )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ CONNECTID nID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StopAll )( 
            IPyCOMTest2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Test )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ VARIANT key,
            /* [in] */ QsBoolean inval,
            /* [retval][out] */ QsBoolean __RPC_FAR *retval);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Test2 )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ QsAttribute inval,
            /* [retval][out] */ QsAttribute __RPC_FAR *retval);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Test3 )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ TestAttributes1 inval,
            /* [retval][out] */ TestAttributes1 __RPC_FAR *retval);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Test4 )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ TestAttributes2 inval,
            /* [retval][out] */ TestAttributes2 __RPC_FAR *retval);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Test5 )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [out][in] */ TestAttributes1 __RPC_FAR *inout);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSetInterface )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ IPyCOMTest __RPC_FAR *ininterface,
            /* [retval][out] */ IPyCOMTest __RPC_FAR *__RPC_FAR *outinterface);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSetInterfaceArray )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ SAFEARRAY __RPC_FAR * ininterface,
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *outinterface);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMultipleInterfaces )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [out] */ IPyCOMTest __RPC_FAR *__RPC_FAR *outinterface1,
            /* [out] */ IPyCOMTest __RPC_FAR *__RPC_FAR *outinterface2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSetDispatch )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *indisp,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *outdisp);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSetUnknown )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *inunk,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *outunk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TakeByRefTypedDispatch )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [out][in] */ IPyCOMTest __RPC_FAR *__RPC_FAR *inout);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TakeByRefDispatch )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [out][in] */ IDispatch __RPC_FAR *__RPC_FAR *inout);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetIntSafeArray )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ SAFEARRAY __RPC_FAR * ints,
            /* [retval][out] */ int __RPC_FAR *resultSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetVariantSafeArray )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ SAFEARRAY __RPC_FAR * vars,
            /* [retval][out] */ int __RPC_FAR *resultSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSimpleSafeArray )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *ints);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSafeArrays )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *attrs,
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *attrs2,
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *ints);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSimpleCounter )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [retval][out] */ ISimpleCounter __RPC_FAR *__RPC_FAR *counter);
        
        /* [vararg] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetVarArgs )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ SAFEARRAY __RPC_FAR * vars);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLastVarArgs )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *result);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Fire )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ long nId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TestOptionals )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR strArg,
            /* [defaultvalue][optional][in] */ short sval,
            /* [defaultvalue][optional][in] */ long lval,
            /* [defaultvalue][optional][in] */ double dval,
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pret);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TestOptionals2 )( 
            IPyCOMTest2 __RPC_FAR * This,
            double dval,
            /* [defaultvalue][optional] */ BSTR strval,
            /* [defaultvalue][optional] */ short sval,
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pret);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStruct )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [retval][out] */ TestStruct1 __RPC_FAR *ret);
        
        /* [restricted] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NotScriptable )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [out][in] */ int __RPC_FAR *val);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TestMyInterface )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *tester);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TestDerived )( 
            IPyCOMTest2 __RPC_FAR * This,
            /* [in] */ QsAttribute inval,
            /* [retval][out] */ QsAttribute __RPC_FAR *retval);
        
        END_INTERFACE
    } IPyCOMTest2Vtbl;

    interface IPyCOMTest2
    {
        CONST_VTBL struct IPyCOMTest2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPyCOMTest2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPyCOMTest2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPyCOMTest2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPyCOMTest2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPyCOMTest2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPyCOMTest2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPyCOMTest2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPyCOMTest2_Start(This,pnID)	\
    (This)->lpVtbl -> Start(This,pnID)

#define IPyCOMTest2_Stop(This,nID)	\
    (This)->lpVtbl -> Stop(This,nID)

#define IPyCOMTest2_StopAll(This)	\
    (This)->lpVtbl -> StopAll(This)

#define IPyCOMTest2_Test(This,key,inval,retval)	\
    (This)->lpVtbl -> Test(This,key,inval,retval)

#define IPyCOMTest2_Test2(This,inval,retval)	\
    (This)->lpVtbl -> Test2(This,inval,retval)

#define IPyCOMTest2_Test3(This,inval,retval)	\
    (This)->lpVtbl -> Test3(This,inval,retval)

#define IPyCOMTest2_Test4(This,inval,retval)	\
    (This)->lpVtbl -> Test4(This,inval,retval)

#define IPyCOMTest2_Test5(This,inout)	\
    (This)->lpVtbl -> Test5(This,inout)

#define IPyCOMTest2_GetSetInterface(This,ininterface,outinterface)	\
    (This)->lpVtbl -> GetSetInterface(This,ininterface,outinterface)

#define IPyCOMTest2_GetSetInterfaceArray(This,ininterface,outinterface)	\
    (This)->lpVtbl -> GetSetInterfaceArray(This,ininterface,outinterface)

#define IPyCOMTest2_GetMultipleInterfaces(This,outinterface1,outinterface2)	\
    (This)->lpVtbl -> GetMultipleInterfaces(This,outinterface1,outinterface2)

#define IPyCOMTest2_GetSetDispatch(This,indisp,outdisp)	\
    (This)->lpVtbl -> GetSetDispatch(This,indisp,outdisp)

#define IPyCOMTest2_GetSetUnknown(This,inunk,outunk)	\
    (This)->lpVtbl -> GetSetUnknown(This,inunk,outunk)

#define IPyCOMTest2_TakeByRefTypedDispatch(This,inout)	\
    (This)->lpVtbl -> TakeByRefTypedDispatch(This,inout)

#define IPyCOMTest2_TakeByRefDispatch(This,inout)	\
    (This)->lpVtbl -> TakeByRefDispatch(This,inout)

#define IPyCOMTest2_SetIntSafeArray(This,ints,resultSize)	\
    (This)->lpVtbl -> SetIntSafeArray(This,ints,resultSize)

#define IPyCOMTest2_SetVariantSafeArray(This,vars,resultSize)	\
    (This)->lpVtbl -> SetVariantSafeArray(This,vars,resultSize)

#define IPyCOMTest2_GetSimpleSafeArray(This,ints)	\
    (This)->lpVtbl -> GetSimpleSafeArray(This,ints)

#define IPyCOMTest2_GetSafeArrays(This,attrs,attrs2,ints)	\
    (This)->lpVtbl -> GetSafeArrays(This,attrs,attrs2,ints)

#define IPyCOMTest2_GetSimpleCounter(This,counter)	\
    (This)->lpVtbl -> GetSimpleCounter(This,counter)

#define IPyCOMTest2_SetVarArgs(This,vars)	\
    (This)->lpVtbl -> SetVarArgs(This,vars)

#define IPyCOMTest2_GetLastVarArgs(This,result)	\
    (This)->lpVtbl -> GetLastVarArgs(This,result)

#define IPyCOMTest2_Fire(This,nId)	\
    (This)->lpVtbl -> Fire(This,nId)

#define IPyCOMTest2_TestOptionals(This,strArg,sval,lval,dval,pret)	\
    (This)->lpVtbl -> TestOptionals(This,strArg,sval,lval,dval,pret)

#define IPyCOMTest2_TestOptionals2(This,dval,strval,sval,pret)	\
    (This)->lpVtbl -> TestOptionals2(This,dval,strval,sval,pret)

#define IPyCOMTest2_GetStruct(This,ret)	\
    (This)->lpVtbl -> GetStruct(This,ret)

#define IPyCOMTest2_NotScriptable(This,val)	\
    (This)->lpVtbl -> NotScriptable(This,val)

#define IPyCOMTest2_TestMyInterface(This,tester)	\
    (This)->lpVtbl -> TestMyInterface(This,tester)


#define IPyCOMTest2_TestDerived(This,inval,retval)	\
    (This)->lpVtbl -> TestDerived(This,inval,retval)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPyCOMTest2_TestDerived_Proxy( 
    IPyCOMTest2 __RPC_FAR * This,
    /* [in] */ QsAttribute inval,
    /* [retval][out] */ QsAttribute __RPC_FAR *retval);


void __RPC_STUB IPyCOMTest2_TestDerived_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPyCOMTest2_INTERFACE_DEFINED__ */


#ifndef __IPyCOMTestEvent_INTERFACE_DEFINED__
#define __IPyCOMTestEvent_INTERFACE_DEFINED__

/* interface IPyCOMTestEvent */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IPyCOMTestEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("af643370-5605-11d0-ae5f-cadd4c000000")
    IPyCOMTestEvent : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Fire( 
            /* [in] */ long nID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPyCOMTestEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPyCOMTestEvent __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPyCOMTestEvent __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPyCOMTestEvent __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IPyCOMTestEvent __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IPyCOMTestEvent __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IPyCOMTestEvent __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IPyCOMTestEvent __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Fire )( 
            IPyCOMTestEvent __RPC_FAR * This,
            /* [in] */ long nID);
        
        END_INTERFACE
    } IPyCOMTestEventVtbl;

    interface IPyCOMTestEvent
    {
        CONST_VTBL struct IPyCOMTestEventVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPyCOMTestEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPyCOMTestEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPyCOMTestEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPyCOMTestEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPyCOMTestEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPyCOMTestEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPyCOMTestEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPyCOMTestEvent_Fire(This,nID)	\
    (This)->lpVtbl -> Fire(This,nID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPyCOMTestEvent_Fire_Proxy( 
    IPyCOMTestEvent __RPC_FAR * This,
    /* [in] */ long nID);


void __RPC_STUB IPyCOMTestEvent_Fire_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPyCOMTestEvent_INTERFACE_DEFINED__ */


#ifndef __PyCOMTestEvent_DISPINTERFACE_DEFINED__
#define __PyCOMTestEvent_DISPINTERFACE_DEFINED__

/* dispinterface PyCOMTestEvent */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID_PyCOMTestEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("b636cac0-5605-11d0-ae5f-cadd4c000000")
    PyCOMTestEvent : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct PyCOMTestEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            PyCOMTestEvent __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            PyCOMTestEvent __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            PyCOMTestEvent __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            PyCOMTestEvent __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            PyCOMTestEvent __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            PyCOMTestEvent __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            PyCOMTestEvent __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } PyCOMTestEventVtbl;

    interface PyCOMTestEvent
    {
        CONST_VTBL struct PyCOMTestEventVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define PyCOMTestEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define PyCOMTestEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define PyCOMTestEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define PyCOMTestEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define PyCOMTestEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define PyCOMTestEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define PyCOMTestEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __PyCOMTestEvent_DISPINTERFACE_DEFINED__ */


#ifndef __IPyCOMTestNoDispatch_INTERFACE_DEFINED__
#define __IPyCOMTestNoDispatch_INTERFACE_DEFINED__

/* interface IPyCOMTestNoDispatch */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IPyCOMTestNoDispatch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("36f7a0f7-10c9-43b7-9bd8-47a932b11d84")
    IPyCOMTestNoDispatch : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PlayWithSomeArgs( 
            /* [out][in] */ VARIANT __RPC_FAR *var,
            /* [out][in] */ long __RPC_FAR *l) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ChangeStruct( 
            /* [in] */ TestStruct1 __RPC_FAR *inval,
            /* [retval][out] */ TestStruct1 __RPC_FAR *ret) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InPtr( 
            /* [in] */ int __RPC_FAR *inval,
            /* [out] */ int __RPC_FAR *outval) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPyCOMTestNoDispatchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPyCOMTestNoDispatch __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPyCOMTestNoDispatch __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPyCOMTestNoDispatch __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PlayWithSomeArgs )( 
            IPyCOMTestNoDispatch __RPC_FAR * This,
            /* [out][in] */ VARIANT __RPC_FAR *var,
            /* [out][in] */ long __RPC_FAR *l);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ChangeStruct )( 
            IPyCOMTestNoDispatch __RPC_FAR * This,
            /* [in] */ TestStruct1 __RPC_FAR *inval,
            /* [retval][out] */ TestStruct1 __RPC_FAR *ret);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InPtr )( 
            IPyCOMTestNoDispatch __RPC_FAR * This,
            /* [in] */ int __RPC_FAR *inval,
            /* [out] */ int __RPC_FAR *outval);
        
        END_INTERFACE
    } IPyCOMTestNoDispatchVtbl;

    interface IPyCOMTestNoDispatch
    {
        CONST_VTBL struct IPyCOMTestNoDispatchVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPyCOMTestNoDispatch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPyCOMTestNoDispatch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPyCOMTestNoDispatch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPyCOMTestNoDispatch_PlayWithSomeArgs(This,var,l)	\
    (This)->lpVtbl -> PlayWithSomeArgs(This,var,l)

#define IPyCOMTestNoDispatch_ChangeStruct(This,inval,ret)	\
    (This)->lpVtbl -> ChangeStruct(This,inval,ret)

#define IPyCOMTestNoDispatch_InPtr(This,inval,outval)	\
    (This)->lpVtbl -> InPtr(This,inval,outval)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPyCOMTestNoDispatch_PlayWithSomeArgs_Proxy( 
    IPyCOMTestNoDispatch __RPC_FAR * This,
    /* [out][in] */ VARIANT __RPC_FAR *var,
    /* [out][in] */ long __RPC_FAR *l);


void __RPC_STUB IPyCOMTestNoDispatch_PlayWithSomeArgs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTestNoDispatch_ChangeStruct_Proxy( 
    IPyCOMTestNoDispatch __RPC_FAR * This,
    /* [in] */ TestStruct1 __RPC_FAR *inval,
    /* [retval][out] */ TestStruct1 __RPC_FAR *ret);


void __RPC_STUB IPyCOMTestNoDispatch_ChangeStruct_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPyCOMTestNoDispatch_InPtr_Proxy( 
    IPyCOMTestNoDispatch __RPC_FAR * This,
    /* [in] */ int __RPC_FAR *inval,
    /* [out] */ int __RPC_FAR *outval);


void __RPC_STUB IPyCOMTestNoDispatch_InPtr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPyCOMTestNoDispatch_INTERFACE_DEFINED__ */


#ifndef __IPyCOMTestNoDispatchEvent_INTERFACE_DEFINED__
#define __IPyCOMTestNoDispatchEvent_INTERFACE_DEFINED__

/* interface IPyCOMTestNoDispatchEvent */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IPyCOMTestNoDispatchEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b21b658a-19a8-488a-8d3a-b63b3cf98501")
    IPyCOMTestNoDispatchEvent : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Fire( 
            /* [in] */ long nID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPyCOMTestNoDispatchEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPyCOMTestNoDispatchEvent __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPyCOMTestNoDispatchEvent __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPyCOMTestNoDispatchEvent __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Fire )( 
            IPyCOMTestNoDispatchEvent __RPC_FAR * This,
            /* [in] */ long nID);
        
        END_INTERFACE
    } IPyCOMTestNoDispatchEventVtbl;

    interface IPyCOMTestNoDispatchEvent
    {
        CONST_VTBL struct IPyCOMTestNoDispatchEventVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPyCOMTestNoDispatchEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPyCOMTestNoDispatchEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPyCOMTestNoDispatchEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPyCOMTestNoDispatchEvent_Fire(This,nID)	\
    (This)->lpVtbl -> Fire(This,nID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPyCOMTestNoDispatchEvent_Fire_Proxy( 
    IPyCOMTestNoDispatchEvent __RPC_FAR * This,
    /* [in] */ long nID);


void __RPC_STUB IPyCOMTestNoDispatchEvent_Fire_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPyCOMTestNoDispatchEvent_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CoPyCOMTestNoDispatch;

#ifdef __cplusplus

class DECLSPEC_UUID("638630ac-a734-45a2-8080-fda5c1e47f66")
CoPyCOMTestNoDispatch;
#endif
#endif /* __PyCOMTestLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


