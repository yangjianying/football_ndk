
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <math.h>
#include <unistd.h>

#include <optional>
#include <string>
#include <vector>

#include <optional>	// 
#include <string>

#if 0

/* binder ndk :

Initial C library for libbinder.

This creates a simple wrapper around libbinder with a stable C ABI. It
also embeds the concept of an IBinder and IBinder transactions into the
ABI so that parts of their transactions can be changed and considered
implementation details of libbinder. With this basic class, you can
create a service, use it with primitive data types, but it does not yet
suppport the entire range of binder objects.


API Level 	Android	Linux kernel
29 			10 		Unknown 						2019-06-05(Beta 4) 	-
28 			9 		4.4.107, 4.9.84, and 4.14.42 	2018-08-06 	无
27 			8.1 	4.10 							2017-12-05 	无
26 			8.0 	4.10 							2017-08-21 	无

*/

#ifdef USING_binder_ndk  // must be under -std=c++17

#include <android/binder_ibinder.h>
#include <android/binder_parcel.h>
#include <android/binder_status.h>

#include <android/binder_auto_utils.h>
	/*
	#include <android/binder_ibinder.h>
	#include <android/binder_parcel.h>
	#include <android/binder_status.h>
	*/
#include <android/binder_parcel_utils.h>
	/*
	#include <android/binder_auto_utils.h>
	#include <android/binder_parcel.h>
	*/

#include <android/binder_interface_utils.h>
	/*
	#include <android/binder_auto_utils.h>
	#include <android/binder_ibinder.h>
	*/

/*

In file included from /home/yangjianying/disk3/football/football/binder/BinderTest.cpp:30:
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:59:10: error: no member named 'optional' in namespace 'std'
    std::optional<std::vector<T>>* vec = static_cast<std::optional<std::vector<T>>*>(vectorData);
    ~~~~~^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:59:33: error: expected
      '(' for function-style cast or type construction
    std::optional<std::vector<T>>* vec = static_cast<std::optional<std::vector<T>>*>(vectorData);
                  ~~~~~~~~~~~~~~^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:59:36: error: use of undeclared identifier 'vec'
    std::optional<std::vector<T>>* vec = static_cast<std::optional<std::vector<T>>*>(vectorData);
                                   ^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:59:59: error: no template named 'optional' in namespace 'std'
    std::optional<std::vector<T>>* vec = static_cast<std::optional<std::vector<T>>*>(vectorData);
                                                     ~~~~~^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:62:10: error: use of undeclared identifier 'vec'
        *vec = std::nullopt;
         ^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:62:21: error: no member named 'nullopt' in namespace 'std'
        *vec = std::nullopt;
               ~~~~~^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:66:17: error: no member named 'optional' in namespace 'std'
    *vec = std::optional<std::vector<T>>(std::vector<T>{});
           ~~~~~^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:66:40: error: expected
      '(' for function-style cast or type construction
    *vec = std::optional<std::vector<T>>(std::vector<T>{});
                         ~~~~~~~~~~~~~~^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:66:6: error: use of undeclared identifier 'vec'
    *vec = std::optional<std::vector<T>>(std::vector<T>{});
     ^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:68:41: error: use of undeclared identifier 'vec'
    if (static_cast<size_t>(length) > (*vec)->max_size()) return false;
                                        ^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:69:7: error: use of undeclared identifier 'vec'
    (*vec)->resize(length);
      ^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:71:20: error: use of undeclared identifier 'vec'
    *outBuffer = (*vec)->data();
                   ^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:110:10: error: no member named 'optional' in namespace 'std'
    std::optional<std::vector<T>>* vec = static_cast<std::optional<std::vector<T>>*>(vectorData);
    ~~~~~^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:110:33: error: expected
      '(' for function-style cast or type construction
    std::optional<std::vector<T>>* vec = static_cast<std::optional<std::vector<T>>*>(vectorData);
                  ~~~~~~~~~~~~~~^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:110:36: error: use of undeclared identifier 'vec'
    std::optional<std::vector<T>>* vec = static_cast<std::optional<std::vector<T>>*>(vectorData);
                                   ^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:110:59: error: no template named 'optional' in namespace 'std'
    std::optional<std::vector<T>>* vec = static_cast<std::optional<std::vector<T>>*>(vectorData);
                                                     ~~~~~^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:113:10: error: use of undeclared identifier 'vec'
        *vec = std::nullopt;
         ^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:113:21: error: no member named 'nullopt' in namespace 'std'
        *vec = std::nullopt;
               ~~~~~^
/home/yangjianying/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/android/binder_parcel_utils.h:117:17: error: no member named 'optional' in namespace 'std'
    *vec = std::optional<std::vector<T>>(std::vector<T>{});
           ~~~~~^
fatal error: too many errors emitted, stopping now [-ferror-limit=]
*/

