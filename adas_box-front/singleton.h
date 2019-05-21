#ifndef SINGLETON_H_
#define SINGLETON_H_

/**
 *  * singleton declaration
 *   */
#define SINGLETON_DECL(x) \
	    public:\
    static x* getInstance() {\
	            if (_instance == 0) {\
			                _instance = new x;\
			            }\
	            return _instance;\
	        }\
    static void releaseInstance() {\
	            delete _instance;\
	            _instance = 0;\
	        }\
private:\
    static x* _instance;

#define SINGLETON_DECL_WITH_CTOR(x) \
	        SINGLETON_DECL(x)\
        x() {}

#define SINGLETON_DECL_WITH_DTOR(x) \
	        SINGLETON_DECL(x)\
        ~x() {}

#define SINGLETON_DECL_WITH_CTOR_AND_DTOR(x) \
	        SINGLETON_DECL(x)\
        x() {}\
        ~x() {}

/**
 *  * singleton implementation
 *   */
#define SINGLETON_IMPL(x) x* x::_instance = 0;

#define INIT(x) x::getInstance()->init()
#define CREATE(x, y) x::getInstance()->create(y)
#define START(x) x::getInstance()->start()
#define JOIN(x) x::getInstance()->join()
#define DESTROY(x) x::getInstance()->destroy()
#define REL(x) x::releaseInstance()

#endif /* SINGLETON_H_ */
