

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Thu Oct 07 11:28:09 2010
 */
/* Compiler settings for .\irShell.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

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

#ifndef __irShell_h__
#define __irShell_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IirShellExt_FWD_DEFINED__
#define __IirShellExt_FWD_DEFINED__
typedef interface IirShellExt IirShellExt;
#endif 	/* __IirShellExt_FWD_DEFINED__ */


#ifndef __irShellExt_FWD_DEFINED__
#define __irShellExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class irShellExt irShellExt;
#else
typedef struct irShellExt irShellExt;
#endif /* __cplusplus */

#endif 	/* __irShellExt_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IirShellExt_INTERFACE_DEFINED__
#define __IirShellExt_INTERFACE_DEFINED__

/* interface IirShellExt */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IirShellExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("02CEB95B-2C12-40CD-A434-D9F699F4B1AA")
    IirShellExt : public IUnknown
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IirShellExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IirShellExt * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IirShellExt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IirShellExt * This);
        
        END_INTERFACE
    } IirShellExtVtbl;

    interface IirShellExt
    {
        CONST_VTBL struct IirShellExtVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IirShellExt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IirShellExt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IirShellExt_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IirShellExt_INTERFACE_DEFINED__ */



#ifndef __irShellLib_LIBRARY_DEFINED__
#define __irShellLib_LIBRARY_DEFINED__

/* library irShellLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_irShellLib;

EXTERN_C const CLSID CLSID_irShellExt;

#ifdef __cplusplus

class DECLSPEC_UUID("7022C5BB-445E-4300-99F2-0B7EDA907A53")
irShellExt;
#endif
#endif /* __irShellLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


