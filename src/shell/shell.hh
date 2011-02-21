

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Mon Feb 21 10:43:51 2011
 */
/* Compiler settings for .\shell.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __shell_hh__
#define __shell_hh__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IShellExt_FWD_DEFINED__
#define __IShellExt_FWD_DEFINED__
typedef interface IShellExt IShellExt;
#endif 	/* __IShellExt_FWD_DEFINED__ */


#ifndef __ShellExt_FWD_DEFINED__
#define __ShellExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellExt ShellExt;
#else
typedef struct ShellExt ShellExt;
#endif /* __cplusplus */

#endif 	/* __ShellExt_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IShellExt_INTERFACE_DEFINED__
#define __IShellExt_INTERFACE_DEFINED__

/* interface IShellExt */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IShellExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("02CEB95B-2C12-40CD-A434-D9F699F4B1AA")
    IShellExt : public IUnknown
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IShellExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellExt * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellExt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellExt * This);
        
        END_INTERFACE
    } IShellExtVtbl;

    interface IShellExt
    {
        CONST_VTBL struct IShellExtVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellExt_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IShellExt_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IShellExt_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IShellExt_INTERFACE_DEFINED__ */



#ifndef __ShellLib_LIBRARY_DEFINED__
#define __ShellLib_LIBRARY_DEFINED__

/* library ShellLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_ShellLib;

EXTERN_C const CLSID CLSID_ShellExt;

#ifdef __cplusplus

class DECLSPEC_UUID("7022C5BB-445E-4300-99F2-0B7EDA907A53")
ShellExt;
#endif
#endif /* __ShellLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