// .h
class IFoo : 
		//public virtual ::android::RefBase 
	{
   public:
    static const char* kSomeInstanceName;
    static const char* kInstanceNameToDieFor;

    static AIBinder_Class* kClass;

    // Takes ownership of IFoo
    binder_status_t addService(const char* instance);
    static ::android::sp<IFoo> getService(const char* instance, AIBinder** outBinder = nullptr);


    enum Call {
        DOFOO = FIRST_CALL_TRANSACTION + 0,
        DIE = FIRST_CALL_TRANSACTION + 1,
    };

    virtual ~IFoo();

    virtual binder_status_t doubleNumber(int32_t in, int32_t* out) = 0;
    virtual binder_status_t die() = 0;

   private:
    // this variable is only when IFoo is local (since this test combines 'IFoo' and 'BnFoo'), not
    // for BpFoo.
    AIBinder_Weak* mWeakBinder = nullptr;
};


///////////////////////////////////// .cpp
struct IFoo_Class_Data {
    sp<IFoo> foo;
};
void* IFoo_Class_onCreate(void* args) {
    IFoo_Class_Data* foo = static_cast<IFoo_Class_Data*>(args);
    // This is a foo, but we're currently not verifying that. So, the method newLocalBinder is
    // coupled with this.
    return static_cast<void*>(foo);
}
void IFoo_Class_onDestroy(void* userData) {
    delete static_cast<IFoo_Class_Data*>(userData);
}
binder_status_t IFoo_Class_onTransact(AIBinder* binder, transaction_code_t code, const AParcel* in,
                                      AParcel* out) {
    binder_status_t stat = STATUS_FAILED_TRANSACTION;

    sp<IFoo> foo = static_cast<IFoo_Class_Data*>(AIBinder_getUserData(binder))->foo;
    CHECK(foo != nullptr) << "Transaction made on already deleted object";

    switch (code) {
        case IFoo::DOFOO: {
            int32_t valueIn;
            int32_t valueOut;
            stat = AParcel_readInt32(in, &valueIn);
            if (stat != STATUS_OK) break;
            stat = foo->doubleNumber(valueIn, &valueOut);
            if (stat != STATUS_OK) break;
            stat = AParcel_writeInt32(out, valueOut);
            break;
        }
        case IFoo::DIE: {
            stat = foo->die();
            break;
        }
    }
    return stat;
}
AIBinder_Class* IFoo::kClass = AIBinder_Class_define(kIFooDescriptor, IFoo_Class_onCreate,
                                                     IFoo_Class_onDestroy, IFoo_Class_onTransact);
IFoo::~IFoo() {
  AIBinder_Weak_delete(mWeakBinder);
}

class BpFoo : public IFoo {
   public:
    explicit BpFoo(AIBinder* binder) : mBinder(binder) {}
    virtual ~BpFoo() { AIBinder_decStrong(mBinder); }

    virtual binder_status_t doubleNumber(int32_t in, int32_t* out) {
        binder_status_t stat = STATUS_OK;

        AParcel* parcelIn;
        stat = AIBinder_prepareTransaction(mBinder, &parcelIn);
        if (stat != STATUS_OK) return stat;

        stat = AParcel_writeInt32(parcelIn, in);
        if (stat != STATUS_OK) return stat;

        ::ndk::ScopedAParcel parcelOut;
        stat = AIBinder_transact(mBinder, IFoo::DOFOO, &parcelIn, parcelOut.getR(), 0 /*flags*/);
        if (stat != STATUS_OK) return stat;

        stat = AParcel_readInt32(parcelOut.get(), out);
        if (stat != STATUS_OK) return stat;

        return stat;
    }

