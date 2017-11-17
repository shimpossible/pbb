#ifndef __PBB_DELEGATE_H__
#define __PBB_DELEGATE_H__

namespace pbb
{
    class delegate
    {
    public:        
        ~delegate() {}
        template<typename T>
        static delegate* create(T& obj, void(T::*Fptr)())
        {
            delegate* val = new delegate();
            Obj<T>* o = new Obj<T>();
            o->mObj = &obj;
            o->mFptr = Fptr;
            val->mBase = o;
            return val;
        }

        virtual void invoke()
        {
            mBase->invoke();
        }
    protected:
        delegate() {}
        class Base {
        public:
            virtual void invoke() = 0;
        } *mBase;
        class Func : public Base
        {
            void (*mFptr)(void* data);
        };
        template<typename T>
        class Obj : public Base
        {
        public:
            virtual void invoke()
            {
                (mObj->*mFptr)();
            }
            void(T::*mFptr)();
            T* mObj;
        };
    };
}
#endif