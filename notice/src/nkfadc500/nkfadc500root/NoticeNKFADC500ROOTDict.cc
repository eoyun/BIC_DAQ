// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME NoticeNKFADC500ROOTDict
#define R__NO_DEPRECATION

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "ROOT/RConfig.hxx"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Header files passed as explicit arguments
#include "NoticeNKFADC500ROOT.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static void *new_NKNKFADC500(void *p = nullptr);
   static void *newArray_NKNKFADC500(Long_t size, void *p);
   static void delete_NKNKFADC500(void *p);
   static void deleteArray_NKNKFADC500(void *p);
   static void destruct_NKNKFADC500(void *p);
   static void streamer_NKNKFADC500(TBuffer &buf, void *obj);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::NKNKFADC500*)
   {
      ::NKNKFADC500 *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::NKNKFADC500 >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("NKNKFADC500", ::NKNKFADC500::Class_Version(), "NoticeNKFADC500ROOT.h", 8,
                  typeid(::NKNKFADC500), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::NKNKFADC500::Dictionary, isa_proxy, 18,
                  sizeof(::NKNKFADC500) );
      instance.SetNew(&new_NKNKFADC500);
      instance.SetNewArray(&newArray_NKNKFADC500);
      instance.SetDelete(&delete_NKNKFADC500);
      instance.SetDeleteArray(&deleteArray_NKNKFADC500);
      instance.SetDestructor(&destruct_NKNKFADC500);
      instance.SetStreamerFunc(&streamer_NKNKFADC500);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::NKNKFADC500*)
   {
      return GenerateInitInstanceLocal(static_cast<::NKNKFADC500*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::NKNKFADC500*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr NKNKFADC500::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *NKNKFADC500::Class_Name()
{
   return "NKNKFADC500";
}

//______________________________________________________________________________
const char *NKNKFADC500::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::NKNKFADC500*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int NKNKFADC500::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::NKNKFADC500*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *NKNKFADC500::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::NKNKFADC500*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *NKNKFADC500::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::NKNKFADC500*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void NKNKFADC500::Streamer(TBuffer &R__b)
{
   // Stream an object of class NKNKFADC500.

   TObject::Streamer(R__b);
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_NKNKFADC500(void *p) {
      return  p ? new(p) ::NKNKFADC500 : new ::NKNKFADC500;
   }
   static void *newArray_NKNKFADC500(Long_t nElements, void *p) {
      return p ? new(p) ::NKNKFADC500[nElements] : new ::NKNKFADC500[nElements];
   }
   // Wrapper around operator delete
   static void delete_NKNKFADC500(void *p) {
      delete (static_cast<::NKNKFADC500*>(p));
   }
   static void deleteArray_NKNKFADC500(void *p) {
      delete [] (static_cast<::NKNKFADC500*>(p));
   }
   static void destruct_NKNKFADC500(void *p) {
      typedef ::NKNKFADC500 current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
   // Wrapper around a custom streamer member function.
   static void streamer_NKNKFADC500(TBuffer &buf, void *obj) {
      ((::NKNKFADC500*)obj)->::NKNKFADC500::Streamer(buf);
   }
} // end of namespace ROOT for class ::NKNKFADC500

namespace {
  void TriggerDictionaryInitialization_NoticeNKFADC500ROOTDict_Impl() {
    static const char* headers[] = {
"NoticeNKFADC500ROOT.h",
nullptr
    };
    static const char* includePaths[] = {
"/home/kobic/bic_daq/notice/include",
"/home/kobic/Software/Install/root-6.32.10/include/",
"/home/kobic/bic_daq/notice/src/nkfadc500/nkfadc500root/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "NoticeNKFADC500ROOTDict dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class __attribute__((annotate("$clingAutoload$NoticeNKFADC500ROOT.h")))  NKNKFADC500;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "NoticeNKFADC500ROOTDict dictionary payload"


#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "NoticeNKFADC500ROOT.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"NKNKFADC500", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("NoticeNKFADC500ROOTDict",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_NoticeNKFADC500ROOTDict_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_NoticeNKFADC500ROOTDict_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_NoticeNKFADC500ROOTDict() {
  TriggerDictionaryInitialization_NoticeNKFADC500ROOTDict_Impl();
}