    virtual binder_status_t die() {
        binder_status_t stat = STATUS_OK;

        AParcel* parcelIn;
        stat = AIBinder_prepareTransaction(mBinder, &parcelIn);

        ::ndk::ScopedAParcel parcelOut;
        stat = AIBinder_transact(mBinder, IFoo::DIE, &parcelIn, parcelOut.getR(), 0 /*flags*/);

        return stat;
    }

   private:
    // Always assumes one refcount
    AIBinder* mBinder;
};



binder_status_t IFoo::addService(const char* instance) {
    AIBinder* binder = nullptr;

    if (mWeakBinder != nullptr) {
        // one strong ref count of binder
        binder = AIBinder_Weak_promote(mWeakBinder);
    }
    if (binder == nullptr) {
        // or one strong refcount here
        binder = AIBinder_new(IFoo::kClass, static_cast<void*>(new IFoo_Class_Data{this}));
        if (mWeakBinder != nullptr) {
            AIBinder_Weak_delete(mWeakBinder);
        }
        mWeakBinder = AIBinder_Weak_new(binder);
    }

    binder_status_t status = AServiceManager_addService(binder, instance);
    // Strong references we care about kept by remote process
    AIBinder_decStrong(binder);
    return status;
}

/* static */sp<IFoo> IFoo::getService(const char* instance, AIBinder** outBinder) {
    AIBinder* binder = AServiceManager_getService(instance);  // maybe nullptr
    if (binder == nullptr) {
        return nullptr;
    }

    if (!AIBinder_associateClass(binder, IFoo::kClass)) {
        AIBinder_decStrong(binder);
        return nullptr;
    }

    if (outBinder != nullptr) {
        AIBinder_incStrong(binder);
        *outBinder = binder;
    }

    if (AIBinder_isRemote(binder)) {
        sp<IFoo> ret = new BpFoo(binder);  // takes ownership of binder
        return ret;
    }

    IFoo_Class_Data* data = static_cast<IFoo_Class_Data*>(AIBinder_getUserData(binder));

    CHECK(data != nullptr);  // always created with non-null data

    sp<IFoo> ret = data->foo;

    AIBinder* held = AIBinder_Weak_promote(ret->mWeakBinder);
    CHECK(held == binder);
    AIBinder_decStrong(held);

    AIBinder_decStrong(binder);
    return ret;
}

// usage

class MyTestFoo : public IFoo {
    binder_status_t doubleNumber(int32_t in, int32_t* out) override {
        *out = 2 * in;
        LOG(INFO) << "doubleNumber (" << in << ") => " << *out;
        return STATUS_OK;
    }
    binder_status_t die() override {
        ADD_FAILURE() << "die called on local instance";
        return STATUS_OK;
    }
};

void test_NdkBinder_GetServiceInProcess() {
    static const char* kInstanceName = "test-get-service-in-process";

    sp<IFoo> foo = new MyTestFoo;
    assert(STATUS_OK == foo->addService(kInstanceName));

    sp<IFoo> getFoo = IFoo::getService(kInstanceName);
    assert(foo.get() == getFoo.get());

    int32_t out;
    assert(STATUS_OK == getFoo->doubleNumber(1, &out));
    assert(2 == out);
}


#endif

#endif


#if 1

/*

// header
class ICtrlService : public android::IInterface
{
public:
	DECLARE_META_INTERFACE(CtrlService);
	// 
	func1
	func2
	...
};
class BnCtrlService : public android::BnInterface<ICtrlService>
{
public:
    virtual android::status_t onTransact( uint32_t code,
                                          const android::Parcel& data,
                                          android::Parcel* reply,
                                          uint32_t flags = 0);
};

// cpp
class BpCtrlService : public BpInterface<ICtrlService>
{
public:
	// proxy impl
	func1
	func2
	...
}

IMPLEMENT_META_INTERFACE(CtrlService, "vendor.display.fr.ICtrlService");

status_t BnCtrlService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
	switch(code) {
	case 1:
		func1
	case 2:
		func2
	...
	}
}


*/

#endif

