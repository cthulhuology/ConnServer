// Cache Class
//
// © 2010 David J. Goehrig
// 

#ifndef __HAVE_CACHE_H__
#define __HAVE_CACHE_H__

#include "object.h"

class Cache : public Object {
	public:
		static bool create(const string& s);
		static bool fini();	
		static bool store();
		static UInt64 next();

		static bool add(ID obj, bool alloced);
		static bool update(Message& m);

		template <class T> static T* find(UInt64 i);
		template <class T> static T* find(const string& s);

		static string statistics();
	
		bool init(const string& s); 
		string tos(); 

	protected: \
		Cache(); \
		Cache(const Cache&);\
		~Cache(); \
		Cache& operator=(const Cache&); \
		
	private:
		static Cache* cache;
		UInt64 next_guid;
		UInt64 idspace;
		map<UInt64,ID> obj_id_map;
		map<string,int> cached;
		map<string,int> allocated;
};

template<class T> T* Cache::find(UInt64 objid)
{
	cerr << "[Cache::find] Looking up id " << objid << endl;
	ID obj = cache->obj_id_map[objid];
	if (obj != NULL) {
		obj->refs += 1;
		return static_cast<T*>(obj);
	}
	T* tobj = T::cache_alloc(objid);
	cerr << "[Cache::find] Not in cache, loading " << objid << endl;
	if(tobj->load()) {
		cerr << "[Cache::find] Loaded " << objid << endl;
		cache->cached[tobj->type()] += 1;
		tobj->refs += 1;
		return tobj;
	}
	delete tobj;
	cerr << "[Cache::find] did not load " << objid << " returning NULL" << endl;
	return NULL;
}

template<class T> T* Cache::find(const string& s)
{
	if (s.empty()) {
		cerr << "[Cache::find] got empty string returning NULL" << endl;
		return (T*) NULL;
	}
	cerr << "[Cache::find] looking up " << s << endl;
	return find<T>(Uint64_of_string(s));
}


#endif
